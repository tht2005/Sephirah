#include "uci.h"
#include "option.h"
#include "position.h"
#include "sephirah.h"
#include "types.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

const std::string startpos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

namespace UCI {

void uci() {
	std::cout << "id name " SEPHIRAH_NAME " " SEPHIRAH_VERSION << std::endl;
	std::cout << "id author " SEPHIRAH_AUTHOR << std::endl;
	std::cout << std::endl;
	for (const auto& [_, op] : Options) {
		std::cout << op << std::endl;
	}
	std::cout << "uciok" << std::endl;
}

void ucinewgame(Position& pos, StateListPtr& dq) {
	dq->clear();
	dq->emplace_back();
	pos.set(startpos, dq->back());
}

void position(std::istringstream& ss, Position& pos, StateListPtr& dq) {
	std::string fen;
	std::string tmp;
	ss >> tmp;
	if (tmp == "startpos") fen = startpos;
	else {
		ss >> fen;
	}
	dq->clear();
	dq->emplace_back();
	pos.set(fen, dq->back());

	ss >> tmp; 
	while (ss >> tmp) {
		Move m = pos.string_to_move(tmp);
		dq->emplace_back();
		pos.do_move(m, dq->back());
	}
}

void go(std::istringstream& ss, Position& pos, StateListPtr& dq) {
	uint64_t btime, wtime;
	std::string name, value;
	
	while (ss >> name) {
		if (name == "btime") {
			ss >> btime;
		} else if (name == "wtime") {
			ss >> wtime;
		} else {
			ss >> value;
		}
	}

	Move m = find_best_move(pos, dq, 9);
	std::cout << "bestmove " << move_to_str(m) << std::endl;
}

void setoption(std::istringstream& ss, std::string& token) {
	std::string name;
	ss >> token >> name;
	while (ss >> token && token != "value")
		name = name + " " + token;
	if (Options.count(name) == 0) {
		std::cout << "Invalid option '" << name << "'" << std::endl;
		return;
	}
	Option& op = Options[name];
	if (op.type != "button") {
		ss >> token;
		if (op.type == "spin") {
			int value;
			ss >> value;
			if (op.min <= value && value <= op.max) {
				op.value = value;
			}
		} else {
			ss >> token;
			std::transform(token.cbegin(), token.cend(), token.begin(),
				[](unsigned char c){ return std::tolower(c); });
			op.value = token;
		}
	}
	if (op.on_change)
		op.on_change(op);
}

int main(int argc, char **argv) {
	Position pos;
	StateListPtr dq(new std::deque<StateInfo>());

	ucinewgame(pos, dq);

	std::cout << SEPHIRAH_NAME " " SEPHIRAH_VERSION " by " SEPHIRAH_AUTHOR << std::endl;
	while (1) {
		std::string cmd;
		std::getline(std::cin, cmd);
		std::istringstream ss(cmd);
		std::string token;

		if (!(ss >> std::skipws >> token)) {
			continue;
		}

		if (token == "uci") uci();
		else if (token == "isready") std::cout << "readyok" << std::endl;
		else if (token == "setoption") setoption(ss, token);
		else if (token == "ucinewgame") ucinewgame(pos, dq);
		else if (token == "position") position(ss, pos, dq);
		else if (token == "go") go(ss, pos, dq);
		else if (token == "quit") break;
		else  std::cout << "No such command '" << token << "'" << std::endl;
	}
	return 0;
}

}