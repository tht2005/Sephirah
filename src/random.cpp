#include <cstdint>
#include <random>

std::mt19937_64 rng(15717128868204243465ULL);

uint64_t random_u64() {
	return uint64_t(rng());
}
