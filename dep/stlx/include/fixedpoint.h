#pragma once
#include "stlxtypes.h"

template <typename T = i32, T Denominator = 100>
class FixedPoint {
private:
	T _val = 0;

public:
	T GetVal() const {
		return _val;
	}

	FixedPoint() {}
	FixedPoint(double rhs)
		: _val(rhs * (double)Denominator + 0.5) {}
	FixedPoint(const FixedPoint & rhs)
		: _val(rhs.GetVal()) {}
	~FixedPoint() {}

	template <typename U = i32>
	String ToString(U threshold = 20) {
		char buf[64];
		auto cents = abs(_val % 100);

		if(_val <= threshold * 100 && cents != 0)
			sprintf(buf, "%i.%02i", _val / 100, cents);
		else
			sprintf(buf, "%i", _val / 100);

		return buf;
	}

	void operator =(T rhs) {
		_val = rhs * Denominator;
	}
	void operator =(double rhs) {
		_val = rhs * (double)Denominator + 0.5;
	}
	void operator =(const FixedPoint & rhs) {
		_val = rhs.GetVal();
	}

	void operator +=(double rhs) {
		_val += (T)(rhs * Denominator + 0.5);
	}
	void operator -=(double rhs) {
		_val -= (T)(rhs * Denominator + 0.5);
	}
	void operator +=(const FixedPoint & rhs) {
		_val += rhs.GetVal();
	}
	void operator -=(const FixedPoint & rhs) {
		_val -= rhs.GetVal();
	}

	void operator *=(T rhs) {
		_val *= rhs;
	}
	void operator /=(T rhs) {
		_val /= rhs;
	}


	FixedPoint operator *(T rhs) const {
		FixedPoint fp(*this);
		fp *= rhs;
		return fp;
	}
	FixedPoint operator /(T rhs) const {
		FixedPoint fp(*this);
		fp /= rhs;
		return fp;
	}

	explicit operator T() {
		return _val / Denominator;
	}

	//explicit operator float() {
	//	return _val / (float)Denominator;
	//}
	//explicit operator float() const {
	//	return _val / (float)Denominator;
	//}
	explicit operator double() {
		return _val / (float)Denominator;
	}
	explicit operator double() const {
		return _val / (float)Denominator;
	}
};