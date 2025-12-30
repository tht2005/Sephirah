#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include <assert.h>
#include <cctype>
#include <cstdint>
#include <string>

typedef uint64_t Bitboard;
typedef uint64_t Key;

constexpr int MAX_MOVES = 256;
constexpr int MAX_PLY   = 246;

enum Color : int {
	WHITE = 0,
	BLACK = 1,
	COLOR_NB,
};

enum PieceType : int {
	NO_PIECE_TYPE = 0,
	PAWN, KNIGHT, ROOK, BISHOP, QUEEN, KING,
	ALL_PIECE = 7,
	PIECE_TYPE_NB =8,
};

enum Piece : int {
	NO_PIECE = 0,
	W_PAWN = (WHITE << 3) + PAWN, W_KNIGHT, W_ROOK, W_BISHOP, W_QUEEN, W_KING,
	B_PAWN = (BLACK << 3) + PAWN, B_KNIGHT, B_ROOK, B_BISHOP, B_QUEEN, B_KING,
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
	SQ_NONE,
	SQ_NB = 64,
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

/// Score enum stores a middlegame and an endgame value in a single integer (enum).
/// The least significant 16 bits are used to store the middlegame value and the
/// upper 16 bits are used to store the endgame value. We have to take care to
/// avoid left-shifting a signed int to avoid undefined behavior.
enum Score : int {
	SCORE_ZERO = 0,
};

enum Bound : int {
	BOUND_NONE,
	BOUND_LOWER,
	BOUND_UPPER,
	BOUND_EXACT = BOUND_LOWER | BOUND_UPPER,
};

enum Value : int {
	VALUE_ZERO      = 0,
	VALUE_DRAW      = 0,
	VALUE_KNOWN_WIN = 10000,
	VALUE_MATE      = 32000,
	VALUE_INFINITE  = 32001,
	VALUE_NONE      = 32002,

	VALUE_MATE_IN_MAX_PLY  =  VALUE_MATE - 2 * MAX_PLY,
	VALUE_MATED_IN_MAX_PLY = -VALUE_MATE + 2 * MAX_PLY,

	PawnValueMg   = 128,   PawnValueEg   = 213,
	KnightValueMg = 781,   KnightValueEg = 854,
	BishopValueMg = 825,   BishopValueEg = 915,
	RookValueMg   = 1276,  RookValueEg   = 1380,
	QueenValueMg  = 2538,  QueenValueEg  = 2682,

