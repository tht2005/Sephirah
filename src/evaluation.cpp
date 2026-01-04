#include "evaluation.h"
#include "bitboard.h"
#include "position.h"
#include "types.h"
#include <algorithm>
#include <cstdlib>

// --- Constants ---
const Value TEMPO = Value(20); 

// Weights for Mop-up
const int MopUp_CenterWeight = 10; // Push enemy king to edge
const int MopUp_CloseWeight  = 4;  // Bring our king closer

// Helper: Manhattan distance from center
// Center squares (e4, d4, e5, d5) have distance 0. Corners have distance 6.
int dist_from_center(Square s) {
	int file = get_file(s);
	int rank = get_rank(s);
	// Center is between 3 and 4 (0-indexed 3.5).
	// Multiply by 2 to keep integers: abs(2*file - 7) + abs(2*rank - 7)
	// Or simpler logic:
	int f_dist = std::max(3 - file, file - 4);
	int r_dist = std::max(3 - rank, rank - 4);
	return f_dist + r_dist;
}

// Helper: Chebyshev distance between two squares (King movement distance)
int dist_between(Square s1, Square s2) {
	int f_diff = std::abs(get_file(s1) - get_file(s2));
	int r_diff = std::abs(get_rank(s1) - get_rank(s2));
	return std::max(f_diff, r_diff);
}

int count(const Position &pos, Color c, PieceType pt) {
	return __builtin_popcountll(pos.pieces(c, pt));
}

// Check if a piece at 'sq' is attacked by a pawn of color 'attacker'
bool is_attacked_by_pawn(const Position& pos, Square sq, Color attacker) {
	Direction down = (attacker == WHITE) ? SOUTH : NORTH;
	Square capture_left = sq + down + WEST;
	if (is_ok(capture_left) && abs(get_file(capture_left) - get_file(sq)) == 1) {
		if (pos.piece_on(capture_left) == make_piece(attacker, PAWN)) return true;
	}
	Square capture_right = sq + down + EAST;
	if (is_ok(capture_right) && abs(get_file(capture_right) - get_file(sq)) == 1) {
		if (pos.piece_on(capture_right) == make_piece(attacker, PAWN)) return true;
	}
	return false;
}

Value eval(const Position &pos) {
	Score score = SCORE_ZERO;
	Color us = pos.side_to_move();
	
	// 1. Material & PSQT (Base Score)
	Score whiteScore = SCORE_ZERO;
	Score blackScore = SCORE_ZERO;

	// We track material to decide if we need Mop-up
	int whiteMaterial = 0;
	int blackMaterial = 0;

	for (PieceType pt = PAWN; pt <= KING; ++pt) {
		Bitboard w = pos.pieces(WHITE, pt);
		Bitboard b = pos.pieces(BLACK, pt);
		
		int countW = 0;
		while (w) {
			Square s = pop_lsb(w);
			whiteScore += PSQT::psq[make_piece(WHITE, pt)][s];
			countW++;
		}
		
		int countB = 0;
		while (b) {
			Square s = pop_lsb(b);
			blackScore += PSQT::psq[make_piece(BLACK, pt)][s];
			countB++;
		}
		
		if (pt != KING) {
			whiteMaterial += countW * PSQT::PieceValue[pt]; // Using MG value approx
			blackMaterial += countB * PSQT::PieceValue[pt];
		}
	}

	Score totalScore = whiteScore - blackScore;

	// 2. Mop-up Evaluation (Fixes the "Zigzag" / "Lazy Mate" issue)
	// Only apply if the winning side has enough material and losing side has no pawns (usually)
	// A simple heuristic: If one side has > 2000 material advantage (approx a Rook + piece)
	
	// Evaluate from White's perspective
	Value mg = mg_value(totalScore);
	Value eg = eg_value(totalScore);

	Square whiteKing = lsb(pos.pieces(WHITE, KING));
	Square blackKing = lsb(pos.pieces(BLACK, KING));

	// If White is winning significantly in Endgame
	if (mg_value(whiteScore) > mg_value(blackScore) + 1500 && eg_value(whiteScore) > eg_value(blackScore) + 1500) {
		// Reward pushing Black king to edge
		int centerDist = dist_from_center(blackKing);
		// Reward White king getting closer
		int closeDist = 14 - dist_between(whiteKing, blackKing);
		
		eg += Value(centerDist * MopUp_CenterWeight + closeDist * MopUp_CloseWeight);
	}
	// If Black is winning significantly
	else if (mg_value(blackScore) > mg_value(whiteScore) + 1500 && eg_value(blackScore) > eg_value(whiteScore) + 1500) {
		// Reward pushing White king to edge
		int centerDist = dist_from_center(whiteKing);
		// Reward Black king getting closer
		int closeDist = 14 - dist_between(blackKing, whiteKing);

		eg -= Value(centerDist * MopUp_CenterWeight + closeDist * MopUp_CloseWeight);
	}

	// 3. Simple Phase Calculation
	int npm = 0; // Non-pawn material
	npm += count(pos, WHITE, KNIGHT) * KnightValueMg;
	npm += count(pos, WHITE, BISHOP) * BishopValueMg;
	npm += count(pos, WHITE, ROOK) * RookValueMg;
	npm += count(pos, WHITE, QUEEN) * QueenValueMg;
	npm += count(pos, BLACK, KNIGHT) * KnightValueMg;
	npm += count(pos, BLACK, BISHOP) * BishopValueMg;
	npm += count(pos, BLACK, ROOK) * RookValueMg;
	npm += count(pos, BLACK, QUEEN) * QueenValueMg;

	int phase = std::min(npm, (int)MidgameLimit); 
	phase = std::max(phase, (int)EndgameLimit);
	int p = ((phase - EndgameLimit) * 128) / (MidgameLimit - EndgameLimit);

	Value v = Value((mg * p + eg * (128 - p)) / 128);

	// 4. Tempo Bonus
	if (us == WHITE) v += TEMPO;
	else v -= TEMPO;

	return (us == WHITE) ? v : -v;
}

