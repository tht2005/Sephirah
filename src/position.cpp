#include "position.h"
#include "bitboard.h"
#include "random.h"
#include "types.h"
#include <cassert>
#include <cctype>
#include <cstring>
#include <sstream>
#include <string>

constexpr Piece Pieces[] = {	W_PAWN, W_ROOK, W_KNIGHT, W_BISHOP, W_QUEEN, W_KING,
				B_PAWN, B_ROOK, B_KNIGHT, B_BISHOP, B_QUEEN, B_KING};

namespace Zobrist {
	Key psq[PIECE_NB][SQ_NB];
	Key enpassant[FILE_NB];
	Key castlingRights[CASTLING_RIGHT_NB];
	Key side, noPawn;
}

void Position::init() {
	for (int i = 0; i < PIECE_NB; ++i)
		for (int j = 0; j < SQ_NB; ++j)
			Zobrist::psq[i][j] = random_u64();
	for (int i = 0; i < FILE_NB; ++i)
		Zobrist::enpassant[i] = random_u64();
	for (int i = 0; i < CASTLING_RIGHT_NB; ++i)
		Zobrist::castlingRights[i] = random_u64();
	Zobrist::side = random_u64();
	Zobrist::noPawn = random_u64();
}

Piece get_piece(char c) {
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

Square get_square(std::string str) {
	assert(str.size() == 2);
	str[0] = tolower(str[0]);
	assert('a' <= str[0] && str[0] <= 'h');
	assert('1' <= str[1] && str[1] <= '8');
	File f = File(FILE_A + (str[0] - 'a'));
	Rank r = Rank(RANK_1 + (str[1] - '1'));
	return make_square(f, r);
}

void Position::set_check_info(StateInfo& st) {
	// TODO: set_check_info
	// Bitboard blockersForKing[COLOR_NB];
	// Bitboard pinners[COLOR_NB];
	// Bitboard checkSquares[PIECE_TYPE_NB];
}

void Position::set_state(StateInfo& st) {
	st.pawnKey = Zobrist::noPawn;
	// st.materialKey = st.key = 0;
	st.key = 0;
	// st.checkersBB =
	set_check_info(st);
	for (Bitboard b = pieces(); b; ) {
		Square sq = pop_lsb(b); // pop a bit of b
		Piece pc = board[sq];
		st.key ^= Zobrist::psq[pc][sq];
		if (get_piece_type(pc) == PAWN)
			st.pawnKey ^= Zobrist::psq[pc][sq];
		else
			// TODO: noPawnMaterial
			;
	}
	if (st.epSquare != SQ_NONE)
		st.key ^= Zobrist::enpassant[get_file(st.epSquare)];
	st.key ^= Zobrist::castlingRights[st.castlingRights];
	if (sideToMove == BLACK) st.key ^= Zobrist::side;
	// TODO: materialKey
}

void Position::set_castling_right(Color c, CastlingRights side) {
	assert(side == KING_SIDE || side == QUEEN_SIDE);
	Rank r = (c == WHITE) ? RANK_1 : RANK_8;
	File rookFile = (side == KING_SIDE) ? FILE_H : FILE_A;
	Square rfrom = make_square(rookFile, r);
	Square kfrom = (c == WHITE) ? SQ_E1 : SQ_E8;
	int cr = get_side(c) & side;

	castlingRightsMask[kfrom] |= cr;
	castlingRightsMask[rfrom] |= cr;
	castlingRookSquare[cr] = rfrom;
	castlingPath[cr] = path_bb(kfrom, rfrom);
}

void Position::set(std::string fenStr, StateInfo& st, Thread& th) {
	assert(NO_PIECE == 0);
	memset(board, NO_PIECE, sizeof(board));
	memset(pieceCount, 0, sizeof(pieceCount));
	memset(byColorBB, 0, sizeof(byColorBB));
	memset(byTypeBB, 0, sizeof(byTypeBB));
	st.castlingRights = 0;
	memset(castlingRightsMask, 0, sizeof(castlingRightsMask));
	memset(castlingPath, 0, sizeof(castlingPath));

	std::istringstream is(fenStr);
	std::string pcPlacement, actColor, castlingRightsStr, epStr;
	int halfmove, fullmove;
	is >> pcPlacement >> actColor >> castlingRightsStr >> epStr >> halfmove >> fullmove;
	// piece placement
	File f = FILE_A;
	Rank r = RANK_8;
	for (char c : pcPlacement) {
		if (c == '/') {
			r = Rank(r - 1);
			f = FILE_A;
		} else if (isdigit(c)) {
			f = File(f + (c - '0'));
		} else {
			Piece pc = get_piece(c);
			Square sq = make_square(f, r);
			put_piece(pc, sq);
		}
	}
	// side to move
	if (actColor == "w") sideToMove = WHITE;
	else {
		sideToMove = BLACK;
	}
	// castling rights
	for (char ch : castlingRightsStr) {
		Color c = islower(ch) ? BLACK : WHITE;
		CastlingRights side = (tolower(ch) == 'q') ? QUEEN_SIDE : KING_SIDE;
		st.castlingRights |= get_side(c) & side;
		set_castling_right(c, side);
	}
	// enpassant
	if (epStr == "-") st.epSquare = SQ_NONE;
	else  {
		st.epSquare = get_square(epStr);
	}
	st.rule50 = halfmove;
	ply = std::max(2 * (fullmove - 1), 0) + (sideToMove == BLACK);
	this->set_state(st);
	this->st = &st;
	this->th = &th;
}

void Position::put_piece(Piece pc, Square sq) {
	board[sq] = pc;
	index[sq] = pieceCount[pc];
	pieceList[pc][pieceCount[pc]++] = sq;
	byColorBB[get_color(pc)] |= 1ULL << sq;
	byTypeBB[ALL_PIECE] |= 1ULL << sq;
	byTypeBB[get_piece_type(pc)] |= 1ULL << sq;
}

void Position::remove_piece(Square sq) {
	Piece pc = board[sq];
	if (pc == NO_PIECE) return;
	board[sq] = NO_PIECE;
	int i = index[sq], last = --pieceCount[pc];
	if (i != last) {
		pieceList[pc][i] = pieceList[pc][last];
		index[pieceList[pc][last]] = i;
	}
	byColorBB[get_color(pc)] &= ~(1ULL << sq);
	byTypeBB[ALL_PIECE] &= ~(1ULL << sq);
	byTypeBB[get_piece_type(pc)] &= ~(1ULL << sq);
}

/**
 * @brief Trust the caller: fr != to
 *
 * @param fr From square
 * @param to To square
 */
void Position::move_piece(Square fr, Square to) {
	if (board[to] != NO_PIECE) remove_piece(to);
	put_piece(board[fr], to);
	remove_piece(fr);
}

Bitboard Position::pieces() const {
	return byTypeBB[ALL_PIECE];
}
Bitboard Position::pieces(PieceType pt) const {
	return byTypeBB[pt];
}
Bitboard Position::pieces(PieceType pt1, PieceType pt2) const {
	return byTypeBB[pt1] | byTypeBB[pt2];
}
Bitboard Position::pieces(Color c) const {
	return byColorBB[c];
}
Bitboard Position::pieces(Color c, PieceType pt) const {
	return byColorBB[c] & byTypeBB[pt];
}
Bitboard Position::pieces(Color c, PieceType pt1, PieceType pt2) const {
	return byColorBB[c] & (byTypeBB[pt1] | byTypeBB[pt2]);
}
Piece Position::piece_on(Square s) const {
	return board[s];
}
Square Position::ep_square() const {
	return st->epSquare;
}
bool Position::empty(Square s) const {
	return board[s] == NO_PIECE;
}
int Position::count(PieceType pt, Color c) const {
	return pieceCount[make_piece(c, pt)];
}
int Position::count(PieceType pt) const {
	return count(pt, WHITE) + count(pt, BLACK);
}
const Square* Position::squares(PieceType pt, Color c) const {
	return pieceList[make_piece(c, pt)];
}
Square Position::square(PieceType pt, Color c) const {
	assert(pieceCount[make_piece(c, pt)] == 1);
	return pieceList[make_piece(c, pt)][0];
}

int Position::castling_rights(Color c) const {
	return get_side(c) & st->castlingRights;
}
bool Position::can_castle(CastlingRights cr) const {
	return cr & st->castlingRights;
}
bool Position::castling_impeded(CastlingRights cr) const {
	assert(cr == WHITE_OO || cr == WHITE_OOO || cr == BLACK_OO || cr == BLACK_OOO);
	return byTypeBB[ALL_PIECE] & castlingPath[cr];
}
Square Position::castling_rook_square(CastlingRights cr) const {
	assert(cr == WHITE_OO || cr == WHITE_OOO || cr == BLACK_OO || cr == BLACK_OOO);
	return castlingRookSquare[cr];
}

