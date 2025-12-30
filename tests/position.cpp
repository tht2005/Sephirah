#include "types.h"
#include "position.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

// const int perft_cnt[] = { 1, 20, 400 };
const int perft_cnt[] = { 1, 20, 400, 8902, 197281, 4865609, 119060324 };
const int MAXD = sizeof(perft_cnt) / sizeof(perft_cnt[0]);
const std::string initialFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

TEST(Position, initialState) {
	StateListPtr dq(new std::deque<StateInfo>());
	dq->emplace_back();
	Position pos;
	pos.set(initialFEN, dq->back());

	for (Square i = SQ_A1; i < SQ_NB; ++i) {
		switch (i) {
			// White back rank
			case SQ_A1: ASSERT_EQ(pos.piece_on(i), W_ROOK); break;
			case SQ_B1: ASSERT_EQ(pos.piece_on(i), W_KNIGHT); break;
			case SQ_C1: ASSERT_EQ(pos.piece_on(i), W_BISHOP); break;
			case SQ_D1: ASSERT_EQ(pos.piece_on(i), W_QUEEN); break;
			case SQ_E1: ASSERT_EQ(pos.piece_on(i), W_KING); break;
			case SQ_F1: ASSERT_EQ(pos.piece_on(i), W_BISHOP); break;
			case SQ_G1: ASSERT_EQ(pos.piece_on(i), W_KNIGHT); break;
			case SQ_H1: ASSERT_EQ(pos.piece_on(i), W_ROOK); break;

			// White pawns
			case SQ_A2: case SQ_B2: case SQ_C2: case SQ_D2:
			case SQ_E2: case SQ_F2: case SQ_G2: case SQ_H2:
				ASSERT_EQ(pos.piece_on(i), W_PAWN);
				break;

			// Black back rank
			case SQ_A8: ASSERT_EQ(pos.piece_on(i), B_ROOK); break;
			case SQ_B8: ASSERT_EQ(pos.piece_on(i), B_KNIGHT); break;
			case SQ_C8: ASSERT_EQ(pos.piece_on(i), B_BISHOP); break;
			case SQ_D8: ASSERT_EQ(pos.piece_on(i), B_QUEEN); break;
			case SQ_E8: ASSERT_EQ(pos.piece_on(i), B_KING); break;
			case SQ_F8: ASSERT_EQ(pos.piece_on(i), B_BISHOP); break;
			case SQ_G8: ASSERT_EQ(pos.piece_on(i), B_KNIGHT); break;
			case SQ_H8: ASSERT_EQ(pos.piece_on(i), B_ROOK); break;

			// Black pawns
			case SQ_A7: case SQ_B7: case SQ_C7: case SQ_D7:
			case SQ_E7: case SQ_F7: case SQ_G7: case SQ_H7:
				ASSERT_EQ(pos.piece_on(i), B_PAWN);
				break;

			// Empty squares
			default:
				ASSERT_EQ(pos.piece_on(i), NO_PIECE);
				break;
		}
	}

	// Optional: check side to move
	ASSERT_EQ(pos.side_to_move(), WHITE);

	// Optional: castling rights
	// ASSERT_TRUE(pos.can_castle_kingside(WHITE));
	// ASSERT_TRUE(pos.can_castle_queenside(WHITE));
	// ASSERT_TRUE(pos.can_castle_kingside(BLACK));
	// ASSERT_TRUE(pos.can_castle_queenside(BLACK));

	// Optional: en passant square
	ASSERT_EQ(pos.ep_square(), SQ_NONE);
}

void dfs(Position& p, int cur_d, int cnt[], StateListPtr& dq) {
	++cnt[cur_d];
	if (cur_d + 1 >= MAXD) return;
	std::vector<Move> moves;
	p.generate_moves(moves);
	for (Move m : moves) {
		dq->emplace_back();
		p.do_move(m, dq->back());
		dfs(p, cur_d + 1, cnt, dq);
		p.undo_move();
		// dq->pop_back();
	}
}

TEST(Position, perft) {
	StateListPtr dq(new std::deque<StateInfo>());
	dq->emplace_back();
	Position pos;
	pos.set(initialFEN, dq->back());

	int cnt[100]; memset(cnt, 0, sizeof(cnt));
	dfs(pos, 0, cnt, dq);
	for (int i = 0; i < MAXD; ++i) {
		std::cout << perft_cnt[i] << ' ' << cnt[i] << '\n';
	}
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
