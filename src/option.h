#ifndef OPTION_H_INCLUDED
#define OPTION_H_INCLUDED

#include <functional>
#include <iostream>
#include <map>
#include <ostream>
#include <string>
#include <variant>

struct Option;

typedef std::function<void(const Option&)> opt_func_t;

/**
 * @class Option
 * @brief Store an uci option
 *
 */
struct Option {
	/**
	 * @brief 
	 *
	 * @param name_ Option name
	 * @param type_ Option type (check, string)
	 * @param defaultstr_ default string value
	 */
	Option(std::string name_, std::string type_, std::string defaultstr_, opt_func_t func = nullptr);
	/**
	 * @brief Constructor for spin options
	 *
	 * @param name_ Option name
	 * @param defaultint_ default int value
	 * @param min_ minimum int value
	 * @param max_ maximum int value
	 */
	Option(std::string name_, int defaultint_, int min_, int max_, opt_func_t func = nullptr);

	/**
	 * @brief Constructor for button options
	 *
	 */
	Option(std::string name_, opt_func_t func = nullptr);

	Option() = default;
	Option(const Option&) = default;
	Option& operator= (const Option&) = delete;

	std::string name;
	std::string type;
	std::variant<std::string, int> defaultvalue;
	std::variant<std::string, int> value;
	int min, max;
	opt_func_t on_change;
};

struct CaseInsensitiveCmp {
	bool operator() (const std::string& a, const std::string& b) const {
		return std::lexicographical_compare(a.begin(), a.end(),
			b.begin(), b.end(), [](char x, char y) {
				return tolower(x) < tolower(y);
			});
	}
};

typedef std::map<std::string, Option, CaseInsensitiveCmp> option_map_t;

inline Option& operator<< (Option& l, const Option& r) {
	l.name = r.name;
	l.type = r.type;
	l.defaultvalue = r.defaultvalue;
	l.value = r.value;
	l.min = r.min;
	l.max = r.max;
	return l;
}

inline std::ostream& operator<< (std::ostream& os, Option op) {
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

#endif
