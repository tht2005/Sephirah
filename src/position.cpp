#include "position.h"
#include "bitboard.h"
#include "random.h"
#include "types.h"
// Đã xóa bits/floatn-common.h
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
// Đã xóa unistd.h

constexpr Piece Pieces[] = {	W_PAWN, W_ROOK, W_KNIGHT, W_BISHOP, W_QUEEN, W_KING,
				B_PAWN, B_ROOK, B_KNIGHT, B_BISHOP, B_QUEEN, B_KING};

namespace Zobrist {
	Key psq[PIECE_NB][SQ_NB];
	Key enpassant[FILE_NB];
	Key castlingRights[CASTLING_RIGHT_NB];
	Key side;
	// Key noPawn;
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
	// Zobrist::noPawn = random_u64();
}

void Position::set(std::string fenStr, StateInfo& st) {
	std::istringstream ss(fenStr);
	std::string piece_placement_str, castling_right_str, ep_str;
	char active_color;
	int halfmove_clock, full_move_number;
	ss >> piece_placement_str >> active_color >> castling_right_str >> ep_str
		>> halfmove_clock >> full_move_number;

	// Piece board[SQ_NB];
	// Bitboard byColorBB[COLOR_NB];
	// Bitboard byTypeBB[PIECE_TYPE_NB];
	// Color sideToMove;
	// int ply;
	// StateInfo *st;

	// struct StateInfo {
	// 	Key key;
	// 	int castlingRights;
	// 	int rule50;
	// 	Piece capturedPiece;
	// 	Move lastmove;
	// 	Square epSquare;
	// 	StateInfo *prev;
	// };

	// init position
	assert(NO_PIECE == 0);
	memset(this->board, NO_PIECE, sizeof(this->board));
	memset(this->byColorBB, 0, sizeof(this->byColorBB));
	memset(this->byTypeBB, 0, sizeof(this->byTypeBB));
	this->st = &st;
	this->ply = 0;

	// init state info
	st.key = 0;
	st.castlingRights = 0;
	st.rule50 = halfmove_clock;
	st.capturedPiece = NO_PIECE;
	st.lastmove = MOVE_NONE;
	// st.epSquare = (ep_str == "-") ? SQ_NONE : str_to_square(ep_str);
	st.prev = nullptr;

	Square current_sq = SQ_A8;
	for (char c : piece_placement_str) {
		if ('0' <= c && c <= '9') {
			current_sq += int(c - '0');
		} else if (c == '/') {
			current_sq -= 16;
		} else {
			Piece p = char_to_piece(c);
			this->board[current_sq] = p;
			act_bit(this->byColorBB[get_color(p)], current_sq);
			act_bit(this->byTypeBB[get_piece_type(p)], current_sq);
			st.key ^= Zobrist::psq[p][current_sq];
			++current_sq;
		}
	}
	assert(current_sq == SQ_A1 + 8);

	this->sideToMove = (active_color == 'b') ? BLACK : WHITE;
	if (this->sideToMove == BLACK) {
		st.key ^= Zobrist::side;
	}

	for (char c : castling_right_str) {
		st.castlingRights |= char_to_castling_rights(c);
	}
	st.key ^= Zobrist::castlingRights[st.castlingRights];

	if (ep_str == "-") {
		st.epSquare = SQ_NONE;
	} else {
		st.epSquare = str_to_square(ep_str);
		st.key ^= Zobrist::enpassant[get_file(st.epSquare)];
	}
}

void Position::put_piece(Piece pc, Square sq) {
	if (pc == NO_PIECE) return;
	this->board[sq] = pc;
	act_bit(this->byColorBB[get_color(pc)], sq);
	act_bit(this->byTypeBB[get_piece_type(pc)], sq);
}

void Position::remove_piece(Square sq) {
	Piece pc = this->board[sq];
	if (pc == NO_PIECE) return;
	this->board[sq] = NO_PIECE;
	dec_bit(this->byColorBB[get_color(pc)], sq);
	dec_bit(this->byTypeBB[get_piece_type(pc)], sq);
}

