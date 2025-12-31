#include "uci.h"
#include "evaluation.h"
#include "position.h"
#include "types.h"

Value search(Position& pos, StateListPtr& dq, int depth, Value alpha, Value beta) {
	if (depth == 0) return eval(pos);
	Value best = -VALUE_INFINITE, score;

	std::vector<Move> moves;
	pos.generate_moves(moves);
	if (moves.empty()) return eval(pos);

	for (Move m : moves) {
		dq->emplace_back();
		pos.do_move(m, dq->back());

		std::cout << "info move " << move_to_str(m) << std::endl;

		score = search(pos, dq, depth - 1, -beta, -alpha);
		if (score > best) {
			best = score;
		}
		if (score > alpha) {
			alpha = score;
		}
		if (alpha >= beta) {
			break;
		}

		pos.undo_move();
		dq->pop_back();
	}

	return best;
}

Move find_best_move(Position& pos, StateListPtr& dq, int depth) {
	Value best_score = -VALUE_INFINITE;
	Move bestmove = MOVE_NONE;
	Value alpha = -VALUE_INFINITE;
	Value beta = VALUE_INFINITE;
	Value score;

	std::vector<Move> moves;
	pos.generate_moves(moves);
	for (Move m : moves) {
		dq->emplace_back();
		pos.do_move(m, dq->back());

		score = -search(pos, dq, depth - 1, -beta, -alpha);
		if (score > best_score) {
			best_score = score;
			bestmove = m;
		}

		pos.undo_move();
		dq->pop_back();
	}

	return bestmove;
}

