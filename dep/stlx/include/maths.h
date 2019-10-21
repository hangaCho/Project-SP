#pragma once

#include <os/time.h>
#define _USE_MATH_DEFINES
#include <math.h>

namespace stlx {

struct Color3 { float r, g, b; };
struct Color4 {
	/*Color4(float r, float g, float b)
		: r(r), g(g), b(b), a(1.f) {}*/
	Color4(float r, float g, float b, float a)
		: r(r), g(g), b(b), a(a) {}
	float r, g, b, a;

	//sf::Color ToSF() {
	//	return sf::Color(r * 255, g * 255, b * 255, a * 255);
	//}
};

#define TO_DEG(radians)	(radians * (float)((double)180. / M_PI))
#define TO_RAD(degrees)	(degrees * (float)(M_PI / (double)180.))
#define ANGLEDIFF(a, b)		(M_PI - std::fabs(std::fmod(std::fabs(a - b), 2 * M_PI) - M_PI))

template <typename PointT, typename LenT>
PointT limitvec(PointT src, PointT dst, LenT len) {
	auto vec = dst - src;
	auto angle = atan2f(vec.y, vec.x);

	return PointT(_pos.x + cos(angle) * len, _pos.y + sin(angle) * len);
}

template<typename A, typename B> A logb(A n, B b) {
	return log(n) / log(b);
}

// previously 'truncate'
template<typename A> A Clamp(const A & n, const A & nmin, const A & nmax) {
	return stlx::Min(stlx::Max(n, nmin), nmax);
}
// Returns the linear blend of x and y
inline float Mix(float x, float y, float a) {
	return fma((y - x), a, x);
}
inline double Mix(double x, double y, double a) {
	return fma((y - x), a, x);
}

inline u32 NextPow2(u32 n) {
	const u32 MantissaMask = (1 << 23) - 1;

	(*(float*)&n) = (float)n;
	n = (n + MantissaMask) & ~MantissaMask;
	n = (unsigned)(*(float*)&n);

	return n;
}

#define IsPowerOfTwo(n)		(n != 0 && ((n - 1) & n) == 0)


namespace Random {
	int Int(int min, int max);
	// [0,max]
	int Int(int max = std::numeric_limits<int>::max());

	uint UInt(uint min, uint max);
	// [0,max]
	uint UInt(uint max = std::numeric_limits<uint>::max());


	bool Chance(uint denominator);
	// prefer the (uint denominator) version as it is the fastest
	bool Chance(double chance);
	bool Chance(uint numerator, uint denominator);
}

inline double randn_notrig(double mu = 0.0, double sigma = 1.0) {
	static bool deviateAvailable = false;	//	flag
	static double storedDeviate;			//	deviate from previous calculation
	double polar, rsquared, var1, var2;

	//	If no deviate has been stored, the polar Box-Muller transformation is 
	//	performed, producing two independent normally-distributed random
	//	deviates.  One is stored for the next round, and one is returned.
	if(!deviateAvailable) {

		//	choose pairs of uniformly distributed deviates, discarding those 
		//	that don't fall within the unit circle
		do {
			var1 = 2.0*(double(std::rand()) / double(RAND_MAX)) - 1.0;
			var2 = 2.0*(double(std::rand()) / double(RAND_MAX)) - 1.0;
			rsquared = var1*var1 + var2*var2;
		} while(rsquared >= 1.0 || rsquared == 0.0);

		//	calculate polar tranformation for each deviate
		polar = sqrt(-2.0*log(rsquared) / rsquared);

		//	store first deviate and set flag
		storedDeviate = var1*polar;
		deviateAvailable = true;

		//	return second deviate
		return var2*polar*sigma + mu;
	}

	//	If a deviate is available from a previous call to this function, it is
	//	returned, and the flag is set to false.
	else {
		deviateAvailable = false;
		return storedDeviate*sigma + mu;
	}
}

inline i32 ndice(int _throw, int range) {
	i32 i, ret;

	if(range <= 0 || _throw <= 0) return 0;

	ret = 0;
	for(i = 1; i <= _throw; i++) {

		ret += (rand() % range) + 1;
	}

	return ret;
}

inline i32 dice(int _throw, int range) {
	if(_throw > 10)
		return (i32)(
			(_throw * range / 2) + (_throw * randn_notrig() / 3) + (_throw / 2)
			);
	else
		return ndice(_throw, range);
	//for decimal mean values
	//return (_throw * range / 2) + (_throw * randn_notrig()/3) + ((range%2) / 2.0) + (_throw / 2);
}

// generates a value [0,'max']
// prefer using uniform_int_distribution where speed isn't critical
inline u32 random(u32 max) {
	return (rand() % (max + 1));
}

// generates a value ['lowest','highest']
// prefer using uniform_int_distribution where speed isn't critical
inline u32 random(u32 lowest, u32 highest) {
	u32 range = highest - lowest + 1;

	if(range <= 0)
		range = lowest;

	return (rand() % range) + lowest;
}
inline i32 randomi(i32 lowest, i32 highest) {
	i32 range = highest - lowest + 1;

	if(range <= 0)
		range = lowest;

	return (rand() % range) + lowest;
}

// generates a value ['lowest','highest'] using an engine
// prefer using uniform_int_distribution where speed isn't critical
template <typename Engine>
inline u32 random(Engine & engine, u32 lowest, u32 highest) {
	u32 range = highest - lowest + 1;

	if(range <= 0)
		range = lowest;

	return (engine() % range) + lowest;
}

inline bool Prob(float probability) {
	return ((float)rand() / (float)RAND_MAX) <= probability;
}

inline bool Prob(int denominator) {
	return (random(0, --denominator) == 0);
}

inline bool Prob(u32 numerator, u32 denominator) {
	return (random(1, denominator) <= numerator);
}

struct ProbVal {
	u8 n;
	bool Roll()							const { return random(1, 100) <= n; }
	ProbVal & operator =(u8 val) { n = val; return *this; }

};

struct ProbValF {
	float n;
	bool Roll()							const { return ((float)rand() / RAND_MAX) <= n; }
	ProbValF & operator =(float val) { n = val; return *this; }
};

#define GetNibble(var, index)		(  ( var & ( 0xF << (4*index) ) ) >> (4*index)  )
#define GetBit(var, index)			( (var & (1 << index)) >> index )
#define ToBit(index)					(1 << index)

inline void SetNibble(u64 &var, u8 index, u8 val) {
	index *= 4;
	var &= ~(0xFull << index);
	var |= (u64)val << index;
}
inline void SetNibble(u32 &var, u8 index, u8 val) {
	index *= 4;
	var &= ~(0xF << index);
	var |= val << index;
}
inline void SetNibble(u16 &var, u8 index, u8 val) {
	index *= 4;
	var &= ~(0xF << index);
	var |= val << index;
}
template<typename T>  inline void SetBit(T &var, u8 index, bool val) {
	var &= std::numeric_limits<T>::max() - (0x1 << index);
	var |= val << index;
}

};