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

	ASSERT_EQ(get_color(W_PAWN), WHITE);
	ASSERT_EQ(get_color(W_ROOK), WHITE);
	ASSERT_EQ(get_color(W_KNIGHT), WHITE);
	ASSERT_EQ(get_color(W_BISHOP), WHITE);
	ASSERT_EQ(get_color(W_QUEEN), WHITE);
	ASSERT_EQ(get_color(W_KING), WHITE);

	ASSERT_EQ(get_color(B_PAWN), BLACK);
	ASSERT_EQ(get_color(B_ROOK), BLACK);
	ASSERT_EQ(get_color(B_KNIGHT), BLACK);
	ASSERT_EQ(get_color(B_BISHOP), BLACK);
	ASSERT_EQ(get_color(B_QUEEN), BLACK);
	ASSERT_EQ(get_color(B_KING), BLACK);

	ASSERT_EQ(get_piece_type(W_PAWN), PAWN);
	ASSERT_EQ(get_piece_type(W_ROOK), ROOK);
	ASSERT_EQ(get_piece_type(W_KNIGHT), KNIGHT);
	ASSERT_EQ(get_piece_type(W_BISHOP), BISHOP);
	ASSERT_EQ(get_piece_type(W_QUEEN), QUEEN);
	ASSERT_EQ(get_piece_type(W_KING), KING);

	ASSERT_EQ(get_piece_type(B_PAWN), PAWN);
	ASSERT_EQ(get_piece_type(B_ROOK), ROOK);
	ASSERT_EQ(get_piece_type(B_KNIGHT), KNIGHT);
	ASSERT_EQ(get_piece_type(B_BISHOP), BISHOP);
	ASSERT_EQ(get_piece_type(B_QUEEN), QUEEN);
	ASSERT_EQ(get_piece_type(B_KING), KING);
}

