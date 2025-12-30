#include "position.h"
#include "bitboard.h"
#include "random.h"
#include "types.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>

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
			if (get_file(to_square) == FILE_B) { // queen side
				rook_from = make_square(FILE_A, rook_rank);
				rook_to = make_square(FILE_C, rook_rank);
			} else {
				rook_from = make_square(FILE_H, rook_rank);
				rook_to = make_square(FILE_F, rook_rank);
			}
			Piece rook_pc = make_piece(this->sideToMove, ROOK);
			newSt.key ^= Zobrist::psq[rook_pc][rook_from];
			this->move_piece(rook_from, rook_to);
			newSt.key ^= Zobrist::psq[rook_pc][rook_to];

			// lost all castling rights
			dec_bit(newSt.castlingRights, get_side(this->sideToMove));
		}	break;
		case NORMAL:
			this->move_piece(fr_square, to_square);
			// check if this move is the pawn move up two step and set newSt.epSquare
			if (fr_piece_type == PAWN
			&& abs(get_rank(fr_square) - get_rank(to_square)) == 2) {
				newSt.epSquare = to_square - push_pawn(this->sideToMove);
			}
			// check if king or rook move
			if (fr_piece_type == KING) {
				dec_bit(newSt.castlingRights, get_side(this->sideToMove));
			} else if (fr_piece_type == ROOK) {
				dec_bit(newSt.castlingRights, get_side(this->sideToMove) & get_rook_side(fr_square));
			}
			break;
	}

	if (fr_piece_type == PAWN || newSt.capturedPiece != NO_PIECE) {
		newSt.rule50 = 0;
	} else {
		++newSt.rule50;
	}

	// update new key
	if (to_piece != NO_PIECE) {
		newSt.key ^= Zobrist::psq[to_piece][to_square];
	}
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
		if (get_file(to_square) == FILE_B) { // queen side
			rook_from = make_square(FILE_A, rook_rank);
			rook_to = make_square(FILE_C, rook_rank);
		} else {
			rook_from = make_square(FILE_H, rook_rank);
			rook_to = make_square(FILE_F, rook_rank);
		}
		this->move_piece(rook_to, rook_from);
	} else if (movetype == ENPASSANT) {
		Square eaten_pawn_sq = fr_square + Direction(get_file(to_square) - get_file(fr_square));
		Piece eaten_pawn_pc = make_piece(flip_color(movingSide), PAWN);
		this->put_piece(eaten_pawn_pc, eaten_pawn_sq);
	}

	this->sideToMove = movingSide;
	this->st = this->st->prev;
	--this->ply;
}

void Position::generate_moves(std::vector<Move>& moves) const {
	for (Square s = SQ_A1; s < SQ_NB; ++s) {
		Piece pc = this->board[s];
		if (pc == NO_PIECE || this->sideToMove != get_color(pc)) continue;

		PieceType pt = get_piece_type(pc);
		switch (pt) {
			case PAWN: {
				Square one_forward = s + push_pawn(this->sideToMove);
				Square two_forward = s + 2 * push_pawn(this->sideToMove);

				// push one
				if (valid_square(one_forward) && this->board[one_forward] == NO_PIECE) {
					if (get_rank(s) == get_initial_pawn_rank(flip_color(this->sideToMove))) {
						Move pro_m= make_move(s, s + push_pawn(this->sideToMove));
						pro_m |= PROMOTION;
						for (PieceType pro_pt = KNIGHT; pro_pt <= QUEEN; ++pro_pt) {
							moves.push_back(act_promotion_type(pro_m, pro_pt));
						}
					} else {
						moves.push_back(make_move(s, s + push_pawn(this->sideToMove)));
					}
					if (valid_square(two_forward) && this->board[two_forward] == NO_PIECE
					&& get_rank(s) == get_initial_pawn_rank(this->sideToMove)) {
						moves.push_back(make_move(s, s + 2 * push_pawn(this->sideToMove)));
					}
				}
				// enpassant
				if (this->st->epSquare != SQ_NONE) {
					Square epSquare = this->st->epSquare;
					if (get_rank(s) + push_pawn(this->sideToMove) == get_rank(epSquare)
					&& abs(get_file(s) - get_file(epSquare)) == 1) {
						Move m = make_move(s, epSquare);
						m |= ENPASSANT;
						moves.push_back(m);
					}
				}
			}	break;

			// simple move type
			case KING:
			case KNIGHT: {
				int num_dir;
				const Direction *dirs;
				if (pt == KNIGHT) {
					static const Direction knight_dirs[] = {
						EAST * 1 + NORTH * 2,
						EAST * 2 + NORTH * 1,
						EAST * 2 + NORTH * -1,
						EAST * 1 + NORTH * -2,
						EAST * -1 + NORTH * -2,
						EAST * -2 + NORTH * -1,
						EAST * -2 + NORTH * 2,
						EAST * -1 + NORTH * 1,
					};
					num_dir = 8;
					dirs = knight_dirs;
				} else if (pt == KING) {
					static const Direction king_dirs[] = { NORTH, SOUTH, WEST, EAST, NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST };
					num_dir = 8;
					dirs = king_dirs;
				}

				for (int i = 0; i < num_dir; ++i) {
					Square to_square = s + dirs[i];
					if (valid_square(to_square) && this->board[to_square] == NO_PIECE) {
						moves.push_back(make_move(s, to_square));
					}
				}
			}	break;

			// ray type
			case BISHOP:
			case ROOK:
			case QUEEN: {
				int num_rays;
				const Direction *rays;

				if (pt == BISHOP) {
					static const Direction bs_rays[] = { NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST };
					num_rays = 4;
					rays = bs_rays;
				} else if (pt == ROOK) {
					static const Direction rk_rays[] = { NORTH, SOUTH, WEST, EAST };
					num_rays = 4;
					rays = rk_rays;
				} else if (pt == QUEEN) {
					static const Direction qu_rays[] = { NORTH, SOUTH, WEST, EAST, NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST };
					num_rays = 8;
					rays = qu_rays;
				}

				for (int i = 0; i < num_rays; ++i) {
					const Direction ray = rays[i];
					Square to_square = s + ray;
					while (true) {
						if (!valid_square(to_square)) break;
						Piece pc = this->board[to_square];
						if (pc == NO_PIECE) {
							moves.push_back(make_move(s, to_square));
						} else {
							Color c = get_color(pc);
							if (this->sideToMove != c) {
								moves.push_back(make_move(s, to_square));
							}
							break;
						}
						to_square += ray;
					}
				}

			}	break;

			default: assert(0);
		}
	}
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