	MidgameLimit  = 15258, EndgameLimit  = 3915
};

enum Direction : int {
	NORTH = 8,
	EAST = 1,
	SOUTH = -NORTH,
	WEST = -EAST,
	SOUTH_EAST = SOUTH + EAST,
	SOUTH_WEST = SOUTH + WEST,
	NORTH_EAST = NORTH + EAST,
	NORTH_WEST = NORTH + WEST,
};

#define ENABLE_BASE_OPERATORS_ON(T)                                \
constexpr T operator+(T d1, T d2) { return T(int(d1) + int(d2)); } \
constexpr T operator-(T d1, T d2) { return T(int(d1) - int(d2)); } \
constexpr T operator-(T d) { return T(-int(d)); }                  \
inline T& operator+=(T& d1, T d2) { return d1 = d1 + d2; }         \
inline T& operator-=(T& d1, T d2) { return d1 = d1 - d2; }

#define ENABLE_INCR_OPERATORS_ON(T)                                \
inline T& operator++(T& d) { return d = T(int(d) + 1); }           \
inline T& operator--(T& d) { return d = T(int(d) - 1); }

#define ENABLE_FULL_OPERATORS_ON(T)                                \
ENABLE_BASE_OPERATORS_ON(T)                                        \
constexpr T operator*(int i, T d) { return T(i * int(d)); }        \
constexpr T operator*(T d, int i) { return T(int(d) * i); }        \
constexpr T operator/(T d, int i) { return T(int(d) / i); }        \
constexpr int operator/(T d1, T d2) { return int(d1) / int(d2); }  \
inline T& operator*=(T& d, int i) { return d = T(int(d) * i); }    \
inline T& operator/=(T& d, int i) { return d = T(int(d) / i); }

ENABLE_FULL_OPERATORS_ON(Square);
ENABLE_INCR_OPERATORS_ON(Square);

ENABLE_INCR_OPERATORS_ON(PieceType);

ENABLE_FULL_OPERATORS_ON(File);
ENABLE_FULL_OPERATORS_ON(Rank);
ENABLE_INCR_OPERATORS_ON(File);
ENABLE_INCR_OPERATORS_ON(Rank);

ENABLE_FULL_OPERATORS_ON(Direction);

inline Move& operator|= (Move& m, MoveType t) {
	m = Move( int(m) | int(t) );
	return m;
}

template<typename T>
inline Square& operator+= (Square& sq, T d) {
	sq = Square(sq + d);
	return sq;
}
template<typename T>
inline Square& operator-= (Square& sq, T d) {
	sq = Square(sq - d);
	return sq;
}

template<typename T>
inline Square operator+ (Square sq, T d) {
	sq = Square(int(sq) + int(d));
	return sq;
}
template<typename T>
inline Square operator- (Square sq, T d) {
	sq = Square(int(sq) - int(d));
	return sq;
}

template <typename T>
constexpr uint8_t distance(T a, T b) {
	return (a < b) ? b - a : a - b;
}

constexpr Direction push_pawn(Color c) {
	return (c == WHITE) ? NORTH : SOUTH;
}

constexpr Score make_score(int mg, int eg) {
	return Score((int)((unsigned int)eg << 16) + mg);
}

/// Extracting the signed lower and upper 16 bits is not so trivial because
/// according to the standard a simple cast to short is implementation defined
/// and so is a right shift of a signed integer.
inline Value eg_value(Score s) {
	union { uint16_t u; int16_t s; } eg = { uint16_t(unsigned(s + 0x8000) >> 16) };
	return Value(eg.s);
}

inline Value mg_value(Score s) {
	union { uint16_t u; int16_t s; } mg = { uint16_t(unsigned(s)) };
	return Value(mg.s);
}

constexpr uint8_t make_genbound(int generation, bool pv, Bound b) {
	return generation | (pv << 5) | (b << 6);
} 
constexpr uint8_t get_generation(uint8_t genbound) {
	return genbound & 0b11111;
}
constexpr bool get_pv(uint8_t genbound) {
	return (genbound >> 5) & 1;
}
constexpr Bound get_bound_type(uint8_t genbound) {
	return Bound((genbound >> 6) & 0b11);
}

constexpr Square make_square(File f, Rank r) {
	return Square((r << 3) + f);
}
constexpr Rank get_rank(Square sq) {
	return Rank(sq >> 3);
}
constexpr File get_file(Square sq) {
	return File(sq & 7);
}

constexpr Piece make_piece(Color c, PieceType pt) {
	return Piece((c << 3) + pt);
}
constexpr Color get_color(Piece pc) {
	return Color(pc >> 3);
}
constexpr PieceType get_piece_type(Piece pc) {
	return PieceType(pc & 7);
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
	return PieceType(((m >> 12) & 3) + 2);
}

constexpr Move act_promotion_type(Move m, PieceType pro_type) {
	int bits = (pro_type - 2) << 12;
	return Move(m | bits);
}

constexpr Move make_move(Square from, Square to) {
	return Move((from << 6) + to);
}

constexpr CastlingRights get_side(Color c) {
	return (c == WHITE) ? WHITE_SIDE : BLACK_SIDE;
}

constexpr CastlingRights get_rook_side(Square sq) {
	return (get_file(sq) == FILE_A) ? QUEEN_SIDE : KING_SIDE;
}

constexpr Color flip_color(Color c) {
	int i = int(WHITE) + int(BLACK) - int(c);
	return Color(i);
}

constexpr CastlingRights char_to_castling_rights(char c) {
	switch (c) {
		case 'q': return BLACK_OOO;
		case 'k': return BLACK_OO;
		case 'Q': return WHITE_OOO;
		case 'K': return WHITE_OO;
		default: return NO_CASTLING_RIGHT;
	}
}

constexpr bool is_ok(Square s) {
	return s >= SQ_A1 && s <= SQ_H8;
}

inline Square str_to_square(std::string str) {
	assert(str.size() == 2);
	str[0] = tolower(str[0]);
	assert('a' <= str[0] && str[0] <= 'h');
	assert('1' <= str[1] && str[1] <= '8');
	File f = File(FILE_A + (str[0] - 'a'));
	Rank r = Rank(RANK_1 + (str[1] - '1'));
	return make_square(f, r);
}

inline std::string square_to_str(Square sq) {
	File f = get_file(sq);
	Rank r = get_rank(sq);
	char cf = 'A' + f;
	char cr = '1' + r;
	return std::string({cf, cr});
}

constexpr bool valid_square(Square s) {
	return SQ_A1 <= s && s < SQ_NB;
}

constexpr Rank get_initial_king_rank(Color c) {
	return (c == WHITE) ? RANK_1 : RANK_8;
}

constexpr Rank get_initial_pawn_rank(Color c) {
	return (c == WHITE) ? RANK_2 : RANK_7;
}

constexpr bool is_ok(Move m) {
	return from_sq(m) != to_sq(m); // Catch MOVE_NULL and MOVE_NONE
}

constexpr Piece char_to_piece (char c) {
	switch (c) {
		case 'p': return B_PAWN;
		case 'r': return B_ROOK;
		case 'n': return B_KNIGHT;
		case 'b': return B_BISHOP;
		case 'q': return B_QUEEN;
		case 'k': return B_KING;
		case 'P': return W_PAWN;
		case 'R': return W_ROOK;
		case 'N': return W_KNIGHT;
		case 'B': return W_BISHOP;
		case 'Q': return W_QUEEN;
		case 'K': return W_KING;
	}
	assert(0);
}

constexpr char piece_to_char (Piece pc) {
	Color cl = get_color(pc);
	PieceType pt = get_piece_type(pc);
	char c = 0;
	switch (pt) {
		case PAWN: c = 'p'; break;
		case ROOK: c = 'r'; break;
		case KNIGHT: c = 'n'; break;
		case BISHOP: c = 'b'; break;
		case QUEEN: c = 'q'; break;
		case KING: c = 'k'; break;
		default: assert(0);
	}
	if (cl == WHITE) c = toupper(c);
	return c;
}

#endif
