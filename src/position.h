#ifndef POSITION_H_INCLUDED
#define POSITION_H_INCLUDED

#include "types.h"
#include <deque>
#include <memory>
#include <string>

struct StateInfo {
	Key pawnKey;
	// Key materialKey;
	Key key;
	// Value nonPawnMaterial[COLOR_NB];
	int castlingRights;
	int rule50;
	// int repetition;
	// int pliesFromNull;
	Piece capturedPiece;
	Move lastmove;
	Square epSquare;
	Bitboard checkersBB;
	Bitboard blockersForKing[COLOR_NB];
	Bitboard pinners[COLOR_NB];
	Bitboard checkSquares[PIECE_TYPE_NB];
	StateInfo *prev;
};

/// A list to keep track of the position states along the setup moves (from the
/// start position to the position just before the search starts). Needed by
/// 'draw by repetition' detection. Use a std::deque because pointers to
/// elements are not invalidated upon list resizing.
typedef std::unique_ptr<std::deque<StateInfo>> StateListPtr;

class Thread;

class Position {
public:
	static void init();

	void set(std::string fenStr, StateInfo& st, Thread& th);
	const std::string fen() const;

	Bitboard pieces() const;
	Bitboard pieces(PieceType pt) const;
	Bitboard pieces(PieceType pt1, PieceType pt2) const;
	Bitboard pieces(Color c) const;
	Bitboard pieces(Color c, PieceType pt) const;
	Bitboard pieces(Color c, PieceType pt1, PieceType pt2) const;
	Piece piece_on(Square s) const;
	Square ep_square() const;
	bool empty(Square s) const;
	int count(PieceType pt, Color c) const;
	int count(PieceType pt) const;
	const Square* squares(PieceType pt, Color c) const;
	Square square(PieceType pt, Color c) const;

	int castling_rights(Color c) const;
	bool can_castle(CastlingRights cr) const;
	bool castling_impeded(CastlingRights cr) const;
	Square castling_rook_square(CastlingRights cr) const;

	void do_move(); // TODO: need this for search algorithm
	void undo_move(); // TODO: need this for search algorithm

private:
	void put_piece(Piece pc, Square sq);
	void remove_piece(Square sq);
	void move_piece(Square fr, Square to);

	void set_state(StateInfo& st);
	void set_check_info(StateInfo& st);
	void set_castling_right(Color c, CastlingRights side);

	Piece board[SQ_NB];
	int pieceCount[PIECE_NB];
	int index[SQ_NB];
	Square pieceList[PIECE_NB][16];
	Bitboard byColorBB[COLOR_NB];
	Bitboard byTypeBB[PIECE_TYPE_NB];
	int castlingRightsMask[SQ_NB];
	Square castlingRookSquare[CASTLING_RIGHT_NB];
	Bitboard castlingPath[CASTLING_RIGHT_NB];
	Color sideToMove;
	int ply;
	StateInfo *st;
	Thread *th;
};

#endif
