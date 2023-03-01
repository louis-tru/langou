/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __quark__util__macros__
#define __quark__util__macros__

//  ----------------------------- Compiling environment -----------------------------

#define Qk_HAS_CPP __cplusplus

#ifdef __EXCEPTIONS
# define Qk_EXCEPTIONS_SUPPORT 1
#else
# define Qk_EXCEPTIONS_SUPPORT 0
#endif

#ifndef NULL
# define NULL 0
#endif

#define Qk_GNUC          0
#define Qk_CLANG         0
#define Qk_MSC           0
#define Qk_ARCH_X86      0
#define Qk_ARCH_ARM      0
#define Qk_ARCH_MIPS     0
#define Qk_ARCH_MIPS64   0
#define Qk_ARCH_IA32     0
#define Qk_ARCH_X64      0
#define Qk_ARCH_ARM64    0
#define Qk_ARCH_ARMV7    0
#define Qk_ARCH_32BIT    0
#define Qk_ARCH_64BIT    0
#define Qk_APPLE         0
#define Qk_POSIX         0
#define Qk_UNIX          0
#define Qk_LINUX         0
#define Qk_BSD           0
#define Qk_CYGWIN        0
#define Qk_NACL          0
#define Qk_iOS           0
#define Qk_OSX           0
#define Qk_ANDROID       0
#define Qk_WIN           0
#define Qk_QNX           0

#if defined(__GNUC__)
# undef Qk_GNUC
# define Qk_GNUC       1
#endif

#if defined(__clang__)
# undef Qk_CLANG
# define Qk_CLANG       1
#endif

#if defined(_MSC_VER)
# undef Qk_MSC
# define Qk_MSC       1
#endif

#if defined(_M_X64) || defined(__x86_64__)
# undef Qk_ARCH_X86
# define Qk_ARCH_X86        1
# if defined(__native_client__)
#   undef Qk_ARCH_IA32
#   define Qk_ARCH_IA32     1
#   undef Qk_ARCH_32BIT
#   define Qk_ARCH_32BIT    1
# else // else __native_client__
#   undef Qk_ARCH_X64
#   define Qk_ARCH_X64      1
#   undef Qk_ARCH_64BIT
#   define Qk_ARCH_64BIT    1
# endif  // __native_client__

#elif defined(_M_IX86) || defined(__i386__)
# undef Qk_ARCH_X86
# define Qk_ARCH_X86        1
# undef Qk_ARCH_IA32
# define Qk_ARCH_IA32       1
# undef Qk_ARCH_32BIT
# define Qk_ARCH_32BIT      1

#elif defined(__arm64__) || defined(__AARCH64EL__)
# undef Qk_ARCH_ARM
# define Qk_ARCH_ARM        1
# undef Qk_ARCH_ARM64
# define Qk_ARCH_ARM64      1
# undef Qk_ARCH_64BIT
# define Qk_ARCH_64BIT      1

#elif defined(__ARMEL__)
# undef Qk_ARCH_ARM
# define Qk_ARCH_ARM        1
# undef Qk_ARCH_32BIT
# define Qk_ARCH_32BIT      1

#elif defined(__mips64)
# undef Qk_ARCH_MIPS
# define Qk_ARCH_MIPS       1
# undef Qk_ARCH_MIPS64
# define Qk_ARCH_MIPS64     1
# undef Qk_ARCH_64BIT
# define Qk_ARCH_64BIT      1

#elif defined(__MIPSEL__)
# undef Qk_ARCH_MIPS
# define Qk_ARCH_MIPS       1
# undef Qk_ARCH_32BIT
# define Qk_ARCH_32BIT      1

#else
# error Host architecture was not detected as supported by quark
#endif

#if defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7__)
# undef Qk_ARCH_ARMV7
# define Qk_ARCH_ARMV7  1
#endif

#if defined(__sun)
# undef Qk_BSD
# define Qk_BSD        1
# undef Qk_UNIX
# define Qk_UNIX       1
#endif

#if defined(__OpenBSD__) || defined(__NetBSD__) || \
		defined(__FreeBSD__) || defined(__DragonFly__)
# undef Qk_POSIX
# define Qk_POSIX      1
# undef Qk_BSD
# define Qk_BSD        1
# undef Qk_UNIX
# define Qk_UNIX       1
#endif

#if defined(__APPLE__)
# include <TargetConditionals.h>
# undef Qk_APPLE
# define Qk_APPLE      1
# undef Qk_POSIX
# define Qk_POSIX      1
# undef Qk_BSD
# define Qk_BSD        1
# undef Qk_UNIX
# define Qk_UNIX       1
#endif

#if defined(__CYGWIN__)
# undef Qk_CYGWIN
# define Qk_CYGWIN     1
# undef Qk_POSIX
# define Qk_POSIX      1
#endif

