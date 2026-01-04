#ifndef THREAD_H_INCLUDED
#define THREAD_H_INCLUDED

#include "position.h"
#include "types.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

struct SearchLimits {
	uint64_t time[COLOR_NB];
	uint64_t inc[COLOR_NB];
	uint64_t movestogo;
	uint64_t move_time;
	int depth;
	int nodes;
	int mate;
	bool infinite;
	uint64_t start_time;
	uint64_t allocated_time;
};

class Thread {
public:
	Thread(size_t id);
	virtual ~Thread();

	// The logic loop for the thread
	void idle_loop();

	// Start searching
	void start_searching();

	// Wait for this specific thread to finish
	void wait_for_search_finished();

	// Internal ID
	size_t id;

	// Each thread needs its own board and history to avoid data races
	Position pos;
	StateListPtr history; 

	// Search statistics
	uint64_t nodes;

	// Threading primitives
	std::thread stdThread;
	std::mutex mutex;
	std::condition_variable cv;
	bool exit;
	bool searching;
};

class ThreadPool {
public:
	void init();
	void start_thinking(Position& pos, StateListPtr& states, const SearchLimits& limits);

	// Stop all threads (set flag to true)
	void stop(); 

	// Contains all worker threads
	std::vector<Thread*> threads;

	// Global stop flag (atomic for thread safety)
	std::atomic<bool> stop_search;

	// Shared limits
	SearchLimits limits;

	// Helper to get the main thread
	Thread* main() { return threads[0]; }
};

extern ThreadPool Threads;

#endif
