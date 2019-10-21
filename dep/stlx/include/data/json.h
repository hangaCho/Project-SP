#pragma once

#include "../detail/json.h"
#include <iostream>
#include <fstream>


namespace stlx {

inline detail::basic_json<> LoadJson(const String & file_path) {
	using namespace detail;

	StringStream ss;

	{
		std::ifstream file(file_path);
		if(!file.is_open()) {
			throw std::exception(String("Failed to open file " + file_path).c_str());
		}

		ss << file.rdbuf();
	}

	return json::parse(ss.str());
};

};