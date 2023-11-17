//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#pragma once

#define _JNC_PCH_H

/// \addtogroup base-def
/// @{

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

// 1) detect build/target environment section

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// detect C++ compiler

#ifdef _MSC_VER
#	define _JNC_CPP_MSC 1
#elif (defined __GNUC__)
#	define _JNC_CPP_GCC 1
#	ifdef __clang__
#		define _JNC_CPP_CLANG 1
#	elif (defined __ICC)
#		define _JNC_CPP_ICC 1
#	endif
#else
#	error unsupported C++ compiler
#endif

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// detect CPU architecture

#if (_JNC_CPP_MSC)
#	if (defined _M_IX86)
#		define _JNC_CPU_IA32  1
#		define _JNC_CPU_X86   1
#	elif (defined _M_AMD64)
#		define _JNC_CPU_AMD64 1
#	elif (defined _M_ARM)
#		define _JNC_CPU_ARM32 1
#	elif (defined _M_ARM64)
#		define _JNC_CPU_ARM64 1
#	endif
#elif (_JNC_CPP_GCC)
#	if defined __i386__
#		define _JNC_CPU_IA32  1
#		define _JNC_CPU_X86   1
#	elif (defined __amd64__)
#		define _JNC_CPU_AMD64 1
#	elif (defined __arm__)
#		define _JNC_CPU_ARM32 1
#	elif (defined __aarch64__)
#		define _JNC_CPU_ARM64 1
#	endif
#endif

#if (_JNC_CPU_X86)
#	define JNC_CPU_STRING "x86"
#	define JNC_PTR_SIZE 4
#elif (_JNC_CPU_AMD64)
#	define JNC_CPU_STRING "amd64"
#	define JNC_PTR_SIZE 8
#elif (_JNC_CPU_ARM32)
#	define JNC_CPU_STRING "arm32"
#	define JNC_PTR_SIZE 4
#elif (_JNC_CPU_ARM64)
#	define JNC_CPU_STRING "arm64"
#	define JNC_PTR_SIZE 8
#else
#	error unsupported CPU architecture
#endif

#define JNC_PTR_BITS (JNC_PTR_SIZE * 8) // often times it's more natural to use bit size

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// detect OS

#ifdef _WIN32
#	define _JNC_OS_WIN 1
#elif (defined __unix__)
#	define _JNC_OS_POSIX 1
#	ifdef __linux__
#		define _JNC_OS_LINUX 1
#	elif (defined __sun__)
#		define _JNC_OS_SOLARIS 1
#	elif (defined __FreeBSD__ || defined __OpenBSD__ || defined __NetBSD__)
#		define _JNC_OS_BSD 1
#	endif
#elif (defined __APPLE__ && defined __MACH__)
#	define _JNC_OS_POSIX  1
#	define _JNC_OS_BSD    1
#	define _JNC_OS_DARWIN 1
#else
#	error unsupported OS
#endif

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// detect Debug build

#if (_JNC_CPP_MSC)
#	ifdef _DEBUG
#		define _JNC_DEBUG 1
#	endif
#else
#	ifndef NDEBUG
#		define _JNC_DEBUG 1
#	endif
#endif

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

// 2) standard includes

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// C/C++ headers

#ifndef __STDC_LIMIT_MACROS
#	define __STDC_LIMIT_MACROS 1
#endif

#ifndef __STDC_CONSTANT_MACROS
#	define __STDC_CONSTANT_MACROS 1
#endif

#if (_JNC_OS_WIN)
#	ifndef _CRT_SECURE_NO_WARNINGS
#		define _CRT_SECURE_NO_WARNINGS 1 // useless warnings about "unsafe" string functions
#	endif

#	ifndef _CRT_SECURE_NO_WARNINGS
#		define _SCL_SECURE_NO_WARNINGS 1 // useless warnings about "unsafe" iterator operations
#	endif
#endif

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
#include <errno.h>

#ifdef __cplusplus
#	include <new>
#endif

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// OS-specific headers

#if (_JNC_OS_WIN)
#	ifndef _WIN32_WINNT
#		define _WIN32_WINNT 0x0600 // Windows Vista
#	endif

#	define WIN32_LEAN_AND_MEAN // prevent winsock.h vs winsock2.h conflict
#	define NOMINMAX            // prevent min/max macro conflicts

#	include <windows.h>
#	include <crtdbg.h>
#elif (_JNC_OS_POSIX)
#	include <assert.h>
#endif

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

// 3) possibly conflicting complementary definitions

// items here sort of complement standard C types/operators/functions
// so prefixing them with 'axl_' would look non-consistent

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

