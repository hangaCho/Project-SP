#pragma once

#include <type_traits>
#include <stlxtypes.h>

namespace stlx {

namespace detail {
	class BinarySerializer abstract {
	public:
		BinarySerializer(char * buffer) 
			: _buf(buffer), _p(buffer)
		{
		}
		~BinarySerializer() {
		}

		auto * Data() const {
			return _buf;
		}

		auto Size() const {
			return std::ptrdiff_t(_p - _buf);
		}

		char * _p;

	protected:
		const char * const _buf;
	};
}

class BinaryWriter : public detail::BinarySerializer {
public:
	using detail::BinarySerializer::BinarySerializer;
	using detail::BinarySerializer::~BinarySerializer;

	void Write() {}

	template <typename T, typename ... Args>
	void Write(T && t, Args && ...args) {
		operator<<(std::forward<T>(t));
		Write(std::forward<Args>(args)...);
	}

	template <typename T, typename = std::enable_if_t<!std::is_pointer<T>::value>>
	BinaryWriter & operator <<(T value) {
		auto p = (T *)_p;
		*p = (T)value;
		_p += sizeof(T);
		return *this;
	}

	BinaryWriter & operator <<(const char * src) {
		strcpy(_p, src);
		_p += strlen(src) + 1;
		return *this;
	}

	BinaryWriter & operator <<(const String & str) {
		auto len = str.size();
		*(u16*) _p = (u16)len;
		_p += sizeof(u16);
		memcpy(_p, str.c_str(), len);
		_p += len;
		return *this;
	}

	template<typename T, typename = std::enable_if_t<std::is_trivially_copyable<T>::value>>
	BinaryWriter & operator <<(const Vector<T> & v) {
		*(u16*)_p = (u16)v.Size();
		_p += sizeof(u16);

		auto sz = v.Size() * sizeof(T);
		if(sz > 0) {
			memcpy(_p, &v.Front(), sz);
			_p += sz;
		}
	}

	BinaryWriter & operator <<(const Vector<String> & v) {
		*(u16*)_p = (u16)v.Size();;
		_p += sizeof(u16);

		for(auto & str : v) {
			auto len = str.size();
			*(u16*)_p = (u16)len;
			_p += sizeof(u16);
			memcpy(_p, str.c_str(), len);
			_p += len;
		}
	}

	void Push(const char * src, u32 len) {
		memcpy(_p, src, len);
		_p += len;
	}
};

class BinaryReader : public detail::BinarySerializer {
public:
	using detail::BinarySerializer::BinarySerializer;
	using detail::BinarySerializer::~BinarySerializer;

	void Read() {}

	template <typename T, typename ... Args>
	void Read(T && t, Args && ...args) {
		operator>>(std::forward<T>(t));
		Read(std::forward<Args>(args)...);
	}

	template <typename T, typename = std::enable_if_t<!std::is_pointer<T>::value>>
	BinaryReader & operator >>(T & value) {
		T * p = (T *)_p;
		value = *p;
		_p += sizeof(T);
		return *this;
	}

	BinaryReader & operator >>(char * dest) {
		u32 len = strlen(_p) + 1;
		memcpy(dest, _p, len);
		_p += len;
		return *this;
	}

	BinaryReader & operator >>(String & str) {
		auto len = *(u16*) _p;
		_p += sizeof(u16);

		str.assign(_p, len);
		_p += len;
		return *this;
	}

	template<typename T, typename = std::enable_if_t<std::is_trivially_copyable<T>::value>>
	BinaryReader & operator >>(Vector<T> & v) {
		auto n = *(u16*)_p;
		_p += sizeof(u16);

		v.Clear();
		auto end = _p + sizeof(T)* n;
		v.Reserve(n);
		v.EmplaceRange((T*)_p, (T*)end);
		_p = end;
	}

	BinaryReader & operator >>(Vector<String> & v) {
		auto n = *(u16*)_p;
		_p += sizeof(u16);

		v.Clear();
		v.Reserve(n);

		String str;
		for(size_t i = 0; i < n; i++) {
			auto len = *(u16*)_p;
			_p += sizeof(u16);

			str.assign(_p, len);
			_p += len;
			v.Emplace(str);
		}
	}

