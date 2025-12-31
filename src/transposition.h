#ifndef TRANSPOSITION_H_INCLUDED
#define TRANSPOSITION_H_INCLUDED

#include "option.h"
#include "types.h"
#include <atomic>
#include <cstddef>
#include <cstdint>

// TODO lockless shared hash table or something ...
struct TTEntry {
	uint16_t key; // truncate key from 64 bit to 16 bit
	uint16_t move;
	uint16_t eval;
	uint16_t value;
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
private:
	std::atomic<uint64_t> packed_key;
	std::vector<TTEntry> entries;
};

extern TranspositionTable ttable;

#endif
