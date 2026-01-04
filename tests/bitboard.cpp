#include "bitboard.h"
#include <gtest/gtest.h>

TEST(Bitboard, PathHorizontalRight) {
    Bitboard b = path_bb(SQ_A1, SQ_D1);
    Bitboard expected = (1ULL << SQ_A1) | (1ULL << SQ_B1) | (1ULL << SQ_C1) | (1ULL << SQ_D1);
    ASSERT_EQ(b, expected);
}

TEST(Bitboard, PathHorizontalLeft) {
    Bitboard b = path_bb(SQ_D1, SQ_A1);
    Bitboard expected = (1ULL << SQ_A1) | (1ULL << SQ_B1) | (1ULL << SQ_C1) | (1ULL << SQ_D1);
    ASSERT_EQ(b, expected);
}

TEST(Bitboard, PathVerticalUp) {
    Bitboard b = path_bb(SQ_A1, SQ_A4);
    Bitboard expected = (1ULL << SQ_A1) | (1ULL << SQ_A2) | (1ULL << SQ_A3) | (1ULL << SQ_A4);
    ASSERT_EQ(b, expected);
}

TEST(Bitboard, PathVerticalDown) {
    Bitboard b = path_bb(SQ_A4, SQ_A1);
    Bitboard expected = (1ULL << SQ_A1) | (1ULL << SQ_A2) | (1ULL << SQ_A3) | (1ULL << SQ_A4);
    ASSERT_EQ(b, expected);
}

TEST(Bitboard, PathSingleSquare) {
    Bitboard b = path_bb(SQ_E5, SQ_E5);
    Bitboard expected = (1ULL << SQ_E5);
    ASSERT_EQ(b, expected);
}

int main(int argc, char **argv)
{
	bitboard::init();
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
