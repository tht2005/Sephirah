#include "uci.h"
#include "option.h"
#include "sephirah.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

option_map_t OptionMap;

namespace UCI {

void init_option() {
	OptionMap["Debug Log File"] << Option("Debug Log File", "string", "<empty>");
	OptionMap["Threads"] << Option("Threads", 1, 1, 1024);
	OptionMap["Hash"] << Option("Hash", 16, 1, 33554432);
	OptionMap["Clear Hash"] << Option("Clear Hash");
	OptionMap["Ponder"] << Option("Ponder", "check", "false");
	OptionMap["EvalType"] << Option("EvalType", "string", "<empty>");
}

int main(int argc, char **argv) {
	init_option();
	std::cout << SEPHIRAH_NAME " " SEPHIRAH_VERSION " by " SEPHIRAH_AUTHOR << std::endl;
	while (1) {
		std::string cmd;
		std::getline(std::cin, cmd);
		std::istringstream ss(cmd);
		std::string s;
		std::vector<std::string> tokens;
		while (ss >> s) tokens.push_back(s);
		if (tokens.empty()) continue;

		if (tokens[0] == "uci") {
			std::cout << "id name " SEPHIRAH_NAME " " SEPHIRAH_VERSION << std::endl;
			std::cout << "id author " SEPHIRAH_AUTHOR << std::endl;
			std::cout << std::endl;
			for (const auto& [_, op] : OptionMap) {
				std::cout << op << std::endl;
			}
			std::cout << "uciok" << std::endl;
		} else if (tokens[0] == "isready") {
			std::cout << "readyok" << std::endl;
		} else if (tokens[0] == "setoption") {
			if (tokens.size() <= 2 || tokens[1] != "name") {
				std::cout << "Invalid setoption syntax" << std::endl;
				continue;
			}
			int i = 3;
			std::string name = tokens[2];
			while (i < (int)tokens.size() && tokens[i] != "value")
				name = name + " " + tokens[i++];
			if (OptionMap.count(name) == 0) {
				std::cout << "Invalid option '" << name << "'" << std::endl;
				continue;
			}
			Option& op = OptionMap[name];
			if (op.type != "button") {
				if (i + 1 >= (int)tokens.size()) {
					std::cout << "Need to specify value" << std::endl;
					continue;
				}
				std::string valuestr = tokens[i + 1];
				if (op.type == "spin") {
					int value = std::stoi(valuestr);
					if (op.min <= value && value <= op.max) {
						op.value = value;
					}
				} else {
					// convert to lowercase
					std::transform(valuestr.cbegin(), valuestr.cend(), valuestr.begin(),
						[](unsigned char c){ return std::tolower(c); });
					op.value = valuestr;
				}
			}
			if (op.on_change)
				op.on_change(op);
		} else if (tokens[0] == "ucinewgame") {
			//TODO
		} else if (tokens[0] == "position") {
			//TODO
		} else if (tokens[0] == "go") {
			// report best move? update state
		} else if (tokens[0] == "stop") {

		} else if (tokens[0] == "ponderhit") {

		} else if (tokens[0] == "quit") {
			break;
		} else {
			std::cout << "No such command '" << tokens[0] << "'." << std::endl;
		}
	}
	return 0;
}

}