#if defined(__unix__)
# undef Qk_UNIX
# define Qk_UNIX       1
#endif

#if defined(__linux__)
# undef Qk_LINUX
# define Qk_LINUX      1
# undef Qk_POSIX
# define Qk_POSIX      1
# undef Qk_UNIX
# define Qk_UNIX       1
#endif

#if defined(__native_client__)
# undef Qk_NACL
# define Qk_NACL       1
# undef Qk_POSIX
# define Qk_POSIX      1
#endif

#if TARGET_OS_IPHONE
# undef Qk_iOS
# define Qk_iOS        1
#endif

#if TARGET_OS_MAC && !TARGET_OS_IPHONE
# undef Qk_OSX
# define Qk_OSX        1
#endif

#if defined(__ANDROID__)
# undef Qk_ANDROID
# define Qk_ANDROID    1
# undef Qk_POSIX
# define Qk_POSIX      1
# undef Qk_LINUX
# define Qk_LINUX      1
# undef Qk_UNIX
# define Qk_UNIX       1
#endif

#if defined(_WINDOWS) || defined(_MSC_VER)
# undef Qk_WIN
# define Qk_WIN        1
#endif

#if defined(__QNXNTO__)
# undef Qk_POSIX
# define Qk_POSIX 1
# undef Qk_QNX
# define Qk_QNX 1
#endif

// ----------------------------- Compiling environment end -----------------------------

#ifndef Qk_MORE_LOG
# define Qk_MORE_LOG 0
#endif

#if Qk_MORE_LOG
# undef  Qk_MEMORY_TRACE_MARK
# define Qk_MEMORY_TRACE_MARK 1
#endif

#ifndef Qk_STRICT_ASSERT
# define  Qk_STRICT_ASSERT 0
#endif

#if DEBUG || Qk_STRICT_ASSERT
# define Qk_ASSERT(cond, ...) if(!(cond)) qk::fatal(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
# define Qk_ASSERT(cond, ...) ((void)0)
#endif

#define Qk_DEFINE_INLINE_CLASS(Inl) public: class Inl; friend class Inl; private:
#define Qk_DEFINE_INLINE_MEMBERS(cls, Inl) \
	static Qk_INLINE cls::Inl* Inl##_##cls(cls* self) { \
		return reinterpret_cast<cls::Inl*>(self); \
	} class cls::Inl: public cls

#define Qk_LOG(msg, ...)      qk::console::log(msg, ##__VA_ARGS__)
#define Qk_WARN(msg, ...)     qk::console::warn(msg, ##__VA_ARGS__)
#define Qk_ERR(msg, ...)      qk::console::error(msg, ##__VA_ARGS__)
#define Qk_FATAL(...)         qk::fatal(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define Qk_UNIMPLEMENTED(...)    Qk_FATAL("Unimplemented code, %s", ##__VA_ARGS__)
#define Qk_UNREACHABLE(...)      Qk_FATAL("Unreachable code, %s", ##__VA_ARGS__)
#define Qk_MIN(A, B)          ((A) < (B) ? (A) : (B))
#define Qk_MAX(A, B)          ((A) > (B) ? (A) : (B))

#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG || Qk_MORE_LOG
# define Qk_DEBUG Qk_LOG
#else
# define Qk_DEBUG(msg, ...) ((void)0)
#endif

// ------------------------------------------------------------------

// This macro allows to test for the version of the GNU C++ compiler.
// Note that this also applies to compilers that masquerade as GCC,
// for example clang and the Intel C++ compiler for Linux.
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
# define Qk_GNUC_PREREQ(major, minor, patchlevel) \
	((__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) >= \
	((major) * 10000 + (minor) * 100 + (patchlevel)))
#elif defined(__GNUC__) && defined(__GNUC_MINOR__)
# define Qk_GNUC_PREREQ(major, minor, patchlevel) \
	((__GNUC__ * 10000 + __GNUC_MINOR__ * 100) >= \
	((major) * 10000 + (minor) * 100 + (patchlevel)))
#else
# define Qk_GNUC_PREREQ(major, minor, patchlevel) 0
#endif

#if Qk_CLANG
# define Qk_HAS_ATTRIBUTE_ALWAYS_INLINE (__has_attribute(always_inline))
# define Qk_HAS_ATTRIBUTE_VISIBILITY (__has_attribute(visibility))
#elif Qk_GNUC
// always_inline is available in gcc 4.0 but not very reliable until 4.4.
// Works around "sorry, unimplemented: inlining failed" build errors with
// older compilers.
# define Qk_HAS_ATTRIBUTE_ALWAYS_INLINE (N_GNUC_PREREQ(4, 4, 0))
# define Qk_HAS_ATTRIBUTE_VISIBILITY (N_GNUC_PREREQ(4, 3, 0))
#elif Qk_MSC
# define Qk_HAS_FORCE_INLINE 1
#endif

