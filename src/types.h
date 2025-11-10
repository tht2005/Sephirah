#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include <cstdint>

typedef uint64_t Bitboard;
typedef uint64_t Key;
typedef uint64_t Value;

enum Color : int {
	WHITE = 0,
	BLACK = 1,
	COLOR_NB,
};

enum PieceType : int {
	NO_PIECE_TYPE = 0,
	PAWN, KING, ROOK, KNIGHT, BISHOP, QUEEN,
	PIECE_TYPE_NB,
};

enum Piece : int {
	NO_PIECE = 0,
	W_PAWN = (WHITE << 3) + PAWN, W_KING, W_ROOK, W_KNIGHT, W_BISHOP, W_QUEEN,
	B_PAWN = (BLACK << 3) + PAWN, B_KING, B_ROOK, B_KNIGHT, B_BISHOP, B_QUEEN,
	ALL_PIECE = 15,
	PIECE_NB = 16,
};

enum Rank : int {
	RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8,
	RANK_NB,
};

enum File : int {
	FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, 
	FILE_NB,
};

enum Square : int {
	SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
	SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
	SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
	SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
	SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
	SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
	SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
	SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
	SQ_NB,
};

enum CastlingRights : int {
	NO_CASTLING_RIGHT = 0,
	WHITE_OO = 1 << 0,
	WHITE_OOO = 1 << 1,
	BLACK_OO = 1 << 2,
	BLACK_OOO = 1 << 3,
	WHITE_SIDE = WHITE_OO | WHITE_OOO,
	BLACK_SIDE = BLACK_OO | BLACK_OOO,
	KING_SIDE = WHITE_OO | BLACK_OO,
	QUEEN_SIDE = WHITE_OOO | BLACK_OOO,
	CASTLING_RIGHT_NB = 16,
};

/// A move needs 16 bits to be stored
///
/// bit  0- 5: destination square (from 0 to 63)
/// bit  6-11: origin square (from 0 to 63)
/// bit 12-13: promotion piece type - 2 (from KNIGHT-2 to QUEEN-2)
/// bit 14-15: special move flag: promotion (1), en passant (2), castling (3)
/// NOTE: EN-PASSANT bit is set only when a pawn can be captured
///
/// Special cases are MOVE_NONE and MOVE_NULL. We can sneak these in because in
/// any normal move destination square is always different from origin square
/// while MOVE_NONE and MOVE_NULL have the same origin and destination square.
enum Move : int {
	MOVE_NONE,
	MOVE_NULL = 65,
};

enum MoveType : int {
	NORMAL,
	PROMOTION = 1 << 14,
	ENPASSANT = 2 << 14,
	CASTLING  = 3 << 14
};

enum Score : int {
	SCORE_ZERO = 0,
};

enum BoundType : int {
	BOUND_NONE,
	BOUND_EXACT,
	BOUND_LOWER,
	BOUND_UPPER,
};

constexpr uint8_t make_genbound(int generation, bool pv, BoundType b) {
	return generation | (pv << 5) | (b << 6);
} 
constexpr uint8_t get_generation(uint8_t genbound) {
	return genbound & 0b11111;
}
constexpr bool get_pv(uint8_t genbound) {
	return (genbound >> 5) & 1;
}
constexpr BoundType get_bound_type(uint8_t genbound) {
	return BoundType((genbound >> 6) & 0b11);
}

constexpr Square make_square(File f, Rank r) {
	return Square((r << 3) + f);
}

constexpr Piece make_piece(Color c, PieceType pt) {
	return Piece((c << 3) + pt);
}

constexpr Square from_sq(Move m) {
	return Square((m >> 6) & 0x3F);
}

constexpr Square to_sq(Move m) {
	return Square(m & 0x3F);
}

constexpr int from_to(Move m) {
	return m & 0xFFF;
}

constexpr MoveType type_of(Move m) {
	return MoveType(m & (3 << 14));
}

constexpr PieceType promotion_type(Move m) {
	return PieceType(((m >> 12) & 3) + KNIGHT);
}

constexpr Move make_move(Square from, Square to) {
	return Move((from << 6) + to);
}

#endif
