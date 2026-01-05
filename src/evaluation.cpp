#include "evaluation.h"
#include "bitboard.h"
#include "position.h"
#include "types.h"
#include <algorithm>

// --- Constants & Weights ---

// Helper to create Score (MG, EG)
constexpr Score S(int mg, int eg) {
	return make_score(mg, eg);
}

// Material adjustments (Bishop Pair)
constexpr Score BonusBishopPair = S(30, 50);

// Pawn Structure Weights
constexpr Score ScoreIsolated    = S(-10, -10);
constexpr Score ScoreDoubled     = S(-10, -20);
constexpr Score ScoreBackward    = S(-15, -10);
constexpr Score ScorePawnShield  = S(10, 0); // Bonus for having a pawn shield
constexpr Score ScoreNoPawnShield = S(-20, -5); // Penalty for missing shield

// Passed Pawn Bonus by Rank (0-7)
constexpr Score BonusPassedPawn[RANK_NB] = {
	S(0, 0), S(5, 10), S(10, 20), S(20, 40), S(40, 70), S(80, 120), S(150, 200), S(0, 0)
};

// Piece Activity
constexpr Score BonusRookOpenFile     = S(30, 15);
constexpr Score BonusRookSemiOpenFile = S(15, 10);
constexpr Score BonusKnightOutpost    = S(30, 10);
constexpr Score BonusBishopOutpost    = S(20, 10);
constexpr Score BonusRookOn7th        = S(20, 40);
constexpr Score BonusKingSafety       = S(20, 5);

// Mobility Weights (Simple count of available squares)
constexpr Score MobilityKnight = S(4, 4);
constexpr Score MobilityBishop = S(4, 4);
constexpr Score MobilityRook   = S(3, 4);
constexpr Score MobilityQueen  = S(2, 4);

// King Safety
constexpr Score KingAttackWeight = S(2, 0); // Per attacker/attacked square

// Penalty
constexpr Score PenaltyKnightOnRim = S(20, 5);
constexpr Score PenaltyEarlyQueen = S(15, 0);

// --- Helper Functions ---

// Bitboard helpers that might be missing in bitboard.h but are needed here
namespace {
	inline Bitboard file_bb(File f) { return FileMask[f]; }
	inline Bitboard rank_bb(Rank r) { return RankMask[r]; }

	inline Bitboard adjacent_files_bb(File f) {
		Bitboard b = 0;
		if (f > FILE_A) b |= FileMask[f - 1];
		if (f < FILE_H) b |= FileMask[f + 1];
		return b;
	}

	inline Bitboard forward_ranks_bb(Color c, Rank r) {
		Bitboard b = 0;
		if (c == WHITE) {
			for (int i = r + 1; i < RANK_NB; ++i) b |= RankMask[i];
		} else {
			for (int i = r - 1; i >= RANK_1; --i) b |= RankMask[i];
		}
		return b;
	}

	inline Bitboard in_front_bb(Color c, Square s) {
		return forward_ranks_bb(c, get_rank(s)) & file_bb(get_file(s));
	}
	
	// Simple sliding attack generation for evaluation (since Magic Bitboards aren't in the project)
	Bitboard get_sliding_attacks(PieceType pt, Square sq, Bitboard occ) {
		Bitboard attacks = 0;
		const int (*dirs)[2];
		int num_dirs = 0;

		static const int r_dirs[4][2] = {{0,1},{0,-1},{1,0},{-1,0}};
		static const int b_dirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
		
		if (pt == ROOK) { dirs = r_dirs; num_dirs = 4; }
		else if (pt == BISHOP) { dirs = b_dirs; num_dirs = 4; }
		else { // Queen
			 // Combine both, but for simplicity in this helper, handle separately or merge
			 return get_sliding_attacks(ROOK, sq, occ) | get_sliding_attacks(BISHOP, sq, occ);
		}

		for(int i=0; i<num_dirs; ++i) {
			Square t = sq;
			while(true) {
				t = advance(t, dirs[i][0], dirs[i][1]);
				if (!is_ok(t)) break;
				act_bit(attacks, t);
				if (hav_bit(occ, t)) break;
			}
		}
		return attacks;
	}
}

// --- Evaluation Class ---

