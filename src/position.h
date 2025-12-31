#ifndef POSITION_H_INCLUDED
#define POSITION_H_INCLUDED

#include "bitboard.h"
#include "types.h"
#include <deque>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

struct StateInfo {
	Key key;
	int castlingRights;
	int rule50;
	Piece capturedPiece;
	Move lastmove;
	Square epSquare;
	StateInfo *prev;
};

typedef std::unique_ptr<std::deque<StateInfo>> StateListPtr;

class Position {
public:
	static void init();

	void print_board() const;

	void set(std::string fenStr, StateInfo& st);
	const std::string fen() const;

	Bitboard pieces() const;
	Bitboard pieces(PieceType pt) const;
	Bitboard pieces(Color c) const;
	Bitboard pieces(Color c, PieceType pt) const;
	Piece piece_on(Square s) const;
	Square ep_square() const;
	Color side_to_move() const;
	int rule50() const;
	Key key() const { return st->key; } 

	int castling_rights() const;
	int castling_rights(Color c) const;
	bool can_castle(CastlingRights cr) const;
	Square castling_king_square(CastlingRights cr) const;
	Square castling_rook_square(CastlingRights cr) const;
	Square castling_rook_to_square(CastlingRights cr) const;

	void do_move(Move m, StateInfo& newSt);
	void undo_move();
	void generate_moves(std::vector<Move>& moves);
	bool checkmate(bool checkOpponent=false);

	Move string_to_move(std::string str);

	bool square_is_attacked(Color c, Square sq) const;
	
private:
	void put_piece(Piece pc, Square sq);
	void remove_piece(Square sq);
	void move_piece(Square fr, Square to);

	bool can_move_to(Square from, Square to);
	bool can_capture_piece(Piece pc);

	bool square_empty(Square sq) const;

	Bitboard generate_attack_bitboard(Color c) const;
	bool squareIsAttacked(Color c, Square to) const;
	bool pieceIsAttacked(Color c, PieceType pt) const;
	bool kingIsAttacked(Color c);

	bool checkStrictlyLegalMove(Move m);

	Piece board[SQ_NB];
	Bitboard byColorBB[COLOR_NB];
	Bitboard byTypeBB[PIECE_TYPE_NB];
	Color sideToMove;
	int ply;
	StateInfo *st;
};

inline Piece Position::piece_on(Square s) const {
	return this->board[s];
}
inline Square Position::ep_square() const {
	return this->st->epSquare;
}
inline Color Position::side_to_move() const {
	return this->sideToMove;
}

inline int Position::castling_rights() const {
	return this->st->castlingRights;
}
inline int Position::castling_rights(Color c) const {
	return this->castling_rights() & get_side(c);
}

inline bool Position::can_castle(CastlingRights cr) const {
	if (!(this->castling_rights() & cr)) return false;

	Color us = this->sideToMove;

	Square king_sq = make_square(FILE_E, get_initial_king_rank(us));
	Square rook_sq = this->castling_rook_square(cr);

	if (this->piece_on(rook_sq) != make_piece(us, ROOK))
		return false;

	Square king_to = this->castling_king_square(cr);
	Square rook_to = this->castling_rook_to_square(cr);

	Bitboard king_path = path_bb(king_sq, king_to) & ~square_bb(king_sq);
	if (king_path & this->pieces())
		return false;

	Bitboard attacks = this->generate_attack_bitboard(flip_color(us));
	if (path_bb(king_sq, king_to) & attacks)
		return false;

	if (this->piece_on(rook_to) != NO_PIECE)
		return false;

	return true;
}

inline Square Position::castling_king_square(CastlingRights cr) const {
	Color c = (cr & WHITE_SIDE) ? WHITE : BLACK;
	Rank r = get_initial_king_rank(c);
	File f = (cr & QUEEN_SIDE) ? FILE_C : FILE_G;
	return make_square(f, r);
}

inline Square Position::castling_rook_square(CastlingRights cr) const {
	Color c = (cr & WHITE_SIDE) ? WHITE : BLACK;
	Rank r = get_initial_king_rank(c);
	File f = (cr & QUEEN_SIDE) ? FILE_A : FILE_H;
	return make_square(f, r);
}
inline Square Position::castling_rook_to_square(CastlingRights cr) const {
	Color c = (cr & WHITE_SIDE) ? WHITE : BLACK;
	Rank r = get_initial_king_rank(c);
	File f = (cr & QUEEN_SIDE) ? FILE_D : FILE_F;
	return make_square(f, r);
}

inline int Position::rule50() const {
	return this->st->rule50;
}

inline Bitboard Position::pieces() const {
	return this->pieces(WHITE) | this->pieces(BLACK);
}

inline Bitboard Position::pieces(Color c) const {
	return this->byColorBB[c];
}

inline Bitboard Position::pieces(PieceType pt) const {
	return this->byTypeBB[pt];
}

inline Bitboard Position::pieces(Color c, PieceType pt) const {
	return pieces(c) & pieces(pt);
}

inline bool Position::square_empty(Square sq) const {
	return this->board[sq] == NO_PIECE;
}

inline bool Position::square_is_attacked(Color c, Square sq) const {
	return this->generate_attack_bitboard(c) & square_bb(sq);
}

#endif