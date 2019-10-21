#pragma once

//#include <list>
//#define LinkedList std::list

//#include <map>
//#define Map std::map
//#define MultiSet std::multiset
//#include <set>
//#define UniSet std::set
//#include <array>
//#include <queue>
//
#include <memory> 
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <bitset>
#include <array>

#include <functional>

#ifndef FORCEINLINE
#define FORCEINLINE __forceinline
#endif

namespace stlx 
{
template <typename T>
class Container {
public:
	using Cont = T;
	using Iterator = typename T::iterator;
	using ConstIterator = typename T::const_iterator;
	using ValueType = typename T::value_type;

	using Reference = typename Cont::reference;
	using ConstReference = typename Cont::const_reference;
	using SizeType = typename Cont::size_type;

	explicit Container() {}
	explicit Container(SizeType size) : _impl(size) {}
	explicit Container(SizeType size, const ValueType & val) : _impl(size, val) {}
	Container(const Container& rhs) : _impl(rhs._impl) {}
	Container(Container&& rhs) : _impl(std::move(rhs._impl)) {}
	Container& operator=(const Container&) & = default;
	Container& operator=(Container&&) & = default;
	Container(std::initializer_list<ValueType> ilist) : _impl(ilist) {}

	template <class U>
	FORCEINLINE void Fill(U&& val) {
		std::fill(std::begin(_impl), std::end(_impl), std::forward<U>(val));
	}

	template <class... Args>
	FORCEINLINE void Emplace(Args&&... args) {
		_impl.emplace_back(std::forward<Args>(args)...);
	}
	template <class... Args>
	FORCEINLINE void EmplaceFront(Args&&... args) {
		EmplaceAt(std::begin(_impl), std::forward<Args>(args)...);
	}
	template <typename T>
	FORCEINLINE void EmplaceRange(T first, T last) {
		_impl.insert(std::end(_impl), first, last);
	}

	template <class... Args>
	FORCEINLINE auto EmplaceAt(ConstIterator & loc, Args&&... args) {
		_impl.emplace(loc, std::forward<Args>(args)...);
	}

	template <typename U, typename = std::enable_if_t<!std::is_base_of<Container, std::decay_t<U>>::value>>
	FORCEINLINE Container & operator +=(U && val) {
		_impl.emplace_back(std::forward<U>(val));
		return *this;
	}

	FORCEINLINE Container & operator +=(const Container & rhs) {
		_impl.insert(std::end(_impl), std::begin(rhs), std::end(rhs));
		return *this;
	}


	FORCEINLINE auto Remove(ConstIterator it) {
		return _impl.erase(it);
	}
	FORCEINLINE auto Remove(ConstIterator first, ConstIterator last) {
		return _impl.erase(first, last);
	}

	FORCEINLINE auto begin() {
		return std::begin(_impl);
	}
	FORCEINLINE auto begin() const {
		return std::begin(_impl);
	}
	FORCEINLINE auto end() {
		return std::end(_impl);
	}
	FORCEINLINE auto end() const {
		return std::end(_impl);
	}
	FORCEINLINE auto rbegin() {
		return std::rbegin(_impl);
	}
	FORCEINLINE auto rbegin() const {
		return std::rbegin(_impl);
	}
	FORCEINLINE auto rend() {
		return std::rend(_impl);
	}
	FORCEINLINE auto rend() const {
		return std::rend(_impl);
	}

	FORCEINLINE Reference Front() {
		return _impl.front();
	}
	FORCEINLINE ConstReference Front() const {
		return _impl.front();
	}

	FORCEINLINE Reference Back() {
		return _impl.back();
	}
	FORCEINLINE ConstReference Back() const {
		return _impl.back();
	}

	FORCEINLINE bool IsEmpty() const {
		return _impl.empty();
	}
	FORCEINLINE SizeType Size() const {
		return _impl.size();
	}

	FORCEINLINE void Resize(SizeType size) {
		_impl.resize(size);
	}
	FORCEINLINE void Resize(SizeType size, const ValueType & value) {
		_impl.resize(size, value);
	}

	FORCEINLINE void Clear() {
		_impl.clear();
	}

	FORCEINLINE void Reserve(SizeType size) {
		_impl.reserve(size);
	}

	FORCEINLINE operator Cont*() {
		return &_impl;
	}

protected:
	T _impl;
};

template <class T>
class Vector : public Container<std::vector<T>> {
public:
	Vector() {}
//	explicit Vector(SizeType size) : Container(size) {}
// Vector(const Vector& cont) : Container(cont) {}
// Vector(Vector&& rhs) : Container(std::forward<Vector>(rhs)) {}
// Vector& operator=(const Vector&) = default;
// Vector& operator=(Vector&&) = default;

