#ifndef BITBOARD_H_INCLUDED
#define BITBOARD_H_INCLUDED

#include "types.h"
#include <cassert>

// 0x0101010101010101 is File A, shifted left for B, C, etc.
constexpr Bitboard FileMask[FILE_NB] = {
	0x0101010101010101ULL, 0x0202020202020202ULL, 
	0x0404040404040404ULL, 0x0808080808080808ULL, 
	0x1010101010101010ULL, 0x2020202020202020ULL, 
	0x4040404040404040ULL, 0x8080808080808080ULL
};

// 0xFF is Rank 1, shifted left for Rank 2, 3, etc.
constexpr Bitboard RankMask[RANK_NB] = {
	0x00000000000000FFULL, 0x000000000000FF00ULL, 
	0x0000000000FF0000ULL, 0x00000000FF000000ULL, 
	0x000000FF00000000ULL, 0x0000FF0000000000ULL, 
	0x00FF000000000000ULL, 0xFF00000000000000ULL
};

// --- THÊM ĐOẠN NÀY ĐỂ HỖ TRỢ WINDOWS (MSVC) ---
#ifdef _MSC_VER
#include <intrin.h>
#pragma intrinsic(_BitScanForward64)
#pragma intrinsic(__popcnt)
#pragma intrinsic(__popcnt64)

// Định nghĩa lại các hàm của Linux sang Windows
inline int __builtin_ctzll(unsigned long long mask) {
    unsigned long index;
    if (_BitScanForward64(&index, mask))
        return (int)index;
    return 0;
}

#define __builtin_popcount __popcnt
#define __builtin_popcountll __popcnt64
#endif
// ----------------------------------------------

#define act_bit(b, i) do { (b) |= 1ULL << (i); } while(0)
#define dec_bit(b, i) do { (b) &= ~(1ULL << (i)); } while (0)
#define hav_bit(b, i) ((b) & (1ULL << (i)))

inline Square lsb(Bitboard b) {
#ifdef _MSC_VER
    unsigned long index;
    if (_BitScanForward64(&index, b))
        return Square(index);
    return SQ_NONE; // Or handle 0 case
#else
    return Square(__builtin_ctzll(b));
#endif
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

extern Bitboard PseudoAttacks[PIECE_TYPE_NB][SQ_NB];
extern Bitboard PawnAttacks[COLOR_NB][SQ_NB];

namespace bitboard {
	void init();
}

#endif
