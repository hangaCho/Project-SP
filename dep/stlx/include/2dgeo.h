#pragma once

#include <maths.h>

namespace stlx {

template <typename T = int>
struct Point {
	using Type = T;
	T x = 0;
	T y = 0;

	constexpr Point() {}
	constexpr Point(const Point& p) { *this = p; }
	constexpr Point(T _x, T _y) : x(_x), y(_y) {}

	template <typename T, typename = typename std::enable_if<!std::is_floating_point<T>::value>::type>
	bool operator == (const Point<T> & p) const {
		return (p.x == x && p.y == y);
	}

	template <typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type, typename = void>
	bool operator == (const Point<T> & p) const {
		//return (fabs(p.x) == abs(x) && abs(p.y) == abs(y));
		// Make sure maxUlps is non-negative and small enough that the
		// default NAN won't compare as equal to anything.
		const u32 maxUlps = 256;

		int aInt = *(int*)&p.x;
		// Make aInt lexicographically ordered as a twos-complement int
		if(aInt < 0)
			aInt = 0x80000000 - aInt;
		// Make bInt lexicographically ordered as a twos-complement int

		int bInt = *(int*)&x;
		if(bInt < 0)
			bInt = 0x80000000 - bInt;
		int intDiff = abs(aInt - bInt);
		if(intDiff > maxUlps)
			return false;

		aInt = *(int*)&p.y;
		if(aInt < 0)
			aInt = 0x80000000 - aInt;

		bInt = *(int*)&y;
		if(bInt < 0)
			bInt = 0x80000000 - bInt;
		intDiff = abs(aInt - bInt);
		if(intDiff > maxUlps)
			return false;

		return true;
	}

	bool operator != (const Point & p) const {
		return !(p == *this);
	}
	bool operator >= (const Point & p)  const {
		return (x >= p.x && y >= p.y);
	}

	Point operator ++ (int) {
		Point copy = *this;
		x++;
		y++;
		return copy;
	}
	Point operator -- (int) {
		Point copy = *this;
		x--;
		y--;
		return copy;
	}

	Point & operator += (const Point & p) {
		x += p.x;
		y += p.y;
		return *this;
	}
	Point & operator -= (const Point & p) {
		x -= p.x;
		y -= p.y;
		return *this;
	}

	Point & operator += (const T & v) {
		x += v;
		y += v;
		return *this;
	}
	Point & operator -= (const T & v) {
		x -= v;
		y -= v;
		return *this;
	}
	Point & operator *= (const T & v) {
		x *= v;
		y *= v;
		return *this;
	}
	Point & operator /= (const T & v) {
		x /= v;
		y /= v;
		return *this;
	}


	Point operator + (const T & v) const {
		Point copy = *this;
		return (copy += v);
	}
	Point operator - (const T & v) const {
		Point copy = *this;
		return (copy -= v);
	}

	Point operator + (const Point & p) const {
		Point copy = *this;
		return (copy += p);
	}
	Point operator - (const Point & p) const {
		Point copy = *this;
		return (copy -= p);
	}
	Point operator / (const T & v) const {
		Point copy = *this;
		return (copy /= v);
	}
	Point operator * (const T & v) const {
		Point copy = *this;
		return (copy *= v);
	}

	template <typename U>
	Point operator + (const Point<U> & p) const {
		return{x + p.x, y + p.y};
	}
	template <typename U>
	Point operator - (const Point<U> & p) const {
		return{x - p.x, y - p.y};
	}


	void Noise(float val) {
		// TODO: float randomness (use mersenne twister?)
		i32 tmp = (i32)(val * 1000.f);
		x += random(-tmp, tmp) / 1000.f;
		y += random(-tmp, tmp) / 1000.f;
	}

	void Noise(i32 val) {
		x += random(-val, val);
		y += random(-val, val);
	}

	void Noise(i32 val_x, i32 val_y) {
		x += random(-val_x, val_x);
		y += random(-val_y, val_y);
	}

	template <typename LenT>
	Point LimitTowards(Point p, LenT len) {
		auto vec = p - *this;
		if(vec.MagnitudeSq() <= (len * len))
			return p;

		auto angle = atan2(vec.y, vec.x);

		return{x + cos(angle) * len,
				 y + sin(angle) * len};
	}

	template <typename LenT>
	Point ProjectTowards(Point p, LenT len) {
		auto vec = p - *this;
		auto angle = atan2(vec.y, vec.x);

		return{x + cos(angle) * len,
				 y + sin(angle) * len};
	}