struct EvalInfo {
	const Position& pos;
	Bitboard pawns[COLOR_NB];
	Bitboard pieces[COLOR_NB];
	Bitboard mobilityArea[COLOR_NB];
	Bitboard kingRing[COLOR_NB];
	Bitboard attackedBy[COLOR_NB][PIECE_TYPE_NB]; // [Color][AttackerType]
	Bitboard allAttackedBy[COLOR_NB];
	
	Score score;

	EvalInfo(const Position& p) : pos(p), score(SCORE_ZERO) {
		pawns[WHITE] = pos.pieces(WHITE, PAWN);
		pawns[BLACK] = pos.pieces(BLACK, PAWN);
		pieces[WHITE] = pos.pieces(WHITE);
		pieces[BLACK] = pos.pieces(BLACK);
		
		// Initialize mobility area: squares not attacked by enemy pawns
		// and not occupied by our own pawns (blocked)
		mobilityArea[WHITE] = ~(pieces[WHITE] & pawns[WHITE]); // Exclude own pawns
		mobilityArea[BLACK] = ~(pieces[BLACK] & pawns[BLACK]);
		
		// Calculate Pawn Attacks first as they define safe mobility area
		allAttackedBy[WHITE] = 0;
		allAttackedBy[BLACK] = 0;
		
		// We need to fill attackedBy for pawns to exclude from mobility
		// Note: This is expensive to do fully, we approximate.
	}
};

// --- Evaluation Terms ---

void eval_pawns(EvalInfo& ei, Color us) {
	Color them = flip_color(us);
	Bitboard ourPawns = ei.pawns[us];
	Bitboard theirPawns = ei.pawns[them];
	Direction up = push_pawn(us);

	Bitboard b = ourPawns;
	while (b) {
		Square s = pop_lsb(b);
		File f = get_file(s);
		Rank r = get_rank(s);

		// 1. Isolated Pawn
		// No pawns on adjacent files
		if ((ei.pawns[us] & adjacent_files_bb(f)) == 0) {
			ei.score += (us == WHITE ? ScoreIsolated : -ScoreIsolated);
		}

		// 2. Doubled Pawn
		// Another pawn of ours on the same file
		if ((ei.pawns[us] & file_bb(f) & ~square_bb(s))) {
			 // We only penalize the rear pawn usually, or just count all. 
			 // Simple approach: penalize every doubled pawn.
			 ei.score += (us == WHITE ? ScoreDoubled : -ScoreDoubled);
		}

		// 3. Passed Pawn
		// No enemy pawns in front on the same file or adjacent files
		Bitboard frontSpan = forward_ranks_bb(us, r);
		Bitboard span = frontSpan & (file_bb(f) | adjacent_files_bb(f));
		
		if ((span & theirPawns) == 0) {
			Score bonus = BonusPassedPawn[r];
			
			// Bonus increases if the passed pawn is supported or blockaded?
			// For now, simple rank-based bonus.
			ei.score += (us == WHITE ? bonus : -bonus);
		}
		
		// 4. Backward Pawn (Simplified)
		// Cannot advance safely and no support from behind
		// (Omitted for brevity/complexity, relying on Isolated/Doubled for structure)
	}
}

