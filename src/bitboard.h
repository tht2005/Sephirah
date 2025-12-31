#ifndef BITBOARD_H_INCLUDED
#define BITBOARD_H_INCLUDED

#include "types.h"
#include <cassert>

#define act_bit(b, i) do { (b) |= 1ULL << (i); } while(0)
#define dec_bit(b, i) do { (b) &= ~(1ULL << (i)); } while (0)
#define hav_bit(b, i) ((b) & (1ULL << (i)))

constexpr Square lsb(Bitboard b) {
	return Square(__builtin_ctzll(b));
}

inline Square pop_lsb(Bitboard& b) {
	Square s = lsb(b);
	b &= b - 1;
	return s;
}

constexpr Bitboard square_bb(Square sq) {
	return 1ULL << sq;
}

constexpr Bitboard path_bb(Square from, Square to) {
	Bitboard b = 0;
	int f_from = get_file(from);
	int r_from = get_rank(from);
	int f_to = get_file(to);
	int r_to = get_rank(to);
	assert(f_from == f_to || r_from == r_to);
	while (true) {
		b |= 1ULL << make_square(File(f_from), Rank(r_from));
		if (f_from < f_to) ++f_from;
		else if (f_from > f_to) --f_from;
		else if (r_from < r_to) ++r_from;
		else if (r_from > r_to) --r_from;
		else break;
	}
	return b;
}

#endif