TEST(types, square) {
	ASSERT_EQ(make_square(FILE_A, RANK_1), SQ_A1);
	ASSERT_EQ(make_square(FILE_A, RANK_2), SQ_A2);
	ASSERT_EQ(make_square(FILE_A, RANK_3), SQ_A3);
	ASSERT_EQ(make_square(FILE_A, RANK_4), SQ_A4);
	ASSERT_EQ(make_square(FILE_A, RANK_5), SQ_A5);
	ASSERT_EQ(make_square(FILE_A, RANK_6), SQ_A6);
	ASSERT_EQ(make_square(FILE_A, RANK_7), SQ_A7);
	ASSERT_EQ(make_square(FILE_A, RANK_8), SQ_A8);
	ASSERT_EQ(make_square(FILE_B, RANK_1), SQ_B1);
	ASSERT_EQ(make_square(FILE_B, RANK_2), SQ_B2);
	ASSERT_EQ(make_square(FILE_B, RANK_3), SQ_B3);
	ASSERT_EQ(make_square(FILE_B, RANK_4), SQ_B4);
	ASSERT_EQ(make_square(FILE_B, RANK_5), SQ_B5);
	ASSERT_EQ(make_square(FILE_B, RANK_6), SQ_B6);
	ASSERT_EQ(make_square(FILE_B, RANK_7), SQ_B7);
	ASSERT_EQ(make_square(FILE_B, RANK_8), SQ_B8);
	ASSERT_EQ(make_square(FILE_C, RANK_1), SQ_C1);
	ASSERT_EQ(make_square(FILE_C, RANK_2), SQ_C2);
	ASSERT_EQ(make_square(FILE_C, RANK_3), SQ_C3);
	ASSERT_EQ(make_square(FILE_C, RANK_4), SQ_C4);
	ASSERT_EQ(make_square(FILE_C, RANK_5), SQ_C5);
	ASSERT_EQ(make_square(FILE_C, RANK_6), SQ_C6);
	ASSERT_EQ(make_square(FILE_C, RANK_7), SQ_C7);
	ASSERT_EQ(make_square(FILE_C, RANK_8), SQ_C8);
	ASSERT_EQ(make_square(FILE_D, RANK_1), SQ_D1);
	ASSERT_EQ(make_square(FILE_D, RANK_2), SQ_D2);
	ASSERT_EQ(make_square(FILE_D, RANK_3), SQ_D3);
	ASSERT_EQ(make_square(FILE_D, RANK_4), SQ_D4);
	ASSERT_EQ(make_square(FILE_D, RANK_5), SQ_D5);
	ASSERT_EQ(make_square(FILE_D, RANK_6), SQ_D6);
	ASSERT_EQ(make_square(FILE_D, RANK_7), SQ_D7);
	ASSERT_EQ(make_square(FILE_D, RANK_8), SQ_D8);
	ASSERT_EQ(make_square(FILE_E, RANK_1), SQ_E1);
	ASSERT_EQ(make_square(FILE_E, RANK_2), SQ_E2);
	ASSERT_EQ(make_square(FILE_E, RANK_3), SQ_E3);
	ASSERT_EQ(make_square(FILE_E, RANK_4), SQ_E4);
	ASSERT_EQ(make_square(FILE_E, RANK_5), SQ_E5);
	ASSERT_EQ(make_square(FILE_E, RANK_6), SQ_E6);
	ASSERT_EQ(make_square(FILE_E, RANK_7), SQ_E7);
	ASSERT_EQ(make_square(FILE_E, RANK_8), SQ_E8);
	ASSERT_EQ(make_square(FILE_F, RANK_1), SQ_F1);
	ASSERT_EQ(make_square(FILE_F, RANK_2), SQ_F2);
	ASSERT_EQ(make_square(FILE_F, RANK_3), SQ_F3);
	ASSERT_EQ(make_square(FILE_F, RANK_4), SQ_F4);
	ASSERT_EQ(make_square(FILE_F, RANK_5), SQ_F5);
	ASSERT_EQ(make_square(FILE_F, RANK_6), SQ_F6);
	ASSERT_EQ(make_square(FILE_F, RANK_7), SQ_F7);
	ASSERT_EQ(make_square(FILE_F, RANK_8), SQ_F8);
	ASSERT_EQ(make_square(FILE_G, RANK_1), SQ_G1);
	ASSERT_EQ(make_square(FILE_G, RANK_2), SQ_G2);
	ASSERT_EQ(make_square(FILE_G, RANK_3), SQ_G3);
	ASSERT_EQ(make_square(FILE_G, RANK_4), SQ_G4);
	ASSERT_EQ(make_square(FILE_G, RANK_5), SQ_G5);
	ASSERT_EQ(make_square(FILE_G, RANK_6), SQ_G6);
	ASSERT_EQ(make_square(FILE_G, RANK_7), SQ_G7);
	ASSERT_EQ(make_square(FILE_G, RANK_8), SQ_G8);
	ASSERT_EQ(make_square(FILE_H, RANK_1), SQ_H1);
	ASSERT_EQ(make_square(FILE_H, RANK_2), SQ_H2);
	ASSERT_EQ(make_square(FILE_H, RANK_3), SQ_H3);
	ASSERT_EQ(make_square(FILE_H, RANK_4), SQ_H4);
	ASSERT_EQ(make_square(FILE_H, RANK_5), SQ_H5);
	ASSERT_EQ(make_square(FILE_H, RANK_6), SQ_H6);
	ASSERT_EQ(make_square(FILE_H, RANK_7), SQ_H7);
	ASSERT_EQ(make_square(FILE_H, RANK_8), SQ_H8);

	ASSERT_EQ(get_file(SQ_C1), FILE_C);
	ASSERT_EQ(get_file(SQ_A7), FILE_A);
	ASSERT_EQ(get_file(SQ_H3), FILE_H);

	ASSERT_EQ(get_rank(SQ_C1), RANK_1);
	ASSERT_EQ(get_rank(SQ_A3), RANK_3);
	ASSERT_EQ(get_rank(SQ_H7), RANK_7);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
