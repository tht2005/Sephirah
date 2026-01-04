// temporary eval function, the complete function follows
// https://hxim.github.io/Stockfish-Evaluation-Guide/

#include "evaluation.h"
#include "bitboard.h"
#include "position.h"
#include "types.h"
#include <algorithm>

// Material Weights (Standardized)
// Already defined in types.h or can be local
const Value TEMPO = Value(20); // Bonus for side to move

// King Safety
const int KingAttackWeight = 20; // Weight per enemy piece attacking king zone
const int KingOpenShieldPenalty = 25; // Penalty for missing pawn shield
const int KingRingAttack = 15; // Attacking squares adjacent to king

// Castling
const int CastlingRightsBonus = 15; // Bonus for having rights
const int CastledBonus = 40;       // Bonus for actually being castled

// Threats
const int HangingBonus = 60;       // Penalty for undefended piece under attack
const int MinorAttackedByPawn = 40;
const int MajorAttackedByPawn = 60;
const int MajorAttackedByMinor = 40;

// Mobility Weights
const int MobilityBonus[] = { 0, 0, 4, 3, 2, 1, 0, 0 }; // P, N, B, R, Q, K

int count(const Position &pos, Color c, PieceType pt) {
	return __builtin_popcountll(pos.pieces(c, pt));
}

// Check if a piece at 'sq' is attacked by a pawn of color 'attacker'
bool is_attacked_by_pawn(const Position& pos, Square sq, Color attacker) {
	// A pawn attacks 'sq' if a pawn is at 'sq - pawn_push(attacker) +/- 1'
	Direction down = (attacker == WHITE) ? SOUTH : NORTH;

	// Check capture from left
	Square capture_left = sq + down + WEST;
	if (is_ok(capture_left) && abs(get_file(capture_left) - get_file(sq)) == 1) {
		if (pos.piece_on(capture_left) == make_piece(attacker, PAWN)) return true;
	}
	// Check capture from right
	Square capture_right = sq + down + EAST;
	if (is_ok(capture_right) && abs(get_file(capture_right) - get_file(sq)) == 1) {
		if (pos.piece_on(capture_right) == make_piece(attacker, PAWN)) return true;
	}
	return false;
}

Value eval(const Position &pos) {
	Score score = SCORE_ZERO;
	Color us = pos.side_to_move();
	Color them = flip_color(us);

	// 1. Material & PSQT (Base Score)
	Score whiteScore = SCORE_ZERO;
	Score blackScore = SCORE_ZERO;

	for (PieceType pt = PAWN; pt <= KING; ++pt) {
		Bitboard w = pos.pieces(WHITE, pt);
		Bitboard b = pos.pieces(BLACK, pt);
		
		while (w) {
			Square s = pop_lsb(w);
			whiteScore += PSQT::PieceValue[pt] + PSQT::psq[make_piece(WHITE, pt)][s];
		}
		while (b) {
			Square s = pop_lsb(b);
			blackScore += PSQT::PieceValue[pt] + PSQT::psq[make_piece(BLACK, pt)][s];
		}
	}

	// 2. Castling Safety
	// Reward retaining rights (flexibility)
	if (pos.castling_rights(WHITE) & KING_SIDE) whiteScore += make_score(CastlingRightsBonus, 0);
	if (pos.castling_rights(WHITE) & QUEEN_SIDE) whiteScore += make_score(CastlingRightsBonus, 0);
	if (pos.castling_rights(BLACK) & KING_SIDE) blackScore += make_score(CastlingRightsBonus, 0);
	if (pos.castling_rights(BLACK) & QUEEN_SIDE) blackScore += make_score(CastlingRightsBonus, 0);

	// Reward actually being castled (King tucked away)
	// Heuristic: King is on G or B file and Rooks are in place/moved
	Square wk = lsb(pos.pieces(WHITE, KING));
	if (get_rank(wk) == RANK_1 && (get_file(wk) == FILE_G || get_file(wk) == FILE_B)) 
		whiteScore += make_score(CastledBonus, 0);
		
	Square bk = lsb(pos.pieces(BLACK, KING));
	if (get_rank(bk) == RANK_8 && (get_file(bk) == FILE_G || get_file(bk) == FILE_B)) 
		blackScore += make_score(CastledBonus, 0);

	// 3. Pawn Structure (Simplified)
	// TODO: (Keep existing Doubled/Isolated/Passed logic if desired, simplified here for brevity)

	// 4. Threats & "Annoyance"
	// Check White pieces being attacked
	Bitboard whitePieces = pos.pieces(WHITE);
	while (whitePieces) {
		Square s = pop_lsb(whitePieces);
		PieceType pt = get_piece_type(pos.piece_on(s));
		
		// Is attacked by Black Pawn?
		if (pt > PAWN && is_attacked_by_pawn(pos, s, BLACK)) {
			// Penalty relative to piece value (Queen hates pawns more than Knights do)
			int penalty = (pt == QUEEN || pt == ROOK) ? MajorAttackedByPawn : MinorAttackedByPawn;
			whiteScore -= make_score(penalty, penalty / 2);
		}

		// Is attacked and undefended? (Hanging)
		if (pos.square_is_attacked(BLACK, s) && !pos.square_is_attacked(WHITE, s)) {
			whiteScore -= make_score(HangingBonus, HangingBonus);
		}
	}

	// Check Black pieces being attacked
	Bitboard blackPieces = pos.pieces(BLACK);
	while (blackPieces) {
		Square s = pop_lsb(blackPieces);
		PieceType pt = get_piece_type(pos.piece_on(s));
		
		if (pt > PAWN && is_attacked_by_pawn(pos, s, WHITE)) {
			int penalty = (pt == QUEEN || pt == ROOK) ? MajorAttackedByPawn : MinorAttackedByPawn;
			blackScore -= make_score(penalty, penalty / 2);
		}

		if (pos.square_is_attacked(WHITE, s) && !pos.square_is_attacked(BLACK, s)) {
			blackScore -= make_score(HangingBonus, HangingBonus);
		}
	}

	// 5. Calculate Final Score
	Score totalScore = whiteScore - blackScore;

	// 6. Game Phase Interpolation
	// Calculate Non-Pawn Material (NPM) to determine phase
	int npm = 0;
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

	// Scale phase: 0 = End, 128 = Mid
	int p = ((phase - EndgameLimit) * 128) / (MidgameLimit - EndgameLimit);

	Value mg = mg_value(totalScore);
	Value eg = eg_value(totalScore);

	Value v = Value((mg * p + eg * (128 - p)) / 128);

	// 7. Tempo Bonus
	// The side to move has a small advantage
	if (us == WHITE) v += TEMPO;
	else v -= TEMPO;

	return (us == WHITE) ? v : -v;
}