	void Pop(char * dest, u32 len) {
		memcpy(dest, _p, len);
		_p += len;
	}
};


namespace detail {
	class SerializeModeBase {};
}

namespace SerializeMode {
	namespace Binary {
		class Write : private detail::SerializeModeBase {};
		class Read : private detail::SerializeModeBase {};
	}
	namespace SQL {
		class Write : private detail::SerializeModeBase {};
		class Read : private detail::SerializeModeBase {};
	}
	class Size : private detail::SerializeModeBase {};
}; // SerializeMode

namespace detail {

// ------------------------------ SerializerBinary ------------------------------
// ------------------------------------------------------------------------------
template <bool deserializing>
struct SerializerBinary {
	using ReturnType = void;
	static constexpr szt _size = 0;

	//SerializerBinary(char * p) : _p(p) {} // This version doesn't update p, so shouldn't really be used
	SerializerBinary(BinarySerializer * serializer) : _p(serializer->_p) {}
	~SerializerBinary() {}

	template<typename ...Args>
	void Use(Args &&...args) {
		Serialize(std::forward<Args>(args)...);
	}

private:
	void Serialize() {
	}

	template<typename T, typename ...Args,
		typename = std::enable_if_t<std::is_assignable<T&,T>::value>, typename = void>
	void Serialize(T & t, Args &&...args) {
		if(deserializing) {
			T * p = (T *)_p;
			t = *p;
			_p += sizeof(T);
		} else {
			T * p = (T*)_p;
			*p = t;
			_p += sizeof(T);
		}

		Serialize(std::forward<Args>(args)...);
	}

	template<typename T, typename ...Args,
		typename = std::enable_if_t<!std::is_assignable<T&,T>::value>
		typename = std::enable_if_t<std::is_trivially_copyable<T>::value>>
	void Serialize(T & t, Args &&...args) {
		if(deserializing) {
			memcpy(&t, _p, sizeof(T));
			_p += sizeof(T);
		} else {
			memcpy(_p, &t, sizeof(T));
			_p += sizeof(T);
		}

		Serialize(std::forward<Args>(args)...);
	}

	char *& _p;
};

#ifdef SOCI_H_INCLUDED
// ------------------------------ SerializerSQLReader ---------------------------
// ------------------------------------------------------------------------------
struct SerializerSQLReader {
	using ReturnType = void;
	static constexpr szt _size = 0;

	SerializerSQLReader(soci::row * row) : _row(row), _num(0) {}
	~SerializerSQLReader() {}

	template<typename ...Args>
	void Use(Args &&...args) {
		Serialize(std::forward<Args>(args)...);
	}

private:
	void Serialize() {}

	template <typename T>	struct SociType { using Type = T; };
	template<> struct SociType<bool>		{ using Type = i32; };
	template<> struct SociType<u8>		{ using Type = i32; };
	template<> struct SociType<i8>		{ using Type = i32; };
	template<> struct SociType<i16>		{ using Type = i32; };
	template<> struct SociType<u16>		{ using Type = i32; };

	template<typename T, typename ...Args, typename = std::enable_if_t<!std::is_enum<T>::value>,
		typename = std::enable_if_t<std::is_assignable<T&,T>::value>>
	void Serialize(T & t, Args &&...args) {
		using Type = SociType<T>::Type;
		t = (T)_row->get<Type>(_num);
		++_num;

		Serialize(std::forward<Args>(args)...);
	}

	// enums
	template<typename T, typename ...Args, typename = std::enable_if_t<std::is_enum<T>::value>>
	void Serialize(T & t, Args &&...args) {
		using Type = SociType<std::underlying_type_t<T>>::Type;
		t = (T)_row->get<Type>(_num);
		++_num;

		Serialize(std::forward<Args>(args)...);
	}

	template<typename ...Args, szt N>
	void Serialize(Array<bool, N> & t, Args &&...args) {
		String s = _row->get<String>(_num);
		++_num;
		ASSERT(s.size() == N);

		szt i = 0;
		for(auto & c : t)
			c = s[i] == '1';

		Serialize(std::forward<Args>(args)...);
	}


	const soci::row * const _row;
	szt _num;
};


// ------------------------------ SerializerSQLWriter ---------------------------
// ------------------------------------------------------------------------------
struct SerializerSQLWriter {
	using ReturnType = void;
	static constexpr szt _size = 0;