void Position::move_piece(Square fr, Square to) {
	assert (fr != to);
	Piece pc = this->board[fr];
	this->remove_piece(fr);
	this->remove_piece(to);
	this->put_piece(pc, to);
}

void Position::do_move(Move m, StateInfo& newSt) {
	// Piece board[SQ_NB];
	// Bitboard byColorBB[COLOR_NB];
	// Bitboard byTypeBB[PIECE_TYPE_NB];
	// Color sideToMove;
	// int ply;
	// StateInfo *st;

	// 	Key key;
	// 	int castlingRights;
	// 	int rule50;
	// 	Piece capturedPiece;
	// 	Move lastmove;
	// 	Square epSquare;
	// 	StateInfo *prev;

	newSt.key = this->st->key;
	newSt.castlingRights = this->st->castlingRights;
	newSt.rule50 = this->st->rule50;
	newSt.capturedPiece = NO_PIECE;
	newSt.lastmove = m;
	newSt.epSquare = SQ_NONE;
	newSt.prev = this->st;

	Square fr_square = from_sq(m);
	Square to_square = to_sq(m);
	Piece fr_piece = this->board[fr_square];
	Piece to_piece = this->board[to_square];
	PieceType fr_piece_type = get_piece_type(fr_piece);

	// undo key
	if (fr_piece != NO_PIECE) {
		newSt.key ^= Zobrist::psq[fr_piece][fr_square];
	}
	if (to_piece != NO_PIECE) {
		newSt.key ^= Zobrist::psq[to_piece][to_square];
		newSt.capturedPiece = to_piece;
	}
	if (this->st->epSquare != SQ_NONE) {
		newSt.key ^= Zobrist::enpassant[get_file(this->st->epSquare)];
	}
	newSt.key ^= Zobrist::castlingRights[this->st->castlingRights];

	// the final key update only handle from and to square hash, additional
	// changes handled for each type
	MoveType moveType = type_of(m);

	switch (moveType) {
		case PROMOTION: {
			Piece pro_piece = make_piece(this->sideToMove, promotion_type(m));
			this->remove_piece(fr_square);
			this->remove_piece(to_square);
			this->put_piece(pro_piece, to_square);
		}	break;
		case ENPASSANT: {
			this->move_piece(fr_square, to_square);
			Square eaten_pawn_sq = fr_square + Direction(get_file(to_square) - get_file(fr_square));
			Piece eaten_pawn_pc = this->board[eaten_pawn_sq];
			newSt.key ^= Zobrist::psq[eaten_pawn_pc][eaten_pawn_sq];
			this->remove_piece(eaten_pawn_sq);

			newSt.capturedPiece = eaten_pawn_pc;
			newSt.key ^= Zobrist::enpassant[get_file(eaten_pawn_sq)];
		}	break;
		case CASTLING: {
			Square rook_from, rook_to;
			Rank rook_rank = get_initial_king_rank(this->sideToMove);
			if (get_file(to_square) == FILE_C) { // queen side
				rook_from = make_square(FILE_A, rook_rank);
				rook_to = make_square(FILE_D, rook_rank);
			} else {
				rook_from = make_square(FILE_H, rook_rank);
				rook_to = make_square(FILE_F, rook_rank);
			}
			Piece rook_pc = make_piece(this->sideToMove, ROOK);
			newSt.key ^= Zobrist::psq[rook_pc][rook_from];
			this->move_piece(rook_from, rook_to);
			newSt.key ^= Zobrist::psq[rook_pc][rook_to];

			this->move_piece(fr_square, to_square);

			// lost all castling rights
			newSt.castlingRights &= ~get_side(this->sideToMove);
		}	break;
		default:
			this->move_piece(fr_square, to_square);
			// check if this move is the pawn move up two step and set newSt.epSquare
			if (fr_piece_type == PAWN
			&& abs(get_rank(fr_square) - get_rank(to_square)) == 2) {
				newSt.epSquare = to_square - push_pawn(this->sideToMove);
			}
			if (fr_piece_type == KING) {
				newSt.castlingRights &= ~get_side(this->sideToMove);
			} else if (fr_piece_type == ROOK) {
				newSt.castlingRights &= ~(get_side(this->sideToMove) & get_rook_side(fr_square));
			}
			break;
	}

	if (fr_piece_type == PAWN || newSt.capturedPiece != NO_PIECE) {
		newSt.rule50 = 0;
	} else {
		++newSt.rule50;
	}

	// update new key
	Piece piece_to_add = fr_piece;
	if (moveType == PROMOTION) {
		piece_to_add = make_piece(this->sideToMove, promotion_type(m));
	}
	newSt.key ^= Zobrist::psq[piece_to_add][to_square];

	if (newSt.epSquare != SQ_NONE) {
		newSt.key ^= Zobrist::enpassant[get_file(newSt.epSquare)];
	}
	newSt.key ^= Zobrist::castlingRights[newSt.castlingRights];

	// flip side
	this->sideToMove = flip_color(this->sideToMove);
	newSt.key ^= Zobrist::side;

	this->st = &newSt;
	++this->ply;
}

