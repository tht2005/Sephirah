#include "thread.h"
#include "search.h"
#include <iostream>

ThreadPool Threads;

Thread::Thread(size_t id) : id(id), exit(false), searching(false) {
	states = std::unique_ptr<std::deque<StateInfo>>(new std::deque<StateInfo>);

	stdThread = std::thread(&Thread::idle_loop, this);
}

Thread::~Thread() {
	mutex.lock();
	exit = true;
	cv.notify_one();
	mutex.unlock();
	if (stdThread.joinable()) stdThread.join();
}

void Thread::idle_loop() {
	while (true) {
		std::unique_lock<std::mutex> lk(mutex);
		cv.wait(lk, [&]{ return searching || exit; });

		if (exit) return;

		// Perform the search
		search_root(*this);

		searching = false;
		// In Lazy SMP, helper threads just stop and wait when done
	}
}

void Thread::start_searching() {
	std::lock_guard<std::mutex> lk(mutex);
	searching = true;
	nodes = 0;
	cv.notify_one();
}

void Thread::wait_for_search_finished() {
	// For Thread 0 (Main), we just block until logic says stop
	// For now, this function is mostly for joining helpers later
	while (searching) {
		std::this_thread::yield();
	}
}

void ThreadPool::init() {
	// Create Main Thread (ID 0) - Wrapper for main execution
	threads.push_back(new Thread(0));

	// Future: Add `threads.push_back(new Thread(i))` loop here for Multithreading
}

void ThreadPool::stop() {
	stop_search = true;
}

void ThreadPool::start_thinking(Position& pos, StateListPtr& states, const SearchLimits& limits) {
	this->limits = limits;
	stop_search = false;

	Thread* mainThread = threads[0];

	mainThread->pos = pos;

	mainThread->states->clear();
	StateInfo* prevPtr = nullptr;
	for (const auto& st : *states) {
		mainThread->states->push_back(st);
		mainThread->states->back().prev = prevPtr;
		prevPtr = &mainThread->states->back();
	}
	mainThread->pos.set_state_pointer(mainThread->states->back());

	mainThread->start_searching();
}
