#pragma once
#include <list>
#include <sstream>
#include <functional>
#include <stlxtypes.h>

namespace stlx {

struct ParamParser {
public:

	void Parse(int argc, char** argv);
	void Parse(const char *args);
	bool Match(const String &match, std::function<void(const String&)> func = [](const String&) {});
	template<typename T> bool Match(const String &match, T &ret) {
		return Match(match, [&](const String &str) { StringStream os(str); os >> ret; });
	}
	bool Match(const String &match, String &ret);
private:
	std::list<String> _params;
};

};