void eval_pieces(EvalInfo& ei, Color us) {
	Color them = flip_color(us);
	Bitboard occupied = ei.pos.pieces();
	Bitboard ourPawns = ei.pawns[us];
	Bitboard theirPawns = ei.pawns[them];
	
	// Outpost ranks: 4, 5, 6 (relative)
	Rank r4 = (us == WHITE ? RANK_4 : RANK_5);
	Rank r5 = (us == WHITE ? RANK_5 : RANK_4);
	Rank r6 = (us == WHITE ? RANK_6 : RANK_3);
	Bitboard outpostRanks = rank_bb(r4) | rank_bb(r5) | rank_bb(r6);

	// --- Knights ---
	Bitboard knights = ei.pos.pieces(us, KNIGHT);
	while (knights) {
		Square s = pop_lsb(knights);
		
		// Mobility
		Bitboard attacks = PseudoAttacks[KNIGHT][s];
		int mob = __builtin_popcountll(attacks & ei.mobilityArea[us]);
		ei.score += (us == WHITE ? MobilityKnight * mob : -MobilityKnight * mob);

		// penalize knights on rim
		File f = get_file(s);
		if (f == FILE_A || f == FILE_H) {
			ei.score -= (us == WHITE ? PenaltyKnightOnRim : -PenaltyKnightOnRim);
		}

		// Outpost
		if (hav_bit(outpostRanks, s)) {
			// Supported by pawn
			if (PawnAttacks[them][s] & ourPawns) { // PawnAttacks[them][s] gives squares that attack s from 'them' perspective? 
				// No, PawnAttacks[c][s] usually means squares attacked BY pawns of color c located at s.
				// We need to check if 's' is attacked by 'our' pawns.
				// That is equivalent to: is there a pawn of 'us' that attacks 's'?
				// Which is: PawnAttacks[them][s] & ourPawns. (Geometry is symmetric).
				ei.score += (us == WHITE ? BonusKnightOutpost : -BonusKnightOutpost);
			}
		}
	}

	// --- Bishops ---
	Bitboard bishops = ei.pos.pieces(us, BISHOP);
	if (__builtin_popcountll(bishops) >= 2) {
		ei.score += (us == WHITE ? BonusBishopPair : -BonusBishopPair);
	}

	while (bishops) {
		Square s = pop_lsb(bishops);
		
		// Mobility (Pseudo)
		Bitboard attacks = get_sliding_attacks(BISHOP, s, occupied);
		int mob = __builtin_popcountll(attacks & ei.mobilityArea[us]);
		ei.score += (us == WHITE ? MobilityBishop * mob : -MobilityBishop * mob);
	}

	// --- Rooks ---
	Bitboard rooks = ei.pos.pieces(us, ROOK);
	while (rooks) {
		Square s = pop_lsb(rooks);
		File f = get_file(s);
		Rank r = get_rank(s);

		// Open / Semi-Open Files
		bool ourPawn = (file_bb(f) & ourPawns);
		bool theirPawn = (file_bb(f) & theirPawns);

		if (!ourPawn) {
			if (!theirPawn) {
				ei.score += (us == WHITE ? BonusRookOpenFile : -BonusRookOpenFile);
			} else {
				ei.score += (us == WHITE ? BonusRookSemiOpenFile : -BonusRookSemiOpenFile);
			}
		}

		// Rook on 7th Rank (relative)
		Rank r7 = (us == WHITE ? RANK_7 : RANK_2);
		if (r == r7) {
			// Bonus if enemy king is on rank 8 or cut off
			ei.score += (us == WHITE ? BonusRookOn7th : -BonusRookOn7th);
		}

		// Mobility
		Bitboard attacks = get_sliding_attacks(ROOK, s, occupied);
		int mob = __builtin_popcountll(attacks & ei.mobilityArea[us]);
		ei.score += (us == WHITE ? MobilityRook * mob : -MobilityRook * mob);
	}
	
	// --- Queens ---
	Bitboard queens = ei.pos.pieces(us, QUEEN);

	// early queen penalty
	Bitboard backRank = (us == WHITE ? RankMask[RANK_1] : RankMask[RANK_8]);
	Bitboard minorsOnBackRank = ei.pos.pieces(us, KNIGHT) | ei.pos.pieces(us, BISHOP);
	minorsOnBackRank &= backRank;
	int undevelopedMinors = __builtin_popcountll(minorsOnBackRank);

	while (queens) {
		Square s = pop_lsb(queens);
		Bitboard attacks = get_sliding_attacks(QUEEN, s, occupied);
		int mob = __builtin_popcountll(attacks & ei.mobilityArea[us]);
		ei.score += (us == WHITE ? MobilityQueen * mob : -MobilityQueen * mob);

		// Apply Penalty if Queen has moved but minors are sleeping
		// We assume if the queen is NOT on her starting square (D1/D8), she moved.
		Square startSq = (us == WHITE ? SQ_D1 : SQ_D8);
		if (s != startSq && undevelopedMinors > 1) {
			 ei.score -= (us == WHITE ? PenaltyEarlyQueen * undevelopedMinors : -PenaltyEarlyQueen * undevelopedMinors);
		}
	}
}

