#include "transposition.h"
#include "option.h"
#include "types.h"
#include <cassert>
#include <cstddef>
#include <vector>

TranspositionTable ttable;

TTEntry::TTEntry() : key(0) {}
TTEntry::TTEntry(Key k_, Move m_, Score e_, Score val_, uint8_t gen_, bool pv,
		BoundType b_, uint8_t d_)
	: key(k_), move(m_), eval(e_), value(val_),
	genbound(make_genbound(gen_, pv, b_)), depth(d_) {}

bool can_replace(TTEntry old, TTEntry nw) {
	// TODO implement proper can_replace
	return true;
}
size_t TranspositionTable::size() {
	return entries.size();
}
size_t get_slot(Key k, size_t size) {
	return k & (size - 1);
}
size_t get_tt_size() {
	int hash_mb = get_option_hash();
	size_t tt_size = ((size_t) hash_mb * 1024 * 1024) / sizeof(TTEntry);
	tt_size = 1ULL << (63 - __builtin_clzll(tt_size));
	assert((tt_size & (tt_size - 1)) == 0); // assert tt_size = 2^k
	return tt_size;
}

void TranspositionTable::init() {
	size_t tt_size = get_tt_size();
	entries.resize(tt_size);
}

void TranspositionTable::on_hash_change(const Option& op) {
	size_t tt_size = get_tt_size();
	ttable.change_size(tt_size);
}

void TranspositionTable::change_size(size_t nsize) {
	// TODO: pause all thread

	std::vector<TTEntry> new_entries(nsize);
	for (size_t i = 0; i < entries.size(); ++i) {
		size_t j = get_slot(entries[i].key, new_entries.size());
		if (new_entries[j].key == 0 || can_replace(new_entries[j], entries[i])) {
			new_entries[j] = entries[i];
		}
	}
	entries.swap(new_entries);

	// TODO: continue all thread
}

TTEntry TranspositionTable::get(Key k) {
	return entries[get_slot(k, entries.size())];
}

void TranspositionTable::set(Key k, const TTEntry& entry) {
	entries[get_slot(k, entries.size())] = entry;
}

