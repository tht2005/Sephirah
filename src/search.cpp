#include "search.h"
#include "thread.h"
#include "evaluation.h"
#include "position.h"
#include "types.h"
#include "transposition.h"
#include <algorithm>
#include <chrono>
#include <cstdint>

struct ScoredMove {
	Move move;
	int score;
};

int score_move(const Position& pos, Move m) {
	if (type_of(m) == PROMOTION) return 20000;
	if (pos.piece_on(to_sq(m)) != NO_PIECE) {
		Piece attacker = pos.piece_on(from_sq(m));
		Piece victim = pos.piece_on(to_sq(m));
		return 10000 + 10 * get_piece_type(victim) - get_piece_type(attacker);
	}
	return 0;
}

void check_time() {
	if (Threads.stop_search) return;

	auto now = std::chrono::steady_clock::now();
	// Use start_time stored in SearchLimits
	auto start = std::chrono::steady_clock::time_point(std::chrono::milliseconds(Threads.limits.start_time));
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();

	if (elapsed > Threads.limits.allocated_time) {
		Threads.stop_search = true;
	}
}

Value qsearch(Position& pos, Value alpha, Value beta, Thread &th) {
	if ((++th.nodes & 2047) == 0) check_time();
	if (Threads.stop_search) return VALUE_ZERO;

	Value stand_pat = eval(pos);
	if (stand_pat >= beta) return beta;
	if (alpha < stand_pat) alpha = stand_pat;

	svec<Move> moves;
	pos.generate_moves(moves);
	
	svec<ScoredMove> scored_moves;
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
		Value val = -qsearch(pos, -beta, -alpha, th);
		pos.undo_move();

		if (val >= beta) return beta;
		if (val > alpha) alpha = val;
	}
	return alpha;
}

// Hàm tìm kiếm chính (Alpha-Beta Pruning)
Value search(Position& pos, StateListPtr& dq, int depth, int ply, Value alpha, Value beta, Thread &th) {
	if ((++th.nodes & 2047) == 0) check_time();
	if (Threads.stop_search) return VALUE_ZERO;

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
	if (depth <= 0 && !in_check) return qsearch(pos, alpha, beta, th);

	svec<Move> moves;
	pos.generate_moves(moves);
	
	// Kiểm tra chiếu hết hoặc hòa cờ
	if (moves.empty()) {
		if (in_check) return Value(-VALUE_MATE + ply);
		return VALUE_DRAW;
	}

	// 3. Sắp xếp nước đi (Move Ordering)
	svec<ScoredMove> scored_moves;
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

		Value val = -search(pos, dq, depth - 1, ply + 1, -beta, -alpha, th);

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

void search_root (Thread& th) {
	Position& pos = th.pos;
	StateListPtr& dq = th.history;

	int max_depth = (Threads.limits.depth > 0) ? Threads.limits.depth : 64;
	Move best_root_move = MOVE_NONE;

	auto start_time = std::chrono::steady_clock::now();
	Threads.limits.start_time = std::chrono::duration_cast<std::chrono::milliseconds>(start_time.time_since_epoch()).count();

	for (int depth = 1; depth <= max_depth; ++depth) {
		if (Threads.stop_search) break;

		Value best_val = -VALUE_INFINITE;
		Value alpha = -VALUE_INFINITE;
		Value beta = VALUE_INFINITE;

		svec<Move> moves;
		pos.generate_moves(moves);
		
		svec<ScoredMove> scored_moves;
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
			if (Threads.stop_search) break;

			Move m = sm.move;
			dq->emplace_back();
			pos.do_move(m, dq->back());

			Value val = -search(pos, dq, depth - 1, 1, -beta, -alpha, th);

			pos.undo_move();
			dq->pop_back();

			if (val > best_val) {
				best_val = val;
				current_best_move = m;
				alpha = val;
			}
		}

		if (!Threads.stop_search) {
			best_root_move = current_best_move;

			auto now = std::chrono::steady_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
			std::cout << "info depth " << depth 
					  << " score cp " << best_val 
					  << " nodes " << th.nodes 
					  << " time " << elapsed 
					  << " pv " << move_to_str(best_root_move) << std::endl;
		}
	}

	if (th.id == 0) {
		std::cout << "bestmove " << move_to_str(best_root_move) << std::endl;
	}
}

