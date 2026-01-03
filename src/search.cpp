#include "uci.h"
#include "evaluation.h"
#include "position.h"
#include "types.h"
#include "transposition.h"
#include <algorithm>
#include <vector>

struct ScoredMove {
	Move move;
	int score;
};

// Hàm chấm điểm sơ bộ để sắp xếp nước đi (MVV-LVA)
int score_move(const Position& pos, Move m) {
	if (type_of(m) == PROMOTION) return 20000;
	if (pos.piece_on(to_sq(m)) != NO_PIECE) {
		Piece attacker = pos.piece_on(from_sq(m));
		Piece victim = pos.piece_on(to_sq(m));
		return 10000 + 10 * get_piece_type(victim) - get_piece_type(attacker);
	}
	return 0;
}

// Quiescence Search: Tìm kiếm yên tĩnh để tránh Horizon Effect
Value qsearch(Position& pos, Value alpha, Value beta, int &nodes) {
	++nodes;

	Value stand_pat = eval(pos);
	if (stand_pat >= beta) return beta;
	if (alpha < stand_pat) alpha = stand_pat;

	std::vector<Move> moves;
	pos.generate_moves(moves);
	
	std::vector<ScoredMove> scored_moves;
	for (Move m : moves) {
		if (pos.piece_on(to_sq(m)) == NO_PIECE && type_of(m) != PROMOTION && type_of(m) != ENPASSANT) continue;
		int s = score_move(pos, m);
		scored_moves.push_back({m, s});
	}

	std::sort(scored_moves.begin(), scored_moves.end(), [](const ScoredMove& a, const ScoredMove& b) {
		return a.score > b.score;
	});

	StateInfo st;
	for (const auto& sm : scored_moves) {
		Move m = sm.move;
		pos.do_move(m, st);
		Value val = -qsearch(pos, -beta, -alpha, nodes);
		pos.undo_move();

		if (val >= beta) return beta;
		if (val > alpha) alpha = val;
	}
	return alpha;
}

// Hàm tìm kiếm chính (Alpha-Beta Pruning)
Value search(Position& pos, StateListPtr& dq, int depth, int ply, Value alpha, Value beta, int& nodes) {
	if (pos.is_draw()) return VALUE_DRAW;
	alpha = std::max(alpha, Value(-VALUE_MATE + ply));
	beta = std::min(beta, Value(VALUE_MATE - ply + 1));
	if (alpha >= beta) return alpha;

	Key key = pos.key();
	TTEntry tte = ttable.get(key);
	Move tt_move = MOVE_NONE;

	if (tte.key == uint64_t(key)) {
		tt_move = Move(tte.move);
		
		if (tte.depth >= depth) {
			Value ttValue = TranspositionTable::value_from_tt(Value(tte.value), ply);
			
			Bound b = get_bound_type(tte.genbound);
			if (b == BOUND_EXACT) return ttValue;
			if (b == BOUND_LOWER && ttValue >= beta) return ttValue;
			if (b == BOUND_UPPER && ttValue <= alpha) return ttValue;
		}
	}

	bool in_check = pos.is_in_check();
	if (in_check) ++depth;
	if (depth <= 0 && !in_check) return qsearch(pos, alpha, beta, nodes);

	// 2. Sinh nước đi
	std::vector<Move> moves;
	pos.generate_moves(moves);
	
	// Kiểm tra chiếu hết hoặc hòa cờ
	if (moves.empty()) {
		if (in_check) return Value(-VALUE_MATE + ply);
		return VALUE_DRAW;
	}

	// 3. Sắp xếp nước đi (Move Ordering)
	std::vector<ScoredMove> scored_moves;
	scored_moves.reserve(moves.size());
	for (Move m : moves) {
		int s = score_move(pos, m);
		if (m == tt_move) s += 30000; // Ưu tiên số 1: Nước đi từ Hash Table
		scored_moves.push_back({m, s});
	}

	// Sắp xếp giảm dần theo điểm
	std::sort(scored_moves.begin(), scored_moves.end(), [](const ScoredMove& a, const ScoredMove& b) {
		return a.score > b.score;
	});

	// 4. Duyệt Alpha-Beta
	Value best_val = -VALUE_INFINITE;
	Move best_move = MOVE_NONE;
	Bound bound = BOUND_UPPER; // Mặc định là Upper Bound (chưa tìm thấy nước nào tốt hơn Alpha)

	for (const auto& sm : scored_moves) {
		Move m = sm.move;
		dq->emplace_back();
		pos.do_move(m, dq->back());

		Value val = -search(pos, dq, depth - 1, ply + 1, -beta, -alpha, nodes);

		pos.undo_move();
		dq->pop_back();

		if (val > best_val) {
			best_val = val;
			best_move = m;
			if (val > alpha) {
				alpha = val;
				bound = BOUND_EXACT; // Tìm thấy nước tốt, cập nhật thành Exact Bound
			}
		}
		if (alpha >= beta) {
			bound = BOUND_LOWER; // Cắt tỉa Beta -> Lower Bound
			break; 
		}
	}

	Value tt_val_to_store = TranspositionTable::value_to_tt(best_val, ply);
	ttable.set(key, TTEntry(key, best_move, SCORE_ZERO, Score(tt_val_to_store), 0, false, bound, depth));

	return best_val;
}

// Hàm gốc gọi từ UCI: Thực hiện Iterative Deepening Search (IDS)
Move find_best_move(Position& pos, StateListPtr& dq, int max_depth) {
	Move best_root_move = MOVE_NONE;

	int nodes = 0;
	
	for (int depth = 1; depth <= max_depth; ++depth) {
		Value best_val = -VALUE_INFINITE;
		Value alpha = -VALUE_INFINITE;
		Value beta = VALUE_INFINITE;

		std::vector<Move> moves;
		pos.generate_moves(moves);
		
		std::vector<ScoredMove> scored_moves;
		for (Move m : moves) {
			int s = score_move(pos, m);
			if (m == best_root_move) s += 30000;
			scored_moves.push_back({m, s});
		}
		std::sort(scored_moves.begin(), scored_moves.end(), [](const ScoredMove& a, const ScoredMove& b) {
			return a.score > b.score;
		});

		Move current_best_move = MOVE_NONE;
		for (const auto& sm : scored_moves) {
			Move m = sm.move;
			dq->emplace_back();
			pos.do_move(m, dq->back());

			Value val = -search(pos, dq, depth - 1, 1, -beta, -alpha, nodes);

			pos.undo_move();
			dq->pop_back();

			if (val > best_val) {
				best_val = val;
				current_best_move = m;
				alpha = val;
			}
		}

		best_root_move = current_best_move;
	}

	return best_root_move;
}
