#include "transposition.h"
#include "option.h"
#include "types.h"
#include <cassert>
#include <cstddef>
#include <vector>

#ifdef _MSC_VER
#include <intrin.h>
#pragma intrinsic(_BitScanReverse64)
#endif

TranspositionTable ttable;

TTEntry::TTEntry() : key(0), move(MOVE_NONE), eval(0), value(0), genbound(0), depth(0) {}
TTEntry::TTEntry(Key k_, Move m_, Score e_, Score val_, uint8_t gen_, bool pv,
		Bound b_, uint8_t d_)
	: key(uint16_t(k_)), move(m_), eval(e_), value(val_),
	genbound(make_genbound(gen_, pv, b_)), depth(d_) {}

bool can_replace(TTEntry old, TTEntry nw) {
	return nw.depth >= old.depth;
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
#ifdef _MSC_VER
    unsigned long idx;
    if (_BitScanReverse64(&idx, tt_size)) {
        tt_size = 1ULL << idx;
    } else {
        tt_size = 1; 
    }
#else
	tt_size = 1ULL << (63 - __builtin_clzll(tt_size));
#endif
	assert((tt_size & (tt_size - 1)) == 0); 
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
	std::vector<TTEntry> new_entries(nsize);
	for (size_t i = 0; i < entries.size(); ++i) {
		if (entries[i].key != 0) {
			size_t j = get_slot(entries[i].key, new_entries.size());
			new_entries[j] = entries[i];
		}
	}
	entries.swap(new_entries);
}

TTEntry TranspositionTable::get(Key k) {
	return entries[get_slot(k, entries.size())];
}

void TranspositionTable::set(Key k, const TTEntry& entry) {
	size_t idx = get_slot(k, entries.size());
	if (entries[idx].key == 0 || can_replace(entries[idx], entry)) {
		entries[idx] = entry;
	}
}