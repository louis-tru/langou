/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __nxkit__util__
#define __nxkit__util__

#if !defined(__cplusplus)
# error "Please use the c++ compiler"
#endif

#if __cplusplus < 201103L
# error "The compiler does not support c++ 11"
#endif

#include "nxkit/object.h"

/**
 * @ns ngui
 */

NX_NS(ngui)

class Error;

namespace console {
	NX_EXPORT void log(char);
	NX_EXPORT void log(byte);
	NX_EXPORT void log(int16);
	NX_EXPORT void log(uint16);
	NX_EXPORT void log(int);
	NX_EXPORT void log(uint);
	NX_EXPORT void log(float);
	NX_EXPORT void log(double);
	NX_EXPORT void log(int64);
	NX_EXPORT void log(uint64);
	NX_EXPORT void log(bool);
	NX_EXPORT void log(cchar*, ...);
	NX_EXPORT void log(cString&);
	NX_EXPORT void log_ucs2(cUcs2String&);
	NX_EXPORT void warn(cchar*, ...);
	NX_EXPORT void warn(cString&);
	NX_EXPORT void error(cchar*, ...);
	NX_EXPORT void error(cString&);
	NX_EXPORT void error(const Error&);
	NX_EXPORT void tag(cchar*, cchar*, ...);
	NX_EXPORT void print(cchar*, ...);
	NX_EXPORT void print(cString&);
	NX_EXPORT void print_err(cchar*, ...);
	NX_EXPORT void print_err(cString&);
	NX_EXPORT void clear();
#if NX_ARCH_32BIT
	NX_EXPORT void log(long);
	NX_EXPORT void log(unsigned long);
#endif 
}

/**
 * @class Console # util log
 */
class NX_EXPORT Console {
 public:
	typedef NonObjectTraits Traits;
	virtual ~Console() = default;
	virtual void log(cString& str);
	virtual void warn(cString& str);
	virtual void error(cString& str);
	virtual void print(cString& str);
	virtual void print_err(cString& str);
	virtual void clear();
	void set_as_default();
};

// ----------------- Number Object -----------------

/**
 * @class Number
 */
template <typename T> class NX_EXPORT Number: public Object {
 public:
	inline Number(T v): value(v) { }
	inline T operator*() { return value; }
	inline Number& operator++() { value++; return *this; } // ++i
	inline Number& operator--() { value--; return *this; } // --i
	inline Number  operator++(int) { T v = value; value++; return v; } // i++
	inline Number  operator--(int) { T v = value; value--; return v; } // i--
	template <typename T2> inline T operator=(T2 v) { value = v.value; return value; }
	template <typename T2> inline bool operator==(T2 v) { return value == v.value; }
	template <typename T2> inline bool operator!=(T2 v) { return value != v.value; }
	template <typename T2> inline bool operator<(T2 v) { return value < v.value; }
	template <typename T2> inline bool operator>(T2 v) { return value > v.value; }
	template <typename T2> inline bool operator<=(T2 v) { return value <= v.value; }
	template <typename T2> inline bool operator>=(T2 v) { return value >= v.value; }
	T value;
	static const T min, max;
};

#define define_number(NAME, T) \
typedef Number<T> NAME; template<> const T NAME::min; template<> const T NAME::max;
	define_number(Bool, bool);
	define_number(Float, float);
	define_number(Double, double);
	define_number(Char, char);
	define_number(Byte, byte);
	define_number(Int16, int16);
	define_number(Uint16, uint16);
	define_number(Int, int);    typedef Number<int>   Int32;
	define_number(Uint, uint);  typedef Number<uint>  Uint32;
	define_number(Int64, int64);
	define_number(Uint64, uint64);
#undef define_number

// ----------------- Number Object END -----------------

/**
 * @class SimpleHash
 */
class NX_EXPORT SimpleHash: public Object {
	uint _hash;
 public:
	inline SimpleHash(): _hash(5381) { }
	inline uint hash_code() { return _hash; }
	inline void clear() { _hash = 5381; }
	String digest();
	template<class T>
	void update(const T* data, uint len) {
		while (len--) _hash += (_hash << 5) + data[len];
	}
};

NX_EXPORT uint hash_code(cchar* data, uint len);
NX_EXPORT String hash(cchar* data, uint len);
NX_EXPORT String hash(cString& str);
NX_EXPORT int random(uint start = 0, uint end = 0x7fffffff);
NX_EXPORT int fix_random(uint a, ...);
NX_EXPORT void fatal(cchar* file, uint line, cchar* func, cchar* msg = 0, ...);
NX_EXPORT uint64 iid();
NX_EXPORT uint   iid32();
NX_EXPORT String version();
NX_EXPORT int64  parse_time(cString& str);
NX_EXPORT String gmt_time_string(int64 second);
NX_EXPORT String platform();
NX_EXPORT void exit(int rc);
NX_EXPORT bool is_exited();

namespace _right_reference {
	// remove_reference
	template <class Tp> struct _remove_reference       { typedef Tp type; };
	template <class Tp> struct _remove_reference<Tp&>  { typedef Tp type; };
	template <class Tp> struct _remove_reference<Tp&&> { typedef Tp type; };

	// is_reference
	template <class Tp> struct _is_lvalue_reference      { static const bool value = false; };
	template <class Tp> struct _is_lvalue_reference<Tp&> { static const bool value = true; };
}

// move

template <class Tp>
NX_INLINE constexpr typename _right_reference::_remove_reference<Tp>::type&& move(Tp&& t) {
	typedef typename _right_reference::_remove_reference<Tp>::type Up;
	return static_cast<Up&&>(t);
}

template <class Tp>
NX_INLINE constexpr Tp&& forward(typename _right_reference::_remove_reference<Tp>::type& t) {
	return static_cast<Tp&&>(t);
}

template <class Tp>
NX_INLINE constexpr Tp&& forward(typename _right_reference::_remove_reference<Tp>::type&& t) {
	typedef typename _right_reference::_is_lvalue_reference<Tp> _is_lvalue_reference;
	static_assert(!_is_lvalue_reference::value, "Can not forward an rvalue as an lvalue.");
	return static_cast<Tp&&>(t);
}

namespace sys {
	NX_EXPORT String name();
	NX_EXPORT String version();
	NX_EXPORT String brand();
	NX_EXPORT String subsystem();
	NX_EXPORT String info();
	NX_EXPORT String languages();
	NX_EXPORT String language();
	NX_EXPORT int64 time();
	NX_EXPORT int64 time_second();
	NX_EXPORT int64 time_monotonic();
};

NX_END

#endif
