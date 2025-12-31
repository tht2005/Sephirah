#include "option.h"
#include "transposition.h"

const std::string EMPTY = "<empty>";

option_map_t Options;

void Option::init() {
	Options["Debug Log File"] << Option("Debug Log File", "string", EMPTY);
	Options["Threads"] << Option("Threads", 1, 1, 1024);
	Options["Hash"] << Option("Hash", 16, 1, 33554432, TranspositionTable::on_hash_change);
	Options["Clear Hash"] << Option("Clear Hash");
	Options["Ponder"] << Option("Ponder", "check", "false");
	Options["EvalType"] << Option("EvalType", "string", EMPTY);
}

Option::Option(std::string name_, std::string type_, std::string defaultstr_, opt_func_t on_change_func) :
	name(name_), type(type_), defaultvalue(defaultstr_), value(defaultstr_),
	on_change(on_change_func)
{}

Option::Option(std::string name_, int defaultint_, int min_, int max_, opt_func_t on_change_func) :
	name(name_), type("spin"), defaultvalue(defaultint_), value(defaultint_),
	on_change(on_change_func), min(min_), max(max_)
{}

Option::Option(std::string name_, opt_func_t on_change_func) : name(name_),
	type("button"), on_change(on_change_func) {}

Option& operator<< (Option& l, const Option& r) {
	l.name = r.name;
	l.type = r.type;
	l.defaultvalue = r.defaultvalue;
	l.value = r.value;
	l.min = r.min;
	l.max = r.max;
	return l;
}

std::ostream& operator<< (std::ostream& os, Option op) {
	os << "option name " << op.name << " type " << op.type;
	if (op.type == "spin") {
		std::cout << " default " << std::get<int> (op.defaultvalue)
			<< " min " << op.min << " max " << op.max;
	} else if (op.type == "button") {
		// not print anything
	} else {
		std::cout << " default " << std::get<std::string> (op.defaultvalue);
	}
	return os;
}