	Point RotateAround(Point origin, float angle) {
		Point p;
		auto angle_cos = cos(angle);
		auto angle_sin = sin(-angle);
		auto v = *this - origin;
		p.x = origin.x + angle_cos * v.x - angle_sin * v.y;
		p.y = origin.y + angle_sin * v.x + angle_cos * v.y;
		return p;
	}

	Point Rotate(float angle) {
		Point p;
		auto angle_cos = cos(angle);
		auto angle_sin = sin(-angle);
		p.x = angle_cos * x - angle_sin * y;
		p.y = angle_sin * x + angle_cos * y;
		return p;
	}

	float DistanceTo(Point p) const {
		return sqrt((p.x - x)*(p.x - x) + (p.y - y)*(p.y - y));
	}

	float DistanceToSq(Point p) const {
		return (p.x - x)*(p.x - x) + (p.y - y)*(p.y - y);
	}

	float Magnitude() const {
		return sqrt(x*x + y*y);
	}

	float MagnitudeSq() const {
		return x*x + y*y;
	}

	float Angle() const {
		return atan2(y, x);
	}

	float AngleTo(Point p) const {
		return (p - *this).Angle();
	}

	auto Floor() {
		return Point{(T)floor(x), (T)floor(y)};
	}
	auto Ceil() {
		return Point{(T)ceil(x), (T)ceil(y)};
	}
	auto Round() {
		return Point{(T)round(x), (T)round(y)};
	}
	auto Abs() {
		return Point{(T)abs(x), (T)abs(y)};
	}

	friend std::ostream &operator<<(std::ostream &os, const Point &p) {
		return os << p.x << ',' << p.y;
	}

protected:
	template <class U = Point>
	struct RangeContainer {
		struct iterator {
			iterator() = delete;
			iterator& operator=(const iterator&) = delete;
			iterator(const U src, const U dst)
				: _src(src), _dst(dst), _width(_dst.x - _src.x) {
				// != gets called before the first iteration...
				--_src.x;
			}

			iterator & operator++() {
				// iterator gets incremented in !=
				return *this;
			}

			const U & operator*() const {
				return _src;
			}

			bool operator!=(const iterator & rhs) {
				++_src.x;

				if(_src.x > _dst.x) {
					++_src.y;
					if(_src.y > _dst.y)
						return false;

					_src.x = _dst.x - _width;
				}

				return true;
			}

		private:
			U _src;
			const U _dst;
			const T _width;
		};

		RangeContainer(const U origin, const U range)
			: _it((U&)(origin - range), (U&)(origin + range)) {}

		RangeContainer(const U origin, const T range)
			: _it((const U&)(origin - range), (const U&)(origin + range)) {}

		const iterator & begin() {
			return _it;
		}

		const iterator & end() {
			// != will use _dst on rhs
			return _it;
		}

	private:
		iterator _it;
	};

public:
	RangeContainer<> Range(Point range) const {
		return RangeContainer<>(*this, range);
	}

	RangeContainer<> Range(T range) const {
		return RangeContainer<>(*this, range);
	}
};

struct Position : public Point<float> {
	Position() {}
	Position(Point<float> src, Point<float> dst, float speed) {
		*this = src;
		Go(dst, speed);
	}

	using Point<float>::operator=;

	void Tick() {
		if(_speed == 0)
			return;

		if(_speed >= DistanceTo(_dstpos)) {
			*this = _dstpos;
			_speed = 0;
			//cout << "Ticks taken: " << (Time::Now() - _starttime) / (1000 / TICK_RATE) << " with " << _count << " counts\n";
		} else {
			*this += _vector;
			//_count--;
		}

		//cout << "Pos: " << x << "," << y << "\n";
	}

	Point<float> AsPoint() const {
		return *this;
	}

	Point<float> GetDest() const {
		return _dstpos;
	}

	void Go(Point<float> p, float speed) {
		//_srcpos = *this;
		//_starttime = Time::Now();
		_dstpos = p;
		_speed = speed;

		auto dist = DistanceTo(_dstpos);
		auto factor = _speed / dist;
		//cout << "Should take: " << (i32)(dist / _speed) << "\n";
		//_count = (i32)(dist / _speed);
		_vector = {_dstpos.x - x, _dstpos.y - y};
		_vector *= factor;
		_angle = atan2f(_vector.y, _vector.x);
	}

	void GoKeepAngle(Point<float> p) {
		_dstpos = p;

		auto factor = _speed / DistanceTo(_dstpos);
		_vector = {_dstpos.x - x, _dstpos.y - y};
		_vector *= factor;
	}