void eval_king_safety(EvalInfo& ei, Color us) {
	Color them = flip_color(us);
	Square ksq = lsb(ei.pos.pieces(us, KING));

	// If King is on files G or B (Kingside) or C (Queenside)
	// AND it is on the back rank, give a bonus.
	const File f = get_file(ksq);
	const Rank r = get_rank(ksq);
	Rank backRank = (us == WHITE ? RANK_1 : RANK_8);

	if (r == backRank) {
		// G-file (Kingside), B/C-file (Queenside)
		if (f == FILE_G || f == FILE_B || f == FILE_C) {
			ei.score += (us == WHITE ? BonusKingSafety : -BonusKingSafety);
		}
	}
	
	// 1. Pawn Shield (Mainly MG)
	// Check pawns in front of the king
	Bitboard shieldMask = 0;
	
	// Define shield squares based on King rank (usually 1 or 2 for white)
	if (us == WHITE ? r <= RANK_2 : r >= RANK_7) {
		// Check 3 files around king, rank + 1
		Rank r_shield = Rank(us == WHITE ? r + 1 : r - 1);
		if (r_shield >= RANK_1 && r_shield <= RANK_8) {
			if (f > FILE_A) act_bit(shieldMask, make_square(File(f-1), r_shield));
			act_bit(shieldMask, make_square(f, r_shield));
			if (f < FILE_H) act_bit(shieldMask, make_square(File(f+1), r_shield));
		}
		
		// Count pawns in shield
		int shieldCount = __builtin_popcountll(shieldMask & ei.pawns[us]);
		if (shieldCount == 3) ei.score += (us == WHITE ? ScorePawnShield : -ScorePawnShield);
		else if (shieldCount < 2) ei.score += (us == WHITE ? ScoreNoPawnShield : -ScoreNoPawnShield);
	}

	// 2. King Safety / Attacked Squares (Simplified)
	// Identify a "King Ring" around the king
	Bitboard kingRing = PseudoAttacks[KING][ksq];
	
	// Check for enemy pieces attacking the ring
	// Note: This requires generating attacks for all enemy pieces, which is expensive.
	// We rely on a simplified check or skip if performance is key.
	// For this implementation, we will skip full attack generation to keep it fast,
	// relying on the Pawn Shield and general activity.
}

// --- Main Evaluation Function ---

Value eval(const Position &pos) {
	EvalInfo ei(pos);

	// 1. Material & PSQT (Base)
	// This is usually incrementally updated, but here we calculate from scratch
	// or rely on what's available. The provided code in evaluation.cpp
	// calculated this manually. We will do the same but accumulate into Score.
	
	for (PieceType pt = PAWN; pt <= KING; ++pt) {
		Bitboard w = pos.pieces(WHITE, pt);
		Bitboard b = pos.pieces(BLACK, pt);
		
		while (w) {
			Square s = pop_lsb(w);
			ei.score += PSQT::psq[make_piece(WHITE, pt)][s];
		}
		while (b) {
			Square s = pop_lsb(b);
			ei.score -= PSQT::psq[make_piece(BLACK, pt)][s];
		}
	}

	// 2. Pawn Structure
	eval_pawns(ei, WHITE);
	eval_pawns(ei, BLACK);

	// 3. Piece Activity & Structure
	eval_pieces(ei, WHITE);
	eval_pieces(ei, BLACK);

	// 4. King Safety
	eval_king_safety(ei, WHITE);
	eval_king_safety(ei, BLACK);

	// 5. Tapered Evaluation (Phase Calculation)
	// Calculate Phase
	int npm = 0; // Non-pawn material
	npm += __builtin_popcountll(pos.pieces(KNIGHT)) * KnightValueMg;
	npm += __builtin_popcountll(pos.pieces(BISHOP)) * BishopValueMg;
	npm += __builtin_popcountll(pos.pieces(ROOK))   * RookValueMg;
	npm += __builtin_popcountll(pos.pieces(QUEEN))  * QueenValueMg;

	int phase = std::min(npm, (int)MidgameLimit); 
	phase = std::max(phase, (int)EndgameLimit);
	
	// Phase factor: 0 (Endgame) to 128 (Midgame)
	int p = ((phase - EndgameLimit) * 128) / (MidgameLimit - EndgameLimit);

	// Interpolate
	Value mg = mg_value(ei.score);
	Value eg = eg_value(ei.score);
	
	Value v = Value((mg * p + eg * (128 - p)) / 128);

	// 6. Tempo
	const Value TEMPO = Value(20);
	if (pos.side_to_move() == WHITE) v += TEMPO;
	else v -= TEMPO;

	return (pos.side_to_move() == WHITE) ? v : -v;
}