void Position::undo_move() {
	Piece capturedPiece = this->st->capturedPiece;
	int cr = this->st->castlingRights;
	Square epSquare = this->st->epSquare;
	Move lastmove = this->st->lastmove;
	MoveType movetype = type_of(lastmove);

	Color movingSide = flip_color(this->sideToMove);

	Square fr_square = from_sq(lastmove);
	Square to_square = to_sq(lastmove);
	Piece fr_piece = movetype == PROMOTION ? make_piece(movingSide, PAWN) : this->board[to_square];
	Piece to_piece = movetype == ENPASSANT ? NO_PIECE : capturedPiece;
	// PieceType fr_piece_type = get_piece_type(fr_piece);

	this->remove_piece(fr_square);
	this->remove_piece(to_square);
	this->put_piece(fr_piece, fr_square);
	this->put_piece(to_piece, to_square);

	if (movetype == CASTLING) {
		Square rook_from, rook_to;
		Rank rook_rank = get_initial_king_rank(movingSide);
		if (get_file(to_square) == FILE_C) { // queen side
			rook_from = make_square(FILE_A, rook_rank);
			rook_to = make_square(FILE_D, rook_rank);
		} else {
			rook_from = make_square(FILE_H, rook_rank);
			rook_to = make_square(FILE_F, rook_rank);
		}
		this->remove_piece(rook_to);
		this->put_piece(make_piece(movingSide, ROOK), rook_from);
	} else if (movetype == ENPASSANT) {
		Square eaten_pawn_sq = fr_square + Direction(get_file(to_square) - get_file(fr_square));
		Piece eaten_pawn_pc = make_piece(flip_color(movingSide), PAWN);
		this->put_piece(eaten_pawn_pc, eaten_pawn_sq);
	}

	this->sideToMove = movingSide;
	this->st = this->st->prev;
	--this->ply;
}

