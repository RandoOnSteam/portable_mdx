#ifndef __STDINTWRAPPER_H__
#define __STDINTWRAPPER_H__
#if !defined(__clang__) /* For Metrowerks and others, clang complains */
	#define __STDC_LIMIT_MACROS
	#define __STDC_CONSTANT_MACROS
#endif
#include <stddef.h>
#if !defined(_MSC_VER) || _MSC_VER >= 1600 /* 2010+ */
	#ifndef UINTMAX_MAX
		#include <stdint.h> /* for NULL, size_t, other_ts */
	#endif
	#if !defined(ULONG_MAX)
		#include <limits.h>
	#endif
#elif defined(_MSC_VER) && !defined(INT_MAX) /* VC6 */
	#include <limits.h>
#endif
#ifndef INT32_MAX
	#ifdef __alpha
		typedef unsigned int uint32_t;
		typedef int int32_t;
	#else
		typedef unsigned long uint32_t;
		#if !defined(__MWERKS__) || !defined(__MACH__)
			typedef long int32_t;
		#endif /*  MWERKS MACH defs int32_t but not uint32_t */
	#endif
	#if !defined(__MWERKS__) || !defined(__MACH__)
		typedef char int8_t;
	#endif /*  MWERKS MACH defs int8_t */
	typedef unsigned char uint8_t;
	typedef short int16_t;
	typedef unsigned short uint16_t;
	#define INT32_MAX (uint32_t)(~0)
#endif
#if (defined(_MSC_VER) && defined(_WIN32) && !defined(__clang__)) \
	|| (defined(__BCPLUSPLUS__) && __BCPLUSPLUS__ >= 0x520 && defined(_WIN32)) \
	|| (defined(__SC__) && defined(__DMC__)) \
	|| (defined(__WATCOMC__) && defined(_WIN32)) \
	|| (defined(__MWERKS__) && defined(NOCRUNTIME) && defined(_WIN32))
	typedef __int64 int64_t; typedef unsigned __int64 uint64_t;
	#if defined(_WIN64)
		#define WSNATIVE64
	#endif
#elif (defined(__MINGW32__) \
	||(defined(__clang__)) \
	||(defined(SIZEOF_LONG_LONG) && SIZEOF_LONG_LONG >= 8) \
	||(defined(__GNUC__)) \
	||(defined(__CYGWIN__)) \
	||(defined(__DJGPP__) && __DJGPP__ >= 2) \
	||(defined(__MWERKS__) && !defined(NOCRUNTIME) )) \
	||(defined(SIZEOF_LONG) && SIZEOF_LONG == 8)
	/* && __option(longlong))) for MWERKS? */
#	ifndef INT64_MAX
#		ifndef __MWERKS__
		typedef long long int64_t; typedef unsigned long long uint64_t;
#		define WSNATIVE64
#		endif
#	elif defined(__LP64__) && !defined(_INT64_T)
		typedef long int64_t; typedef unsigned long uint64_t;
#		define WSNATIVE64
#	elif defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) \
			|| defined(_M_AMD64) || defined(__ia64__) || defined(_IA64) \
			|| defined(__ppc64__) || defined(__powerpc64__) \
			|| defined(__aarch64__) || defined(__LP64__) || defined(_LP64)
#		define WSNATIVE64
#	endif
#else/*  !(defined(_M_X64) || defined(__LP64__)) */
#define WSNO64
#endif
#endif /* __STDINTWRAPPER_H__ */