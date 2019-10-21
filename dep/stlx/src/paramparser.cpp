#include <os/paramparser.h>
#include <algorithm>
#include <iterator>

namespace stlx {

void ParamParser::Parse(int argc, char** argv) {
	std::copy(argv, argv + argc, back_inserter(_params));
}

void ParamParser::Parse(const char *args) {
	StringStream ss(args);
	std::copy(std::istream_iterator<String>(ss), std::istream_iterator<String>(), std::back_inserter(_params));
}

bool ParamParser::Match(const String &match, std::function<void(const String&)> func) {

	struct Match {

		Match(const String & toMatch) : _toMatch(toMatch) {}

		bool operator()(const String & str) {
			return str.compare(0, _toMatch.size(), _toMatch) == 0;
		}
	private:
		const String &_toMatch;
	};
	auto iter = std::find_if(std::begin(_params), std::end(_params), Match(match));
	if(iter != std::end(_params)) {
		if(func && iter->size() >= match.size() + 1) {
			func(iter->substr(match.size() + 1));
		}
		return true;
	}
	return false;
}

bool ParamParser::Match(const String &match, String &ret) {

	return Match(match, [&](const String & str)
	{
		ret = str;
	});
}

};