#include "bitboard.h"
#include "types.h"
#include <cstring>

Bitboard PseudoAttacks[PIECE_TYPE_NB][SQ_NB];
Bitboard PawnAttacks[COLOR_NB][SQ_NB];

namespace bitboard {

Bitboard safe_step(Square s, int df, int dr) {
	File f = File(get_file(s) + df);
	Rank r = Rank(get_rank(s) + dr);
	if (f < FILE_A || f > FILE_H || r < RANK_1 || r > RANK_8) return 0;
	return square_bb(make_square(f, r));
}

void init() {
	memset(PseudoAttacks, 0, sizeof(PseudoAttacks));
	memset(PawnAttacks, 0, sizeof(PawnAttacks));

	for (int s = 0; s < SQ_NB; ++s) {
		Square sq = Square(s);

		// 1. KNIGHT Attacks
		int k_steps[8][2] = {{1,2}, {2,1}, {2,-1}, {1,-2}, {-1,-2}, {-2,-1}, {-2,1}, {-1,2}};
		for (auto& step : k_steps) 
			PseudoAttacks[KNIGHT][s] |= safe_step(sq, step[0], step[1]);

		// 2. KING Attacks
		int ki_steps[8][2] = {{0,1}, {0,-1}, {1,0}, {-1,0}, {1,1}, {1,-1}, {-1,1}, {-1,-1}};
		for (auto& step : ki_steps) 
			PseudoAttacks[KING][s] |= safe_step(sq, step[0], step[1]);

		// 3. PAWN Attacks (Captures only)
		// White pawns capture NorthEast (+1, +1) and NorthWest (-1, +1)
		PawnAttacks[WHITE][s] |= safe_step(sq, 1, 1);
		PawnAttacks[WHITE][s] |= safe_step(sq, -1, 1);

		// Black pawns capture SouthEast (+1, -1) and SouthWest (-1, -1)
		PawnAttacks[BLACK][s] |= safe_step(sq, 1, -1);
		PawnAttacks[BLACK][s] |= safe_step(sq, -1, -1);
	}
}

}
