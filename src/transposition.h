#ifndef TRANSPOSITION_H_INCLUDED
#define TRANSPOSITION_H_INCLUDED

#include "option.h"
#include "types.h"
#include <cstddef>
#include <cstdint>

struct TTEntry {
	uint64_t key;
	uint16_t move;
	int16_t eval;
	int16_t value;
	uint8_t genbound; // 5 bit for generation, 1 bit for pv node, 2 bit for bound type
	uint8_t depth;

	TTEntry();
	TTEntry(Key k_, Move m_, Score e_, Score val_, uint8_t gen_, bool pv, Bound b_, uint8_t d_);
};

class TranspositionTable {
public:
	static void on_hash_change(const Option& op);

	void init();
	void change_size(size_t nsize);
	TTEntry get(Key k);
	void set(Key k, const TTEntry& entry);
	size_t size();
	void clear();

	// Helpers for Mate Score normalization
	static Value value_to_tt(Value v, int ply);
	static Value value_from_tt(Value v, int ply);

private:
	std::vector<TTEntry> entries;
};

extern TranspositionTable ttable;

#endif