	void Stop() {
		auto dst = _dstpos;
		_dstpos = *this;
		_speed = 0;
		TurnTowards(dst);
	}

	void StopOn(Point<float> p) {
		_dstpos = p;

		/*
		auto dst = _dstpos;
		*this = _dstpos = p;
		_movetime = Time::Now();
		_speed = 0;
		TurnTowards(dst);*/
	}

	void Teleport(Point<float> p) {
		TurnTowards(p);
		*this = _dstpos = p;
		_speed = 0;
	}

	void Reset(Point<float> p) {
		*this = _dstpos = p;
		_speed = 0;
		_angle = (Random::UInt(u32(M_PI * 2000.f)) - (float)M_PI * 1000.f) / 1000.f;
		//_angle = random(u32(-M_PI * 1000.f), u32(M_PI * 1000.f)) / 1000.f;
	}

	bool IsMoving() const {
		return _speed != 0;
	}

	float GetAngle() const {
		return _angle;
	}

	float GetAngleTo(Point<float> p) const {
		auto vec = p - *this;
		return atan2f(vec.y, vec.x);
	}

	void TurnTowards(Point<float> dst) {
		auto vec = dst - *this;
		_angle = atan2f(vec.y, vec.x);
		_dstpos = *this;
	}


private:
	Point<float> _dstpos;
	Point<float> _vector;
	float _speed;
	float _angle;

	//Point<float> _srcpos;
	//i32 _count;
	//Time::Val _starttime;
};

//struct Position : public Point<float>
//{
//	Position() {}
//	Position(Point<float> src, Point<float> dst, float speed) {
//		(Point<float>&)*this = src;
//		_dstpos = dst;
//		_speed = speed;
//		auto vec = _dstpos - *this;
//		_angle = atan2f(vec.y, vec.x);
//		_movetime = Time::Now();
//	}
//
//	Point<float> Get() const {
//		auto len = Time::Elapsed(_movetime) * _speed;
//		truncatemax(len, DistanceTo(_dstpos));
//		auto newpos = _dstpos;
//		newpos.FindLimitFrom(*this, len);
//
//		return newpos;
//	}
//
//	Point<float> GetDest() const{
//		return _dstpos;
//	}
//
//	void Go(Point<float> p, float speed) {
//		(Point<float>&)*this = Get();
//		_dstpos = p;
//		_speed = speed;
//		auto vec = _dstpos - *this;
//		_angle = atan2f(vec.y, vec.x);
//
//		_movetime = Time::Now();
//	}
//
//	void GoKeepAngle(Point<float> p) {
//		(Point<float>&)*this = Get();
//		_dstpos = p;
//		_movetime = Time::Now();
//	}
//
//	void Stop() {
//		auto dst = _dstpos;
//		(Point<float>&)*this = _dstpos = Get();
//		_movetime = Time::Now();
//		_speed = 0;
//		TurnTowards(dst);
//	}
//	
//	void StopOn(Point<float> p) {
//		(Point<float>&)*this = Get();
//		_dstpos = p;
//		_movetime = Time::Now();
//
//		/*
//		auto dst = _dstpos;
//		(Point<float>&)*this = _dstpos = p;
//		_movetime = Time::Now();
//		_speed = 0;
//		TurnTowards(dst);*/
//	}
//
//	void Teleport(Point<float> p) {
//		TurnTowards(_dstpos);
//		(Point<float>&)*this = _dstpos = p;
//		_movetime = Time::Now();
//		_speed = 0;
//	}
//
//	void Reset(Point<float> p) {
//		(Point<float>&)*this = _dstpos = p;
//		_movetime = Time::Now();
//		_speed = 0;
//	}
//
//	bool IsMoving() const {
//		return Get() != _dstpos;
//	}
//
//	float GetAngle() const {
//		return _angle;
//	}
//
//	float GetAngleTo( Point<float> p ) const
//	{
//		auto vec = p - *this;
//		return atan2f(vec.y, vec.x);
//	}
//
//	void TurnTowards(Point<float> dst)
//	{
//		auto vec = dst - *this;
//		_angle = atan2f(vec.y, vec.x);
//		(Point<float>&)*this = _dstpos = Get();
//	}
//	
//
//private:
//	Point<float> _dstpos;
//	Time::Val _movetime;
//	float _speed;
//	float _angle;
//};

//template <ConsoleType LogLevel, typename T>
//auto & operator<<(ConsoleOut<LogLevel> & os, const Point<T> & p) {
template <typename T>
std::ostream & operator<<(std::ostream & os, const Point<T> & p) {
	os << p.x << "," << p.y;
	return os;
}

template<typename T>
struct Rect {

