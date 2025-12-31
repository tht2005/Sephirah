#include "bitboard.h"
#include "types.h"
#include "position.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

TEST(Position, initialState) {
	const std::string initialFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

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

void dfs(Position& p, int cur_d, int cnt[], StateListPtr& dq, const int MAXD) {
	++cnt[cur_d];
	if (cur_d + 1 >= MAXD) return;

	std::vector<Move> moves;
	p.generate_moves(moves);

	for (Move m : moves) {
		dq->emplace_back();

		p.do_move(m, dq->back());
		dfs(p, cur_d + 1, cnt, dq, MAXD);
		p.undo_move();
		dq->pop_back();
	}
}

TEST(Position, perft_1) {
	const int perft_cnt[] = { 1, 20, 400, 8902, 197281, 4865609 };
	const int MAXD = sizeof(perft_cnt) / sizeof(perft_cnt[0]);
	const std::string initialFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

	StateListPtr dq(new std::deque<StateInfo>());
	dq->emplace_back();
	Position pos;
	pos.set(initialFEN, dq->back());

	int cnt[100]; memset(cnt, 0, sizeof(cnt));
	dfs(pos, 0, cnt, dq, MAXD);
	for (int i = 0; i < MAXD; ++i) {
		std::cout << perft_cnt[i] << ' ' << cnt[i] << '\n';
		ASSERT_EQ(perft_cnt[i], cnt[i]);
	}
}

TEST(Position, perft_2) {
	const int perft_cnt[] = { 1, 48, 2039, 97862, 4085603 };
	const int MAXD = sizeof(perft_cnt) / sizeof(perft_cnt[0]);
	const std::string initialFEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";

	StateListPtr dq(new std::deque<StateInfo>());
	dq->emplace_back();
	Position pos;
	pos.set(initialFEN, dq->back());

	int cnt[100]; memset(cnt, 0, sizeof(cnt));
	dfs(pos, 0, cnt, dq, MAXD);
	for (int i = 0; i < MAXD; ++i) {
		std::cout << perft_cnt[i] << ' ' << cnt[i] << '\n';
		ASSERT_EQ(perft_cnt[i], cnt[i]);
	}
}

TEST(Position, perft_3) {
	const int perft_cnt[] = { 1, 14, 191, 2812, 43238 };
	const int MAXD = sizeof(perft_cnt) / sizeof(perft_cnt[0]);
	const std::string initialFEN = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1 ";

	StateListPtr dq(new std::deque<StateInfo>());
	dq->emplace_back();
	Position pos;
	pos.set(initialFEN, dq->back());

	int cnt[100]; memset(cnt, 0, sizeof(cnt));
	dfs(pos, 0, cnt, dq, MAXD);
	for (int i = 0; i < MAXD; ++i) {
		std::cout << perft_cnt[i] << ' ' << cnt[i] << '\n';
		ASSERT_EQ(perft_cnt[i], cnt[i]);
	}
}

TEST(Position, perft_5) {
	const int perft_cnt[] = { 1, 44, 1486, 62379, 2103487 };
	const int MAXD = sizeof(perft_cnt) / sizeof(perft_cnt[0]);
	const std::string initialFEN = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";

	StateListPtr dq(new std::deque<StateInfo>());
	dq->emplace_back();
	Position pos;
	pos.set(initialFEN, dq->back());

	int cnt[100]; memset(cnt, 0, sizeof(cnt));
	dfs(pos, 0, cnt, dq, MAXD);
	for (int i = 0; i < MAXD; ++i) {
		std::cout << perft_cnt[i] << ' ' << cnt[i] << '\n';
		ASSERT_EQ(perft_cnt[i], cnt[i]);
	}
}

TEST(Position, perft_6) {
	const int perft_cnt[] = { 1, 46, 2079, 89890, 3894594 };
	const int MAXD = sizeof(perft_cnt) / sizeof(perft_cnt[0]);
	const std::string initialFEN = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";

	StateListPtr dq(new std::deque<StateInfo>());
	dq->emplace_back();
	Position pos;
	pos.set(initialFEN, dq->back());

	int cnt[100]; memset(cnt, 0, sizeof(cnt));
	dfs(pos, 0, cnt, dq, MAXD);
	for (int i = 0; i < MAXD; ++i) {
		std::cout << perft_cnt[i] << ' ' << cnt[i] << '\n';
		ASSERT_EQ(perft_cnt[i], cnt[i]);
	}
}

TEST(Position, castling) {
	// const int perft_cnt[] = { 1, 48, 2039, 97862, 4085603 };
	// const int MAXD = sizeof(perft_cnt) / sizeof(perft_cnt[0]);
	const std::string initialFEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";

	StateListPtr dq(new std::deque<StateInfo>());
	dq->emplace_back();
	Position pos;
	pos.set(initialFEN, dq->back());

	Move m = make_move(SQ_E1, SQ_G1);
	m |= CASTLING;
	
	pos.print_board();
	dq->emplace_back();
	pos.do_move(m, dq->back());
	pos.print_board();

	pos.undo_move();
	pos.print_board();
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
