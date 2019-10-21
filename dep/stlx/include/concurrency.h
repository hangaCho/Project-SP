#pragma once
#include <atomic>

namespace stlx {

class lock_guard_flag {	// class with destructor that unlocks an std::atomic_flag mutex
public:
	explicit lock_guard_flag(std::atomic_flag & m)
		: _mutex(m) {	// construct and lock
		while(_mutex.test_and_set())
			;
	}

	lock_guard_flag(std::atomic_flag & m, std::memory_order order)
		: _mutex(m) {
		while(_mutex.test_and_set(order))
			;
	}

	~lock_guard_flag() _NOEXCEPT
	{	// unlock
		_mutex.clear();
	}

	lock_guard_flag(const lock_guard_flag&) = delete;
	lock_guard_flag& operator=(const lock_guard_flag&) = delete;

private:
	std::atomic_flag & _mutex;
};

template <typename T>
struct MutexedObjectInstance {
	MutexedObjectInstance(std::atomic_flag & m, T * t)
		: _guard(m), _t(t) {}
	~MutexedObjectInstance() {}
	MutexedObjectInstance(const MutexedObjectInstance&) {
		static_assert(false, "This function only exists to provide copy ellision (RVO). This class is NOT copyable.");
	}

	T* operator ->() const { return _t; }
	operator T*() const { return _t; }
	T * Get() const { return _t; }

private:
	lock_guard_flag _guard;
	T * const _t;
};

template <class T>
struct MutexedObject {
	MutexedObject() {
		_mutex.clear();
	}

	MutexedObjectInstance<T> Lock() {
		return MutexedObjectInstance<T>(_mutex, &_t);
	}

	T * Unsafe() { return &_t; }
	const T * Unsafe() const { return &_t; }

private:
	T _t;
	std::atomic_flag _mutex;
};

}; // namespace stlx