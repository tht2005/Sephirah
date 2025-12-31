#include "option.h"
#include "transposition.h"
#include <gtest/gtest.h>

// This is not true anymore :(
// TEST(TTEntry, Align) {
// 	EXPECT_EQ(sizeof(TTEntry), 8);
// }

TEST(TranspositionTable, EmptyTableReturnsDefault) {
	TranspositionTable ttable;
	ttable.init();

	TTEntry entry = ttable.get(0x123456789ABCDEF0ULL);
	EXPECT_EQ(entry.key, 0);
	// Optionally check other default fields if initialized
}

TEST(TranspositionTable, SetAndGetEntry) {
	TranspositionTable ttable;
	ttable.init();

	TTEntry entry;
	entry.key = 0x123456789ABCDEF0ULL;
	entry.move = 42;
	entry.value = 100;
	entry.eval = 90;
	entry.genbound = 1;
	entry.depth = 5;

	ttable.set(entry.key, entry);

	TTEntry fetched = ttable.get(entry.key);

	EXPECT_EQ(fetched.key, entry.key);
	EXPECT_EQ(fetched.move, entry.move);
	EXPECT_EQ(fetched.value, entry.value);
	EXPECT_EQ(fetched.eval, entry.eval);
	EXPECT_EQ(fetched.genbound, entry.genbound);
	EXPECT_EQ(fetched.depth, entry.depth);
}

TEST(TranspositionTable, CollisionOverwrite) {
	TranspositionTable ttable;
	ttable.init();

	TTEntry e1, e2;
	e1.key = 0xAAAA;
	e1.value = 10;

	e2.key = 0xAAAA; // same slot, simulate collision
	e2.value = 20;

	ttable.set(e1.key, e1);
	ttable.set(e2.key, e2); // overwrite

	TTEntry fetched = ttable.get(e1.key);
	EXPECT_EQ(fetched.value, 20); // latest value
}

TEST(TranspositionTable, SlotBounds) {
	TranspositionTable ttable;
	ttable.init();

	size_t size = ttable.size(); // if you have a function returning TT size
	EXPECT_GT(size, 0u);

	for (size_t i = 0; i < 10; ++i) {
		TTEntry entry;
		entry.key = i;
		ttable.set(entry.key, entry);

		TTEntry fetched = ttable.get(entry.key);
		EXPECT_EQ(fetched.key, entry.key);
	}
}

int main(int argc, char **argv)
{
	Option::init();
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
