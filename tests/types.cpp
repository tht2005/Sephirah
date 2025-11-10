#include <gtest/gtest.h>
#include "types.h"

TEST(types, piece) {
	ASSERT_EQ(make_piece(WHITE, PAWN), W_PAWN);
	ASSERT_EQ(make_piece(WHITE, ROOK), W_ROOK);
	ASSERT_EQ(make_piece(WHITE, KNIGHT), W_KNIGHT);
	ASSERT_EQ(make_piece(WHITE, BISHOP), W_BISHOP);
	ASSERT_EQ(make_piece(WHITE, QUEEN), W_QUEEN);
	ASSERT_EQ(make_piece(WHITE, KING), W_KING);

	ASSERT_EQ(make_piece(BLACK, PAWN), B_PAWN);
	ASSERT_EQ(make_piece(BLACK, ROOK), B_ROOK);
	ASSERT_EQ(make_piece(BLACK, KNIGHT), B_KNIGHT);
	ASSERT_EQ(make_piece(BLACK, BISHOP), B_BISHOP);
	ASSERT_EQ(make_piece(BLACK, QUEEN), B_QUEEN);
	ASSERT_EQ(make_piece(BLACK, KING), B_KING);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