#if !defined(Qk_CPU_BENDIAN) && !defined(Qk_CPU_LENDIAN)
	#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
		#define Qk_CPU_BENDIAN 1
	#elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
		#define Qk_CPU_LENDIAN 1
	#elif defined(__sparc) || defined(__sparc__) || \
		defined(_POWER) || defined(__powerpc__) || \
		defined(__ppc__) || defined(__hppa) || \
		defined(__PPC__) || defined(__PPC64__) || \
		defined(_MIPSEB) || defined(__ARMEB__) || \
		defined(__s390__) || \
		(defined(__sh__) && defined(__BIG_ENDIAN__)) || \
		(defined(__ia64) && defined(__BIG_ENDIAN__))
			#define Qk_CPU_BENDIAN 1
	#else
		#define Qk_CPU_LENDIAN 1
	#endif
#endif

// ------------------------------------------------------------------

#if Qk_MSC
	#ifdef Qk_BUILDING_SHARED
	# define Qk_EXPORT __declspec(dllexport)
	#elif Qk_USING_SHARED
	# define Qk_EXPORT __declspec(dllimport)
	#else
	# define Qk_EXPORT
	#endif  // Qk_BUILDING_SHARED
#else  // Qk_MSC
	// Setup for Linux shared library export.
	#if Qk_HAS_ATTRIBUTE_VISIBILITY
	# ifdef Qk_BUILDING_SHARED
	#  define Qk_EXPORT __attribute__((visibility("default")))
	# else
	#  define Qk_EXPORT
	# endif
	#else
	# define Qk_EXPORT
	#endif
#endif // Qk_MSC


#if Qk_MSC
	#pragma section(".CRT$XCU", read)
	# define Qk_INIT_BLOCK(fn)	\
	extern void __cdecl fn(void);	\
	__declspec(dllexport, allocate(".CRT$XCU"))	\
	void (__cdecl*fn##_)(void) = fn;	\
	extern void __cdecl fn(void)
#else // Qk_MSC
	# define Qk_INIT_BLOCK(fn)	\
	extern __attribute__((constructor)) void __##fn(void)
#endif

// A macro used to make better inlining. Don't bother for debug builds.
//
#if Qk_HAS_ATTRIBUTE_ALWAYS_INLINE && !DEBUG
# define Qk_INLINE inline __attribute__((always_inline))
#elif Qk_HAS_FORCE_INLINE && !DEBUG
# define Qk_INLINE __forceinline
#else
# define Qk_INLINE inline
#endif
#undef Qk_HAS_ATTRIBUTE_ALWAYS_INLINE
#undef Qk_HAS_FORCE_INLINE

#define Qk_HIDDEN_COPY_CONSTRUCTOR(T)  private: T(const T& t) = delete;
#define Qk_HIDDEN_ASSIGN_OPERATOR(T)   private: T& operator=(const T& t) = delete;
#define Qk_HIDDEN_ALL_COPY(T)	\
	private: T(const T& t) = delete;	\
	private: T& operator=(const T& t) = delete;
#define Qk_HIDDEN_HEAP_ALLOC() \
	private: static void* operator new(size_t size) = delete; \
	private: static void operator delete(void*, size_t) = delete

#define Qk_DEFINE_PROP_MODIFIER const
#define Qk_DEFINE_PROP_MODIFIERNoConst

#define Qk_DEFINE_PROP_ACC_GET(type, name, ...) \
	type name () Qk_DEFINE_PROP_MODIFIER##__VA_ARGS__; public:

#define Qk_DEFINE_PROP_ACC(type, name, ...) \
	void set_##name (type val); \
	Qk_DEFINE_PROP_ACC_GET(type, name, ##__VA_ARGS__) \

#define Qk_DEFINE_PROP_GET(type, name, ...) \
	inline type name () Qk_DEFINE_PROP_MODIFIER##__VA_ARGS__ { return _##name; } \
	private: type _##name; public:\

#define Qk_DEFINE_PROP(type, name, ...) \
	void set_##name (type val); \
	Qk_DEFINE_PROP_GET(type, name, ##__VA_ARGS__) \

#define Qk_DEFINE_CLASS(Name) class Name;
#define Qk_DEFINE_VISITOR_VISIT(N) virtual void visit##N(N *v) = 0;
#define Qk_DEFINE_VISITOR(Name, Each) \
	Each(Qk_DEFINE_CLASS); \
	class Name##Visitor { \
	public: \
		Each(Qk_DEFINE_VISITOR_VISIT); \
}

// Helper macros end
// -----------------------------------------------------------------------------

#endif