	using Container = Container<std::vector<T>>;

	using Container::Container;
	using Container::operator +=;

	operator const std::vector<T>*() {
		return &_impl;
	}
	operator std::vector<T>*() {
		return &_impl;
	}

	template <typename T>
	FORCEINLINE ConstReference operator[](T pos) const {
		return this->_impl[pos];
	}
	template <typename T>
	FORCEINLINE Reference operator[](T pos) {
		return this->_impl[pos];
	}

	FORCEINLINE void PopBack() {
		this->_impl.pop_back();
	}
	FORCEINLINE void PopFront() {
		_impl.front() = std::move(_impl.back());
		_impl.pop_back();
	}
	FORCEINLINE void PopFrontOrdered() {
		_impl.pop_front();
	}

	FORCEINLINE auto Find(const T& t) {
		return find(std::begin(this->_impl), std::end(this->_impl), t);
	}
	FORCEINLINE auto Find(const T& t) const {
		return find(std::begin(this->_impl), std::end(this->_impl), t);
	}
	template <typename U, typename = std::enable_if_t<!std::is_pointer<U>::value>, typename = void>
	FORCEINLINE auto Find(const U& t) {
		return find(std::begin(this->_impl), std::end(this->_impl), t);
	}
	template <typename U, typename = std::enable_if_t<!std::is_pointer<U>::value>, typename = void>
	FORCEINLINE auto Find(const U& t) const {
		return find(std::begin(this->_impl), std::end(this->_impl), t);
	}

	template <typename U = T, typename Ptr = typename U::element_type, typename = std::enable_if_t<std::is_pointer<Ptr>::value>>
	FORCEINLINE auto Find(const Ptr& ptr) {
		return find_if(std::begin(this->_impl), std::end(this->_impl),
			[&ptr](const T& t) { return t.get() == ptr; });
	}
	template <typename U = T, typename Ptr = typename U::element_type, typename = std::enable_if_t<std::is_pointer<Ptr>::value>>
	FORCEINLINE auto Find(const Ptr& ptr) const {
		return find_if(std::begin(this->_impl), std::end(this->_impl),
			[&ptr](const T& t) { return t.get() == ptr; });
	}

	template <typename Predicate>
	FORCEINLINE auto FindIf(Predicate pred) const {
		return find_if(std::begin(this->_impl), std::end(this->_impl), pred);
	}

	// TODO: Renamed to RemoveOrdered and provide an unerdored Remove by default
	using Container::Remove;
	FORCEINLINE auto Remove(const T& t) {
		return this->_impl.erase( Find(t) );
	}
	template <typename U, typename = std::enable_if_t<!std::is_pointer<U>::value &&
		!std::is_same<U, typename Container::ConstIterator>::value && !std::is_same<U, typename Container::Iterator>::value>, typename = void>
	FORCEINLINE auto Remove(const U& t) {
		return this->_impl.erase(Find(t));
	}
	template <typename U = T, typename Ptr = typename U::element_type, typename = std::enable_if_t<std::is_pointer<Ptr>::value>>
	FORCEINLINE auto Remove(const Ptr& ptr) {
		return this->_impl.erase( Find(ptr) );
	}

	template <typename Predicate>
	FORCEINLINE auto RemoveIf(Predicate pred) {
		return this->_impl.erase(std::remove_if(std::begin(this->_impl), std::end(this->_impl),
			pred), std::end(this->_impl));
	}

	template <typename U>
	FORCEINLINE auto & operator -=(U && args) {
		Remove(std::forward<U>(args));
		return *this;
	}	

	FORCEINLINE void FillMem(char val) {
		//std::memset((void*)&_impl[0], val, sizeof(ValueType) * Size());
		std::fill((char*)&_impl[0], (char*)&_impl[0] + _impl.size() * sizeof(ValueType), val);
	}

	// Warning: container must container N number of elements where N is a multiple of 4
	FORCEINLINE void FillMem32(uint32_t val) {
		// This block runs fastest on i7-3930k for Size() = 278784
 		using CopyT = u32;
		CopyT * p = (CopyT*)&_impl[0];
  		int sz = Size() * sizeof(ValueType) / sizeof(CopyT);
  		//#pragma omp parallel num_threads(2)
  		for(int i = 0; i < sz; ++i) {
 			p[i] = 0x7F7F7F7F;
 			//p[i] = 0x7F7F7F7F7F7F7F7Full;
  		}

// 		ValueType val_cast = val;
// 		for(auto & v : _impl)
// 			v = val_cast;

		//std::fill((uint32_t*)&_impl[0], (uint32_t*)&_impl[0] + _impl.size() / (sizeof(int) / sizeof(ValueType)), val);
	}

};


template <class T>
class __KeyMap : public Container<T> {
public:
	using Container = Container<T>;
	using Key = typename Container::Cont::key_type;
	using MappedType = typename Container::Cont::mapped_type;
	using SizeType = typename Container::Cont::size_type;