void Position::generate_moves(svec<Move>& moves) {
	moves.clear();

	Color us = this->sideToMove;
	Color them = flip_color(us);

	Bitboard our_pieces = this->pieces(us);
	Bitboard their_pieces = this->pieces(them);
	Bitboard empty_squares = ~(our_pieces | their_pieces);

	// --- PAWNS ---
	Bitboard pawns = this->pieces(us, PAWN);
	Direction up = push_pawn(us);
	Rank promo_rank = get_initial_pawn_rank(them);
	Rank start_rank = get_initial_pawn_rank(us);

	// 1. Single Push
	// Shift pawns 'up'. AND with empty squares.
	Bitboard single_push = (us == WHITE ? (pawns << 8) : (pawns >> 8)) & empty_squares;

	Bitboard b = single_push;
	while (b) {
		Square to = pop_lsb(b);
		Square from = to - up;
		
		if (get_rank(from) == promo_rank) {
			Move m = make_move(from, to);
			m |= PROMOTION;
			moves.push_back(act_promotion_type(m, QUEEN));
			moves.push_back(act_promotion_type(m, ROOK));
			moves.push_back(act_promotion_type(m, BISHOP));
			moves.push_back(act_promotion_type(m, KNIGHT));
		} else {
			moves.push_back(make_move(from, to));
		}
	}

	// 2. Double Push
	// Shift single_push 'up' again. AND with empty squares. AND check rank.
	// White: Rank 3 -> 4. Black: Rank 6 -> 5.
	Bitboard double_push = (us == WHITE ? (single_push << 8) : (single_push >> 8)) & empty_squares;
	// Mask specific ranks (Rank 4 for White, Rank 5 for Black)
	double_push &= (us == WHITE ? 0x00000000FF000000ULL : 0x000000FF00000000ULL);

	b = double_push;
	while (b) {
		Square to = pop_lsb(b);
		moves.push_back(make_move(to - up - up, to));
	}

	// 3. Captures
	// Left Capture
	Bitboard captures = 0;
	// Note: Simple shift logic, requires file masking to prevent wrapping A-file to H-file
	if (us == WHITE) captures = ((pawns & ~FileMask[FILE_A]) << 7) & their_pieces;
	else             captures = ((pawns & ~FileMask[FILE_A]) >> 9) & their_pieces;

	b = captures;
	while (b) {
		Square to = pop_lsb(b);
		// From square calculation depends on direction
		Square from = (us == WHITE) ? (to - 7) : (to + 9);
		
		if (get_rank(from) == promo_rank) {
			Move m = Move(make_move(from, to) | PROMOTION);
			moves.push_back(act_promotion_type(m, QUEEN));
			moves.push_back(act_promotion_type(m, ROOK));
			moves.push_back(act_promotion_type(m, BISHOP));
			moves.push_back(act_promotion_type(m, KNIGHT));
		} else {
			moves.push_back(make_move(from, to));
		}
	}

	// Right Capture
	if (us == WHITE) captures = ((pawns & ~FileMask[FILE_H]) << 9) & their_pieces;
	else             captures = ((pawns & ~FileMask[FILE_H]) >> 7) & their_pieces;

	b = captures;
	while (b) {
		Square to = pop_lsb(b);
		Square from = (us == WHITE) ? (to - 9) : (to + 7);
		
		if (get_rank(from) == promo_rank) {
			Move m = Move(make_move(from, to) | PROMOTION);
			moves.push_back(act_promotion_type(m, QUEEN));
			moves.push_back(act_promotion_type(m, ROOK));
			moves.push_back(act_promotion_type(m, BISHOP));
			moves.push_back(act_promotion_type(m, KNIGHT));
		} else {
			moves.push_back(make_move(from, to));
		}
	}

	// 4. En Passant
	if (this->st->epSquare != SQ_NONE) {
		Square ep = this->st->epSquare;
		// Check pawns that can capture EP (left and right of EP square)
		// This effectively checks "pawns attacking epSquare" logic reversed
		Bitboard attackers = this->pieces(us, PAWN);
		// Check pawn to the "left" (file - 1)
		if (get_file(ep) > FILE_A) {
			Square from = ep - up - 1; // e.g., if ep is d3, from is c3 (white) or c4? 
			// Better: Check if we have a pawn at [ep_file-1, current_rank]
			Square l_pawn = make_square(File(get_file(ep)-1), Rank(get_rank(ep)-push_pawn_rank(us)));
			if (this->piece_on(l_pawn) == make_piece(us, PAWN)) {
				 Move m = make_move(l_pawn, ep);
				 m |= ENPASSANT;
				 moves.push_back(m);
			}
		}
		if (get_file(ep) < FILE_H) {
			Square r_pawn = make_square(File(get_file(ep)+1), Rank(get_rank(ep)-push_pawn_rank(us)));
			if (this->piece_on(r_pawn) == make_piece(us, PAWN)) {
				 Move m = make_move(r_pawn, ep);
				 m |= ENPASSANT;
				 moves.push_back(m);
			}
		}
	}

	// --- KNIGHTS ---
	Bitboard knights = this->pieces(us, KNIGHT);
	while (knights) {
		Square from = pop_lsb(knights);
		// We need a lookup table for attacks. 
		// Since we removed the slow loop, we need a quick way.
		// Assuming you don't have precomputed tables yet, we'll use a small loop 
		// but only for the specific piece, not board scan.
		static const int k_dirs[8][2] = {{1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1},{-2,1},{-1,2}};
		for (auto& d : k_dirs) {
			Square to = advance(from, d[0], d[1]);
			if (is_ok(to) && (this->piece_on(to) == NO_PIECE || get_color(this->piece_on(to)) == them)) {
				moves.push_back(make_move(from, to));
			}
		}
	}

	// --- KINGS ---
	Bitboard king = this->pieces(us, KING); // Should be 1 bit
	if (king) {
		Square from = pop_lsb(king);
		static const int ki_dirs[8][2] = {{0,1},{0,-1},{1,0},{-1,0},{1,1},{1,-1},{-1,1},{-1,-1}};
		for (auto& d : ki_dirs) {
			Square to = advance(from, d[0], d[1]);
			if (is_ok(to) && (this->piece_on(to) == NO_PIECE || get_color(this->piece_on(to)) == them)) {
				moves.push_back(make_move(from, to));
			}
		}
	}

	// --- SLIDING PIECES (Bishops, Rooks, Queens) ---
	// Merging logic for efficiency
	Bitboard sliders = this->pieces(us, BISHOP) | this->pieces(us, ROOK) | this->pieces(us, QUEEN);

	while (sliders) {
		Square from = pop_lsb(sliders);
		PieceType pt = get_piece_type(this->piece_on(from));
		
		// Define directions based on piece type
		bool orth = (pt == ROOK || pt == QUEEN);
		bool diag = (pt == BISHOP || pt == QUEEN);
		
		static const int r_dirs[4][2] = {{0,1},{0,-1},{1,0},{-1,0}};
		static const int b_dirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};

		if (orth) {
			for (auto& d : r_dirs) {
				Square to = from;
				while (true) {
					to = advance(to, d[0], d[1]);
					if (!is_ok(to)) break;
					Piece target = this->piece_on(to);
					if (target == NO_PIECE) {
						moves.push_back(make_move(from, to));
					} else {
						if (get_color(target) == them) moves.push_back(make_move(from, to));
						break; // Hit a piece (own or enemy), stop ray
					}
				}
			}
		}
		if (diag) {
			for (auto& d : b_dirs) {
				Square to = from;
				while (true) {
					to = advance(to, d[0], d[1]);
					if (!is_ok(to)) break;
					Piece target = this->piece_on(to);
					if (target == NO_PIECE) {
						moves.push_back(make_move(from, to));
					} else {
						if (get_color(target) == them) moves.push_back(make_move(from, to));
						break;
					}
				}
			}
		}
	}

	// --- CASTLING ---
	CastlingRights cr_mask = get_side(us);
	if (this->castling_rights() & cr_mask) {
		if (this->can_castle(CastlingRights(cr_mask & KING_SIDE))) {
			Square from = make_square(FILE_E, get_initial_king_rank(us));
			Square to = make_square(FILE_G, get_initial_king_rank(us));
			Move m = make_move(from, to);
			m |= CASTLING;
			moves.push_back(m);
		}
		if (this->can_castle(CastlingRights(cr_mask & QUEEN_SIDE))) {
			Square from = make_square(FILE_E, get_initial_king_rank(us));
			Square to = make_square(FILE_C, get_initial_king_rank(us));
			Move m = make_move(from, to);
			m |= CASTLING;
			moves.push_back(m);
		}
	}

	// --- FILTER LEGAL MOVES ---
	int valid_count = 0;
	for (int i = 0; i < moves.count; ++i) {
		if (this->checkStrictlyLegalMove(moves[i])) {
			moves[valid_count++] = moves[i];
		}
	}
	moves.count = valid_count;
}