	void Set(T _left, T _top, T _right, T _bottom) {
		left = _left;
		top = _top;
		right = _right;
		bottom = _bottom;
	}

	bool Set(const std::string & str) {
		if(sscanf(str.c_str(), "%d,%d,%d,%d", &left, &top, &right, &bottom) == 4) {
			return true;
		}
		return false;
	}

	void SetDimensions(T _width, T _height) {
		right = left + _width;
		bottom = top + _height;
	}

	void SetPosition(T _X, T _Y) {
		right = _X + Width();
		bottom = _Y + Height();
		left = _X;
		top = _Y;
	}

	T Width() const {
		return right - left;
	}

	T Height() const {
		return bottom - top;
	}

	Rect & operator =(const Rect & rect) {
		left = rect.left;
		top = rect.top;
		right = rect.right;
		bottom = rect.bottom;
		return *this;
	}

	T DistanceTo(Point<T> p) const {
		T _max = 0;
		if(p.x < left)
			_max = left - p.x;
		else if(p.x > right)
			_max = p.x - right;

		if(p.y < top)
			_max = Max(_max, top - p.y);
		else if(p.y > bottom)
			_max = Max(_max, p.y - bottom);

		return _max;
	}

	Point<T> RandomPoint() const {
		Point<T> p;
		p.x = random(left, right);
		p.y = random(top, bottom);
		return p;
	}

	bool IsValid() const {
		return !(left == -1 || top == -1 || right == -1 || bottom == -1);
	}

	void Reset() {
		left = -1;
	}

	bool Contains(Point<T> p) const {
		return (p.x >= left && p.y >= top && p.x <= right && p.y <= bottom);
	}
	bool Contains(T x, T y) const {
		return (x >= left && y >= top && x <= right && y <= bottom);
	}

#ifdef _WINDEF_
	RECT ToRECT() const {
		RECT ret = {left, top, right, bottom};
		return ret;
	}
#endif

	template<typename Func>
	void ForEachPoint(Func func) {
		for(T _x = left; _x <= right; _x++) {
			for(T _y = top; _y <= bottom; _y++) {
				func(_x, _y);
			}
		}
	}

	T left;
	T top;
	T right;
	T bottom;
};

template <typename T, typename Func>
void BresenhamFractional(T o, T d, Func func) {
	auto v = d - o;
	v.x = abs(v.x);
	v.y = abs(v.y);

	Point<i16> p = {(i16)o.x, (i16)o.y};

	int n = 1;
	int x_inc, y_inc;
	float error;

	if(v.x == 0) {
		x_inc = 0;
		error = std::numeric_limits<float>::infinity();
	} else if(d.x > o.x) {
		x_inc = 1;
		n += int(d.x) - p.x;
		error = (floor(o.x) + 1 - o.x) * v.y;
	} else {
		x_inc = -1;
		n += p.x - int(d.x);
		error = (o.x - floor(o.x)) * v.y;
	}

	if(v.y == 0) {
		y_inc = 0;
		error -= std::numeric_limits<float>::infinity();
	} else if(d.y > o.y) {
		y_inc = 1;
		n += int(d.y) - p.y;
		error -= (floor(o.y) + 1 - o.y) * v.x;
	} else {
		y_inc = -1;
		n += p.y - int(d.y);
		error -= (o.y - floor(o.y)) * v.x;
	}

	for(; n > 0; --n) {
		if(!func(p))
			return;

		if(error > 0) {
			p.y += y_inc;
			error -= v.x;
		} else {
			p.x += x_inc;
			error += v.y;
		}
	}
}

template <typename T, typename Func, typename U = decltype(T::x),
	typename = std::enable_if_t<!std::is_floating_point<T>::value>>
void Bresenham(T o, T d, Func func) {
	auto v = d - o;
	v.x = abs(v.x);
	v.y = abs(v.y);

	auto n = 1 + v.x + v.y;
	int x_inc = (d.x > o.x) ? 1 : -1;
	int y_inc = (d.y > o.y) ? 1 : -1;
	int error = v.x - v.y;

	v *= 2; // So that we calculate each point at 0.5,0.5
	auto p = o;

	for(; n > 0; --n) {
		if(!func(p))
			return;

		if(error > 0) {
			p.x += x_inc;
			error -= v.y;
		} else if(error < 0) {
			p.y += y_inc;
			error += v.x;
		} else {
			p.x += x_inc;
			error -= v.y;
			p.y += y_inc;
			error += v.x;
			n--;
		}
	}
}

};