	explicit __KeyMap() {}
	explicit __KeyMap(SizeType size) : Container(size) {}

	FORCEINLINE MappedType & operator[](Key && key) {
		return _impl[key];
	}
	FORCEINLINE MappedType & operator[](const Key & key) {
		return _impl[key];
	}

	FORCEINLINE MappedType & At(const Key & key) {
		return _impl.at(key);
	}
	FORCEINLINE const MappedType & At(const Key & key) const {
		return _impl.at(key);
	}

	FORCEINLINE auto Find(const Key & key) {
		return this->_impl.find(key);
	}
	FORCEINLINE auto Find(const Key & key) const {
		return this->_impl.find(key);
	}

	FORCEINLINE auto Count(const Key & key) const {
		return this->_impl.count(key);
	}

	FORCEINLINE bool Contains(const Key & key) const {
		return _impl.find(key) != _impl.end();
	}

	using Container::Remove;
	FORCEINLINE auto Remove(const Key & key) {
		return this->_impl.erase(key);
	}
};

template <class Key, class T, class Pred = std::less<Key>>
class OrderedMap : public __KeyMap < std::map<Key, T, Pred> > {
public:
	using Container = __KeyMap < std::map<Key, T, Pred> >;

	explicit OrderedMap() {}
	explicit OrderedMap(typename Container::SizeType size) : __KeyMap(size) {}
};

template <class Key, class T>
class HashMap : public __KeyMap < std::unordered_map<Key, T> > {
public:
	using Container = __KeyMap < std::unordered_map<Key, T> >;

	explicit HashMap() {}
	explicit HashMap(typename Container::SizeType size) : __KeyMap(size) {}
};

template <class T, class Pred = std::less<T>>
class OrderedSet : public Container < std::set<T, Pred> > {
public:
	using Container = Container < std::set<T, Pred> >;

	explicit OrderedSet() {}
	explicit OrderedSet(typename Container::SizeType size) : __KeyMap(size) {}

	FORCEINLINE bool Contains(const T & val) const {
		return _impl.find(val) != _impl.end();
	}

	FORCEINLINE auto Remove(const T & val) {
		return this->_impl.erase(val);
	}

	template <typename U, typename = std::enable_if_t<!std::is_same<OrderedSet, std::decay_t<U>>::value>>
	FORCEINLINE OrderedSet & operator +=(U && val) {
		_impl.emplace(std::forward<U>(val));
		return *this;
	}

	FORCEINLINE OrderedSet & operator +=(const OrderedSet & rhs) {
		_impl.insert(std::end(this->_impl), std::begin(rhs), std::end(rhs));
		return *this;
	}
};

template <class T>
class HashSet : public Container < std::unordered_set<T> > {
public:
	using Container = Container < std::unordered_set<T> >;
	explicit HashSet() {}

	template <class... Args>
	FORCEINLINE void Emplace(Args&&... args) {
		_impl.emplace(std::forward<Args>(args)...);
	}

	using Container::Remove;
	FORCEINLINE auto Remove(const T& t) {
		return this->_impl.erase(t);
	}

	FORCEINLINE bool Contains(const T & val) const {
		return _impl.find(val) != _impl.end();
	}

	template <typename U, typename = std::enable_if_t<!std::is_base_of<Container, std::decay_t<U>>::value>>
	FORCEINLINE HashSet & operator +=(U && val) {
		_impl.emplace(std::forward<U>(val));
		return *this;
	}

	FORCEINLINE HashSet & operator +=(const HashSet & rhs) {
		_impl.insert(std::end(this->_impl), std::begin(rhs), std::end(rhs));
		return *this;
	}
};


template<typename T, typename Number = int>
class PriorityQueue {
public:
	using Element = std::pair < Number, T >;
	using Cont = std::priority_queue < Element, std::vector<Element>,
		std::greater < Element >> ;
	using SizeType = typename Cont::size_type;

	FORCEINLINE bool IsEmpty() const { return _impl.empty(); }

	template <class... Args>
	FORCEINLINE void Emplace(Number priority, Args&&... args) {
		_impl.emplace(priority, std::forward<Args>(args)...);
	}

	FORCEINLINE T Pop() {
		T best_item = _impl.top().second;
		_impl.pop();
		return best_item;
	}

	FORCEINLINE SizeType Size() const {
		return _impl.size();
	}

private:
	Cont _impl;
};

} // namespace stlx
