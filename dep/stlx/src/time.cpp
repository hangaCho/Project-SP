#include <os/time.h>
#include <windows.h>
#include <limits>

namespace stlx {

Time::TimeObject _time_object;

namespace Time {
	thread_local Val _cachedTime;
	thread_local Val _delta = 0;
	thread_local float _deltaf = 0.f;


	TimeObject::TimeObject() {
		TIMECAPS tc;

		if(timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR) {
			// Error; application can't continue.
			assert(0);
		}

		_timer_res = tc.wPeriodMin;
		if(timeBeginPeriod(_timer_res) != TIMERR_NOERROR)
			assert(0);
	}

	TimeObject::~TimeObject() {
		timeEndPeriod(_timer_res);
	}


	void Update()
	{
		//assert(_time_object._timer_res != 0);
		auto now = timeGetTime();
		_delta = now - _cachedTime;
		_deltaf = (now - _cachedTime) / 1000.f;
		_cachedTime = now;
		// possible 64-bit time implementation
		// test the arithmetic!
		//	_cachedTime += now - (u32)_cachedTime;
	}

	void Update(Time::Val delta) {
		_cachedTime += delta;
		_delta = delta;
		_deltaf = delta / 1000.f;
	}

	Val Now()
	{
		//assert(_time_object._timer_res != 0);
		return _cachedTime;
	}

	Val Delta() {
		return _delta;
	}

	float DeltaF() {
		return _deltaf;
	}

	Val Elapsed(Val t)
	{
		auto now = _cachedTime;
		if(now < t)
			return t + (std::numeric_limits<Val>::max() - now); // 32-bit looping
		else
			return (now - t);
	}

	bool Elapsed(Val t, Val period)
	{
		return Elapsed(t) >= period;
	}

	Val ElapsedReal(Val t) {
		auto now = timeGetTime();
		if(now < t)
			return t + (std::numeric_limits<Val>::max() - now); // 32-bit looping
		else
			return (now - t);
	}

	bool ElapsedReal(Val t, Val period) {
		return ElapsedReal(t) >= period;
	}


	Val Until(Val t)
	{
		if(t < _cachedTime)
			return _cachedTime + (std::numeric_limits<Val>::max() - t); // 32-bit looping
		else
			return (t - _cachedTime);
	}

	// TODO: 32-bit looping checks
	bool Reached(Val t)
	{
		return (_cachedTime >= t);
	}


	// High res
	const auto g_Frequency = []() {
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		return frequency.QuadPart;
	}();

	HighRes::Val HighRes::Now() {
		LARGE_INTEGER count;
		QueryPerformanceCounter(&count);
		return HighRes::Val(duration(count.QuadPart * static_cast<rep>(period::den) / g_Frequency));
	}
} // namespace Time

} // namespace stlx