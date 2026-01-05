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

int score_move(const Position& pos, Move m, const Thread& th, int ply) {
	// 1. PV Move (Handled by checking against ttMove in the loop, usually gave max score)
	
	// 2. Captures (MVV-LVA)
	if (pos.piece_on(to_sq(m)) != NO_PIECE || type_of(m) == PROMOTION || type_of(m) == ENPASSANT) {
		int score = 0;
		if (type_of(m) == PROMOTION) score += 20000;
		
		Piece attacker = pos.piece_on(from_sq(m));
		Piece victim = (type_of(m) == ENPASSANT) ? make_piece(flip_color(pos.side_to_move()), PAWN) 
												 : pos.piece_on(to_sq(m));
		
		if (victim != NO_PIECE) {
			score += 10000 + 10 * get_piece_type(victim) - get_piece_type(attacker);
		}
		return score;
	}

	// 3. Killer Moves (Quiet moves that caused a cutoff recently)
	if (ply < MAX_PLY) {
		if (m == th.killers[ply][0]) return 9000;
		if (m == th.killers[ply][1]) return 8000;
	}

	// 4. History Heuristic (Quiet moves that are generally good)
	// Cap value to avoid overflow if needed, or just return raw history
	return th.history[pos.piece_on(from_sq(m))][to_sq(m)];
}

// Update History logic (call this when a quiet move fails high)
void update_history(Thread& th, Move m, int depth) {
	Piece p = th.pos.piece_on(from_sq(m));
	Square to = to_sq(m);
	// Bonus proportional to depth squared (deep cutoffs are more valuable)
	int bonus = depth * depth;
	
	// Clamp to prevent overflow
	if (th.history[p][to] < 20000) 
		th.history[p][to] += bonus;
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
		int s = score_move(pos, m, th, MAX_PLY);
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

	if (!in_check && depth >= 3 && pos.has_non_pawn_material(pos.side_to_move())) {
		int R = (depth > 6) ? 3 : 2;

		dq->emplace_back();
		pos.do_null_move(dq->back());

		Value nullValue = -search(pos, dq, depth - 1 - R, ply + 1, -beta, Value(-beta + 1), th);

		pos.undo_null_move();
		dq->pop_back();

		if (Threads.stop_search) return VALUE_ZERO;
		if (nullValue >= beta) {
			if (nullValue >= VALUE_MATE_IN_MAX_PLY) return beta;
			return nullValue;
		}
	}

	if (depth < 4 && !in_check && alpha < VALUE_MATE_IN_MAX_PLY && beta > VALUE_MATED_IN_MAX_PLY) {
		int static_eval = eval(pos);
		int margin = 128 * depth;
		if (static_eval + margin < alpha) {
			return qsearch(pos, alpha, beta, th);
		}
	}

	svec<Move> moves;
	pos.generate_moves(moves);
	if (moves.empty()) {
		if (in_check) return Value(-VALUE_MATE + ply);
		return VALUE_DRAW;
	}

	svec<ScoredMove> scored_moves;
	for (Move m : moves) {
		int s = score_move(pos, m, th, ply);
		if (m == tt_move) s += 40000;
		scored_moves.push_back({m, s});
	}
	std::sort(scored_moves.begin(), scored_moves.end(), [](const ScoredMove& a, const ScoredMove& b) {
		return a.score > b.score;
	});

	Value best_val = -VALUE_INFINITE;
	Move best_move = MOVE_NONE;
	Bound bound = BOUND_UPPER;

	int moves_searched = 0;

	for (const auto& sm : scored_moves) {
		Move m = sm.move;
		bool is_capture = (pos.piece_on(to_sq(m)) != NO_PIECE) || (type_of(m) == PROMOTION);

		dq->emplace_back();
		pos.do_move(m, dq->back());

		Value val;
		if (moves_searched == 0) {
			val = -search(pos, dq, depth - 1, ply + 1, -beta, -alpha, th);
		} else {
			// Late Moves
			// Calculation Reduction (LMR)
			int reduction = 0;
			// Conditions: Depth is high, move is ordered late, not a capture/check
			if (depth >= 3 && moves_searched > 3 && !is_capture && !in_check) {
				reduction = 1;
				if (moves_searched > 8) reduction = 2; // Reduce more for very late moves
				if (depth > 8) reduction += 1; // Reduce more at high depth
			}

			// Search with Zero Window (Null Window) + Reduction
			// We expect this move to fail low (val <= alpha)
			val = -search(pos, dq, depth - 1 - reduction, ply + 1, Value(-alpha - 1), -alpha, th);

			// Re-search 1: If LMR failed (move was better than expected), search again unreduced (but still Zero Window)
			if (val > alpha && reduction > 0) {
				val = -search(pos, dq, depth - 1, ply + 1, Value(-alpha - 1), -alpha, th);
			}

			// Re-search 2: If Zero Window failed (move improves alpha), search again with Full Window
			if (val > alpha && val < beta) {
				val = -search(pos, dq, depth - 1, ply + 1, -beta, -alpha, th);
			}
		}

		pos.undo_move();
		dq->pop_back();

		if (Threads.stop_search) return VALUE_ZERO;
		++moves_searched;

		if (val > best_val) {
			best_val = val;
			best_move = m;
			if (val > alpha) {
				alpha = val;
				bound = BOUND_EXACT; // Tìm thấy nước tốt, cập nhật thành Exact Bound
			}
		}

		if (alpha >= beta) {
			// Beta Cutoff (Fail High)
			bound = BOUND_LOWER;
			
			// --- UPDATE KILLER & HISTORY HEURISTICS ---
			if (!is_capture && ply < MAX_PLY) {
				// Store Killer
				if (th.killers[ply][0] != m) {
					th.killers[ply][1] = th.killers[ply][0];
					th.killers[ply][0] = m;
				}
				// Update History
				update_history(th, m, depth);
			}
			break; 
		}
	}

	Value tt_val_to_store = TranspositionTable::value_to_tt(best_val, ply);
	ttable.set(key, TTEntry(key, best_move, SCORE_ZERO, Score(tt_val_to_store), 0, false, bound, depth));

	return best_val;
}

void search_root (Thread& th) {
	Position& pos = th.pos;
	StateListPtr& dq = th.states;

	th.clear_heuristics();

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
			int s = score_move(pos, m, th, MAX_PLY);
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

