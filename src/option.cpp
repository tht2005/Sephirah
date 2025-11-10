#include "option.h"

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
