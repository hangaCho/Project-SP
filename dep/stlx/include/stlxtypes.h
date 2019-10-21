#pragma once

#include <stdint.h>

using u64 = uint64_t;
using i64 = int64_t;
using u32 = uint32_t;
using i32 = int32_t;
using u16 = uint16_t;
using i16 = int16_t;
using u8 = uint8_t;
using i8 = int8_t;
using f32 = float;
using f64 = double;
using szt = size_t;
using uint = unsigned int;


//#define thread_local __declspec(thread)

//-------------------------------------------MACROS----------------------------------------------------------
#include <assert.h>
#define ASSERT(assertion) { if( !(assertion) ) { assert( assertion &&0); } }


#ifndef NO_COMMON_STD

#include <string>
#include <stlx.h>
using namespace std::string_literals;
#include <format.h>
using fmt::format;

namespace stlx {

#ifdef __STDCPP_THR__
using Thread = std::thread;
#endif
template <typename T, typename Deleter = std::default_delete<T>>
	using UniquePtr = std::unique_ptr<T, Deleter>;
template <typename T>
	using WeakPtr = std::weak_ptr<T>;
template <szt Bits>
	using Bitset = std::bitset<Bits>;
template <typename T, szt Size>
	using Array = std::array<T, Size>;
template <typename T>
	using SharedPtr = std::shared_ptr<T>;

template<class _Ty, class... _Types> FORCEINLINE
	typename std::enable_if<!std::is_array<_Ty>::value,
		UniquePtr<_Ty> >::type MakeUnique(_Types&&... _Args) {
		return std::make_unique<_Ty>(std::forward<_Types>(_Args)...);
	}

template<class _Ty> FORCEINLINE
	typename std::enable_if<std::is_array<_Ty>::value && std::extent<_Ty>::value == 0,
		UniquePtr<_Ty> >::type MakeUnique(size_t _Size) {
		typedef typename std::remove_extent<_Ty>::type _Elem;
		return (UniquePtr<_Ty>(new _Elem[_Size]()));
	}

template<class _Ty, class... _Types> FORCEINLINE
typename std::enable_if<!std::is_array<_Ty>::value,
	SharedPtr<_Ty> >::type MakeShared(_Types&&... _Args) {
	return std::make_shared<_Ty>(std::forward<_Types>(_Args)...);
}

template <typename T> FORCEINLINE
T Max(const T & a, const T & b) { return std::max(a, b); }
template <typename T> FORCEINLINE
T Min(const T & a, const T & b) { return std::min(a, b); }

#ifdef _UNICODE
	using String = std::wstring;
#else
	using String = std::string;
#endif
using StringStream = std::stringstream;

#define atoul(str)			strtoul(str, NULL, 10)
#define atoull(str)			_strtoui64(str, NULL, 10)
#define strcpyn(dst, src)	strncpy(dst, src, sizeof(dst))

using Exception = std::exception;

};

#endif