const std::string Position::fen() const {
	// std::istringstream ss(fenStr);
	// std::string piece_placement_str, castling_right_str, ep_str;
	// char active_color;
	// int halfmove_clock, full_move_number;
	// ss >> piece_placement_str >> active_color >> castling_right_str >> ep_str
	// 	>> halfmove_clock >> full_move_number;

	std::ostringstream ss;

	for (Rank r = RANK_8; r >= RANK_1; --r) {
		int cons_empty = 0;
		for (File f = FILE_A; f <= FILE_H; ++f) {
			Square sq = make_square(f, r);
			if (this->board[sq] == NO_PIECE) {
				++cons_empty;
			} else {
				if (cons_empty > 0) {
					ss << char(cons_empty + '0');
					cons_empty = 0;
				}
				ss << piece_to_char(this->board[sq]);
			}
		}
		if (cons_empty > 0) {
			ss << char(cons_empty + '0');
			cons_empty = 0;
		}
		if (r != RANK_1) ss << '/';
	}

	ss << ' ' << (this->side_to_move() == BLACK ? 'b' : 'w');

	ss << ' ';
	if (this->castling_rights() == 0) {
		ss << '-';
	} else {
		if (this->castling_rights(WHITE) & KING_SIDE) ss << 'K';
		if (this->castling_rights(WHITE) & QUEEN_SIDE) ss << 'Q';
		if (this->castling_rights(BLACK) & KING_SIDE) ss << 'k';
		if (this->castling_rights(BLACK) & QUEEN_SIDE) ss << 'q';
	}

	ss << ' ';
	if (this->ep_square() == SQ_NONE) {
		ss << '-';
	} else {
		ss << square_to_str(this->ep_square());
	}

	// TODO: half move clock and full move number
	ss << " 0 1";
	
	return ss.str();
}

