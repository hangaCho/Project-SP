#include <maths.h>
#include <functional>
#include <random>


namespace stlx {

namespace Random {
	template<class T = std::mt19937, size_t N = T::state_size>
	auto SeededRandomEngine() -> typename std::enable_if<!!N, T>::type {
		typename T::result_type random_data[N];
		std::random_device source;
		std::generate(std::begin(random_data), std::end(random_data), std::ref(source));
		std::seed_seq seeds(std::begin(random_data), std::end(random_data));
		T seededEngine(seeds);
		return seededEngine;
	}

	thread_local std::mt19937 rng(SeededRandomEngine());

	int Int(int min, int max) {
		std::uniform_int_distribution<int> dist(min, max);
		return dist(rng);
	}
	// [0,max]
	int Int(int max) {
		std::uniform_int_distribution<int> dist(0, max);
		return dist(rng);
	}


	uint UInt(uint min, uint max) {
		std::uniform_int_distribution<uint> dist(min, max);
		return dist(rng);
	}
	// [0,max]
	uint UInt(uint max) {
		std::uniform_int_distribution<uint> dist(0, max);
		return dist(rng);
	}

	// pcg32_once_insecure is faster on this function, with about same precision
	// consider std::discrete_distribution to save state between sessions
	bool Chance(uint denominator) {
		return rng() <= rng.max() / denominator;
	}
	bool Chance(double chance) {
		return rng() <= chance * rng.max();
	}
	bool Chance(uint numerator, uint denominator) {
		return rng() <= rng.max() / denominator * numerator;
	}
};

};