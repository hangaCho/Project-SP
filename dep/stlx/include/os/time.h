#pragma once

#include <stlxtypes.h>
#include <chrono>

namespace stlx {

namespace Time {
	struct TimeObject {
		// Used to query windows high resolution timer
		TimeObject();
		~TimeObject();

		u32 _timer_res = 0;
	};

	using Val = u32;
	//enum class Val : u32 {};
	//namespace Literals {
	//	constexpr Val operator "" _s(u64 n)	{ return (Val)(n * CLOCKS_PER_SEC); }
	//	constexpr Val operator "" _ms(u64 n) { return (Val)(n * CLOCKS_PER_SEC / 1000); }
	//	constexpr Val operator "" _m(u64 n)	{ return (Val)(n * CLOCKS_PER_SEC * 60); }
	//	constexpr Val operator "" _h(u64 n)	{ return (Val)(n * CLOCKS_PER_SEC * 60 * 60); }
	//	constexpr Val operator "" _d(u64 n)	{ return (Val)(n * CLOCKS_PER_SEC * 60 * 60 * 24); }
	//}

	// Updates time. Get the value with Time::Now()
	void Update();
	// Updates time by adding delta to cached value
	void Update(Time::Val delta);

	// Returns time, calculated on Time::Update()
	Val Now();

	// Time elapsed since last Time::Update()
	Val Delta();
	// Time elapsed since last Time::Update() as a float (1.f = 1 second)
	float DeltaF();

	// Returns time elapsed since 't'
	// or whether 't' has reached 'period'
	Val Elapsed(Val t);

	// Elapsed(t) >= period
	bool Elapsed(Val t, Val period);

	// Elapsed by calling real time (not cached)
	Val ElapsedReal(Val t);
	bool ElapsedReal(Val t, Val period);


	// Returns time left until 't'
	Val Until(Val t);

	// Returns whether point in time 't' has been reached
	bool Reached(Val t);

	struct HighRes {
		using rep = long long;
		using period = std::nano;
		using duration = std::chrono::duration<rep, period>;
		using Val = std::chrono::time_point<HighRes>;

		static Val Now();
	};

// Using these for now until user-defined literals are supported
#define _ms					_s / 1000
#define _s					* CLOCKS_PER_SEC
#define _m					_s * 60
#define _h					_m * 60
#define _d					_h * 24

} // namespace Time

} // namespace stlx