	SerializerSQLWriter(soci::statement * st) : _st(st) {}
	~SerializerSQLWriter() {}

	template<typename ...Args>
	ReturnType Use(Args &&...args) {
		Serialize(std::forward<Args>(args)...);
	}

private:
	void Serialize() {}

	template <typename T>	struct SociType { using Type = T; };
	template<> struct SociType<bool>		{ using Type = u8; };
	template<> struct SociType<i8>		{ using Type = u8; };

	template<typename T, typename = std::enable_if_t<!std::is_enum<T>::value>,
		typename ...Args, typename = void>
	void Serialize(T & t, Args &&...args) {
		using Type = SociType<T>::Type;
		Type & v = (Type&)t;
		_st->exchange(soci::use(v));

		Serialize(std::forward<Args>(args)...);
	}	

	// enums
	template<typename T, typename = std::enable_if_t<std::is_enum<T>::value>, typename ...Args>
	void Serialize(T & t, Args &&...args) {
		using Type = SociType<std::underlying_type_t<T>>::Type;
		Type & v = (Type&)t;
		_st->exchange(soci::use(v));

		Serialize(std::forward<Args>(args)...);
	}


	template<typename ...Args, szt N>
	void Serialize(Array<bool,N> & t, Args &&...args) {
		String s;
		s.resize(N);
		szt i = 0;
		for(const auto & c : t)
			s[i] = c ? '1' : '0';
		_st->exchange(soci::use(s));

		Serialize(std::forward<Args>(args)...);
	}

	soci::statement * const _st;
};

#endif // SOCI_H_INCLUDED

// ------------------------------ SerializerSize -------------------------------
// -----------------------------------------------------------------------------
template <typename T>
struct SerializerSize {
	using ReturnType = szt;
	ReturnType _size = 0;

	SerializerSize(const T*) {}
	~SerializerSize() {}

	template<typename ...Args>
	void Use(Args &&...args) {
		_size += GetSize(std::forward<Args>(args)...);
	}

private:
	static constexpr ReturnType GetSize() { return 0; }

	template<typename T, typename = std::enable_if_t<std::is_scalar<T>::value>, typename ...Args>
	static constexpr ReturnType GetSize(T & t, Args &&...args) {
		return sizeof(t) + GetSize(std::forward<Args>(args)...);
	}
};


template <typename Mode>
using Serializer =
	std::conditional_t< std::is_same<Mode, SerializeMode::Size>::value,
		SerializerSize<void>,
#ifdef SOCI_H_INCLUDED
		std::conditional_t< std::is_same<Mode, SerializeMode::SQL::Write>::value,
		SerializerSQLWriter,
		std::conditional_t< std::is_same<Mode, SerializeMode::SQL::Read>::value,
		SerializerSQLReader,
#endif
		std::conditional_t< std::is_same<Mode, SerializeMode::Binary::Write>::value,
		SerializerBinary<false>,
		std::conditional_t< std::is_same<Mode, SerializeMode::Binary::Read>::value,
		SerializerBinary<true>, void
#ifdef SOCI_H_INCLUDED
	>>
#endif
	>>>;

}; // detail


// ------------------------- Boiler plate macros -------------------------------
// -----------------------------------------------------------------------------
#define SERIALIZE_BEGIN(...)	\
	template <typename Mode, typename T = void> \
	auto Serialize(T * t = nullptr, __VA_ARGS__) { \
		using namespace stlx::detail; \
		static_assert(std::is_base_of<SerializeModeBase, Mode>::value, \
			"Mode must be a mode from SerializeMode"); \
		using Type = Serializer<Mode>; \
		Type serializer(t);

#define SERIALIZE_ADD(...)	\
		serializer.Use(__VA_ARGS__);

#define SERIALIZE_END \
		return Type::ReturnType(serializer._size); \
	}

#define SERIALIZE_IS_READ \
	  (std::is_same<Mode, SerializeMode::SQL::Read>::value \
	|| std::is_same<Mode, SerializeMode::Binary::Read>::value)

#define SERIALIZE_IS_WRITE \
	  (std::is_same<Mode, SerializeMode::SQL::Write>::value \
	|| std::is_same<Mode, SerializeMode::Binary::Write>::value)

}; // stlx