/**

\subgroup

	Common integer type aliases in addition to the ones defined in ``<stdint.h>``, i.e. ``int8_t``, ``uint8_t``, ``int16_t``, ``uint16_t``, ``int32_t``, ``uint32_t``, ``int64_t``, ``uint64_t``, ``intptr_t``, ``uintptr_t``.

	``bool_t`` aliases to ``int`` and is used to denote boolean values.

	``uchar_t`` and ``byte_t`` alias to unsigned integer type with width of 8 bits.

	``ushort_t`` and ``word_t`` alias to unsigned integer type with width of 16 bits.

	``uint_t``, ``ulong_t`` and ``dword_t`` alias to unsigned integer type with width of 32 bits.

	``qword_t`` aliases to unsigned integer type with width of 64 bits.

*/

typedef int               bool_t;
typedef unsigned int      uint_t;
typedef unsigned char     uchar_t;
typedef unsigned short    ushort_t;
typedef unsigned long     ulong_t;

typedef uint8_t           byte_t;
typedef uint16_t          word_t;

#if (_JNC_CPP_MSC)
typedef ulong_t           dword_t;
#else
typedef uint32_t          dword_t;
#endif

typedef uint64_t          qword_t;

/// \cond EXTRA_TYPEDEFS

typedef void*             handle_t;
typedef char              utf8_t;

#if (WCHAR_MAX <= 0xffff)
typedef wchar_t           utf16_t;
typedef int32_t           utf32_t;
#else
typedef int16_t           utf16_t;
typedef wchar_t           utf32_t;
#endif

/// \endcond

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

// 4) non-confliciting macro definitions

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// declaration options/attributes

#if (_JNC_CPP_MSC)
#	define JNC_CDECL       __cdecl
#	define JNC_STDCALL     __stdcall
#	define JNC_SELECT_ANY  __declspec(selectany)
#	define JNC_EXPORT      __declspec(dllexport)

#	define JNC_GCC_ALIGN(n)
#	define JNC_GCC_MSC_STRUCT
#	define JNC_GCC_NO_ASAN
#elif (_JNC_CPP_GCC)
#	if (_JNC_CPU_X86)
#		define JNC_CDECL   __attribute__((cdecl))
#		define JNC_STDCALL __attribute__((stdcall))
#	else
#		define JNC_CDECL
#		define JNC_STDCALL
#	endif
#	define JNC_SELECT_ANY  __attribute__((weak))
#	define JNC_EXPORT      __attribute__((visibility("default")))

#	define JNC_GCC_ALIGN(n) __attribute__((aligned(n)))

#	ifdef __has_attribute
#		if (__has_attribute(ms_struct))
#			define JNC_GCC_MSC_STRUCT __attribute__((ms_struct))
#   	else
#			define JNC_GCC_MSC_STRUCT
#		endif
#   else
#		define JNC_GCC_MSC_STRUCT
#   endif

#	ifdef __has_feature
#		if (__has_feature(address_sanitizer))
#	 		define _JNC_GCC_ASAN 1
#		endif
#	elif (defined(__SANITIZE_ADDRESS__))
# 		define _JNC_GCC_ASAN 1
#	endif

#	define JNC_GCC_NO_ASAN __attribute__((no_sanitize_address))
#endif

#ifdef __cplusplus
#	define JNC_EXTERN_C extern "C"
#	define JNC_INLINE   inline
#else
#	define JNC_EXTERN_C
#	define JNC_INLINE   static __inline
#endif

#if (_JNC_OBJECT_LIB && _JNC_CPP_GCC)
#	define JNC_EXPORT_O JNC_EXPORT
#else
#	define JNC_EXPORT_O
#endif

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

/**
	\verbatim

	On ``Debug`` builds, this macro causes an **assertion check**: it calculates the value of its argument, and if it is ``false``, then a corresponding message is displayed and a program is terminated.

	The exact way of displaying an assertion message and its format is platform dependent, but it always includes the location of failing ``JNC_ASSERT`` macro and the actual expression which caused it to fail.

	.. rubric:: Sample:

	.. code-block:: none

		test_cpp: /home/user/test_cpp/main.cpp:100: int main(int, char**): Assertion `line < lineCount' failed.

	On ``Release`` builds, this macro does nothing(expands to an empty sequence).

	\endverbatim
*/

#if (_JNC_OS_WIN)
#	define JNC_ASSERT _ASSERTE // from crtdbg.h
#else
#	define JNC_ASSERT assert   // from assert.h
#endif

//..............................................................................

/// @}

#include "jnc_WarningSuppression.h"
#include "jnc_Config.h"

#ifdef _AXL_G_PCH_H
AXL_SELECT_ANY struct ToAxl* toAxl; // for convenient conversion operators
#endif