Bitboard Position::generate_attack_bitboard(Color col) const {
	Bitboard b = 0;
	for (Square s = SQ_A1; s < SQ_NB; ++s) {
		Piece pc = this->board[s];
		if (pc == NO_PIECE || col != get_color(pc)) continue;

		PieceType pt = get_piece_type(pc);
		switch (pt) {
			case PAWN: {
				// diagonal
				for (int dir : { -1, 1 }) {
					Square atk = advance(s, dir, push_pawn_rank(col));
					if (valid_square(atk)) {
						act_bit(b, atk);
					}
				}
				// enpassant
				Square epSquare = this->st->epSquare;
				if (epSquare != SQ_NONE) {
					if (get_rank(s) + push_pawn_rank(col) == get_rank(epSquare)
					&& abs(get_file(s) - get_file(epSquare)) == 1) {
						act_bit(b, epSquare);
					}
				}
			}	break;

			// simple move type
			case KING:
			case KNIGHT: {
				int num_dir;
				const int (*dirs)[2];
				if (pt == KNIGHT) {
					static const int knight_dirs[][2] = {
						{1, 2},
						{2, 1},
						{2, -1},
						{1, -2},
						{-1, -2},
						{-2, -1},
						{-2, 1},
						{-1, 2},
					};
					num_dir = 8;
					dirs = knight_dirs;
				} else if (pt == KING) {
					static const int king_dirs[][2] = {
						{0, 1}, {0, -1}, {-1, 0}, {1, 0}, {1, 1}, {-1, 1}, {1, -1}, {-1, -1},
					};
					num_dir = 8;
					dirs = king_dirs;
				}

				for (int i = 0; i < num_dir; ++i) {
					Square to_square = advance(s, dirs[i][0], dirs[i][1]);
					if (valid_square(to_square)) {
						act_bit(b, to_square);
					}
				}
			}	break;

			// ray type
			case BISHOP:
			case ROOK:
			case QUEEN: {
				int num_rays;
				const int (*rays)[2];

				if (pt == BISHOP) {
					static const int bs_rays[][2] = { {1, 1}, {-1, 1}, {1, -1}, {-1, -1} };
					num_rays = 4;
					rays = bs_rays;
				} else if (pt == ROOK) {
					static const int rk_rays[][2] = { {0, 1}, {0, -1}, {-1, 0}, {1, 0} };
					num_rays = 4;
					rays = rk_rays;
				} else if (pt == QUEEN) {
					static const int qu_rays[][2] = {
						{0, 1}, {0, -1}, {-1, 0}, {1, 0}, {1, 1}, {-1, 1}, {1, -1}, {-1, -1},
					};
					num_rays = 8;
					rays = qu_rays;
				}

				for (int i = 0; i < num_rays; ++i) {
					Square to_square = advance(s, rays[i][0], rays[i][1]);
					while (true) {
						if (!valid_square(to_square)) break;
						Piece pc = this->board[to_square];
						if (pc == NO_PIECE) {
							act_bit(b, to_square);
						} else {
							act_bit(b, to_square);
							break;
						}
						to_square = advance(to_square, rays[i][0], rays[i][1]);
					}
				}

			}	break;

			default: assert(0);
		}
	}
	return b;
}

