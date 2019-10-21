#pragma once

#include <os/time.h>

namespace stlx {

namespace Bench {
template <typename duration_t = std::chrono::milliseconds>
struct Function {
	Function(const char * str_)
		: str(str_), start(Time::HighRes::Now()) {}

	~Function() {
		std::cout << "BENCH " << str << ": " << std::chrono::duration_cast<duration_t>(
			Time::HighRes::Now() - start).count() << " "
			<< DurationName() << std::endl;
	}

	constexpr const char * DurationName() const;

	Time::HighRes::Val start;
	const char * str;
};

constexpr const char * Function<std::chrono::seconds>::DurationName() const { return "s"; }
constexpr const char * Function<std::chrono::milliseconds>::DurationName() const { return "ms"; }
constexpr const char * Function<std::chrono::microseconds>::DurationName() const { return "us"; }
constexpr const char * Function<std::chrono::nanoseconds>::DurationName() const { return "ns"; }

};

};

#define BENCHFN		Bench::Function<std::chrono::milliseconds> __benchfn(__FUNCTION__);
#define BENCHFN_US	Bench::Function<std::chrono::microseconds> __benchfn(__FUNCTION__);
#define BENCHFN_NS	Bench::Function<std::chrono::nanoseconds> __benchfn(__FUNCTION__);
#define BENCH(x)		auto x = Time::HighRes::Now();
#define BENCH_END(x)			Log::Warning("BENCH {}: {}ms", #x, std::chrono::duration_cast<std::chrono::milliseconds>(Time::HighRes::Now() - x).count());
#define BENCH_END_US(x)		Log::Warning("BENCH {}: {}us", #x, std::chrono::duration_cast<std::chrono::microseconds>(Time::HighRes::Now() - x).count());
#define BENCH_END_NS(x)		Log::Warning("BENCH {}: {}ns", #x, std::chrono::duration_cast<std::chrono::nanoseconds> (Time::HighRes::Now() - x).count());