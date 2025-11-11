#include "uci.h"
#include "option.h"
#include "position.h"
#include "sephirah.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

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
			// convert to lowercase
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
	// StateListPtr stlist;

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
		else if (token == "ucinewgame") ;
		else if (token == "position") ;
		else if (token == "go") ;
		else if (token == "stop") ;
		else if (token == "ponderhit") ;
		else if (token == "quit") break;
		else  std::cout << "No such command '" << token << "'" << std::endl;
	}
	return 0;
}

}