bool Position::squareIsAttacked(Color c, Square to) const {
	return hav_bit(this->generate_attack_bitboard(c), to);
}

bool Position::pieceIsAttacked(Color c, PieceType pt) const {
	Bitboard b = this->pieces(c, pt);
	assert(b);
	Square sq = lsb(b);
	return this->squareIsAttacked(flip_color(c), sq);
}

bool Position::kingIsAttacked(Color c) const {
	return this->pieceIsAttacked(c, KING);
}

bool Position::checkStrictlyLegalMove(Move m) {
	StateInfo st;
	Color us = this->sideToMove;
	this->do_move(m, st);
	bool legal = !this->kingIsAttacked(us);
	this->undo_move();
	return legal;
}

bool Position::can_move_to(Square from, Square to) {
    Piece pc = this->board[to];
    return pc == NO_PIECE || get_color(pc) != sideToMove;
}

bool Position::can_capture_piece(Piece pc) {
    return pc != NO_PIECE && get_piece_type(pc) != KING;
}

Move Position::string_to_move(std::string str) {
	assert(str.size() >= 4);
	Square fr = str_to_square(str.substr(0, 2));
	Square to = str_to_square(str.substr(2, 2));
	Move m = make_move(fr, to);

	if (str.size() == 5) {
		m |= PROMOTION;
		switch(str.back()) {
			case 'n': m = act_promotion_type(m, KNIGHT); break;
			case 'r': m = act_promotion_type(m, ROOK); break;
			case 'b': m = act_promotion_type(m, BISHOP); break;
			case 'q': m = act_promotion_type(m, QUEEN); break;
			default: assert(0);
		}
	} else if (get_piece_type(this->board[fr]) == PAWN && to == this->ep_square() &&
			abs(get_file(fr) - get_file(to)) == 1) {
		m |= ENPASSANT;
	} else if (get_piece_type(this->board[fr]) == KING && abs(get_file(fr) - get_file(to)) == 2) {
		m |= CASTLING;
	}
	return m;
}

void Position::print_board() const {
	for (Rank r = RANK_8; r >= RANK_1; --r) {
		for (File f = FILE_A; f <= FILE_H; ++f) {
			Square s = make_square(f, r);
			if (s % 8 == 0) {
				std::cout << std::endl;
			}
			if (this->board[s] == NO_PIECE) std::cout << '_';
			else std::cout << piece_to_char(this->board[s]);
		}
	}
	std::cout << std::endl;
}


bool Position::is_checkmate(bool checkOpponent) {
	Color c = this->side_to_move();
	if (checkOpponent) c = flip_color(c);

	if (!this->kingIsAttacked(c)) return false;

	if (c == this->side_to_move()) {
		svec<Move> moves;
		this->generate_moves(moves); 
		return moves.empty(); 
	}

	return false;
}

bool Position::is_draw() const {
	if (this->st->rule50 >= 100) return true;

	int moves_to_check = this->st->rule50;
	StateInfo* curr = this->st->prev;

	for (int i = 0; i < moves_to_check && curr != nullptr; ++i) {
		if (curr->key == this->st->key) {
			return true; // Found repetition
		}
		curr = curr->prev;
	}
	return false;
}

bool Position::is_in_check() const {
	return this->kingIsAttacked(this->sideToMove);
}

