#pragma once

#define _JNC_DEF_H

/// \addtogroup base-def
/// @{

//.............................................................................

// standard C/C++ headers

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#if (_WIN32)
#	define _CRT_SECURE_NO_WARNINGS  // useless warnings about "unsafe" string functions
#	define _SCL_SECURE_NO_WARNINGS  // useless warnings about "unsafe" iterator operations
#endif

#ifdef __cplusplus
#	include <new>
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

//.............................................................................

// Windows headers

#if (_WIN32)
#	ifndef _WIN32_WINNT             // Specifies that the minimum required platform is Windows Vista.
#		define _WIN32_WINNT 0x0600  // Change this to the appropriate value to target other versions of Windows.
#	endif
#
#	define WIN32_LEAN_AND_MEAN      // Exclude rarely-used stuff from Windows headers
#	include <windows.h>
#endif

//.............................................................................

// JNC_ASSERT

#if (_WIN32)
#	include <crtdbg.h>
#	define JNC_ASSERT _ASSERTE
#elif (__GNUC__)
#	include <assert.h>
#	define JNC_ASSERT assert
#endif 

//.............................................................................

// C++ compiler ids

#define JNC_CPP_MSC  1  // Microsoft Visual C++ compiler (cl.exe)
#define JNC_CPP_GCC  2  // GNU C++ compiler
#define JNC_CPP_ICC  3  // Intel C++ compiler

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#ifndef _JNC_CPP // detect C++ compiler
#	ifdef _MSC_VER
#		define _JNC_CPP JNC_CPP_MSC
#	elif (defined __GNUC__)
#		define _JNC_CPP JNC_CPP_GCC
#	else
#		error unsupported compiler
#	endif
#elif (_JNC_CPP != JNC_CPP_MSC && _JNC_CPP != JNC_CPP_GCC)
#	error _JNC_CPP is set to unknown C++ compiler id
#endif

//.............................................................................

// CPU arch ids

#define JNC_CPU_X86   1  // Intel i386
#define JNC_CPU_AMD64 2  // AMD64 / Intel x86_64

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#ifndef _JNC_CPU // detect CPU arch
#	if (_JNC_CPP == JNC_CPP_MSC)
#		if (defined _M_IX86)
#			define _JNC_CPU JNC_CPU_X86
#		elif (defined _M_AMD64)
#			define _JNC_CPU JNC_CPU_AMD64
#		endif
#	elif (_JNC_CPP == JNC_CPP_GCC)
#		if defined __i386__
#			define _JNC_CPU JNC_CPU_X86
#		elif defined __amd64__
#			define _JNC_CPU JNC_CPU_AMD64
#		endif
#	endif
#endif

#if (_JNC_CPU == JNC_CPU_X86)
#	define _JNC_CPU_STRING "x86"
#elif (_JNC_CPU == JNC_CPU_AMD64)
#	define _JNC_CPU_STRING "amd64"
#else
#	error _JNC_CPU is set to unknown CPU arch id
#endif

#if (_JNC_CPU == JNC_CPU_AMD64)
#	define _JNC_PTR_BITNESS 64
#	define _JNC_PTR_SIZE    8
#else
#	define _JNC_PTR_BITNESS 32
#	define _JNC_PTR_SIZE    4
#endif

//.............................................................................

// runtime environment ids

#define JNC_ENV_WIN   1  // win32 / win64 user mode module
#define JNC_ENV_NT    2  // NT native / kernel mode module
#define JNC_ENV_POSIX 3  // Unix / Linux / MacOSX

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#ifndef _JNC_ENV
#	if (_JNC_CPP == JNC_CPP_MSC)
#		define _JNC_ENV JNC_ENV_WIN // default runtime environment is Win32 / Win64
#	elif (_JNC_CPP == JNC_CPP_GCC)
#		define _JNC_ENV JNC_ENV_POSIX
#	endif
#endif

#if (_JNC_ENV < JNC_ENV_WIN || _JNC_ENV > JNC_ENV_POSIX)
#	error _JNC_ENV is set to unknown runtime environment id
#endif

//.............................................................................

// POSIX flavors

#define JNC_POSIX_LINUX   1
#define JNC_POSIX FREEBSD 2
#define JNC_POSIX_DARWIN  3

#if (_JNC_ENV != JNC_ENV_POSIX)
#	undef _JNC_POSIX
#else
#	if (!defined _JNC_POSIX)
#		ifdef __linux__
#			define _JNC_POSIX JNC_POSIX_LINUX
#		elif (defined __FreeBSD__)
#			define _JNC_POSIX JNC_POSIX_FREEBSD
#		elif (defined __APPLE__ && defined __MACH__)
#			define _JNC_POSIX JNC_POSIX_DARWIN
#		endif
#	endif
#
#	if (_JNC_POSIX < JNC_POSIX_LINUX || _JNC_POSIX > JNC_POSIX_DARWIN)
#		error _JNC_POSIX is set to unknown POSIX id
#	endif
#endif

//.............................................................................

#ifdef __cplusplus
#	define JNC_EXTERN_C extern "C"
#	define JNC_INLINE   inline
#else
#	define JNC_EXTERN_C
#	define JNC_INLINE   static __inline
#endif

#if (_JNC_CPP == JNC_CPP_MSC)
#	define JNC_CDECL      __cdecl
#	define JNC_STDCALL    __stdcall
#	define JNC_SELECT_ANY __declspec (selectany)
#	define JNC_NO_VTABLE  __declspec (novtable)
#	define JNC_EXPORT     __declspec (dllexport)
#
#	ifdef _DEBUG
#		define _JNC_DEBUG 1
#		define _JNC_DEBUG_STRING "Debug"
#		define _JNC_DEBUG_SUFFIX " Debug"
#	else
#		undef _JNC_DEBUG
#		define _JNC_DEBUG_STRING ""
#		define _JNC_DEBUG_SUFFIX ""
#	endif
#elif (_JNC_CPP == JNC_CPP_GCC)
#	if (_JNC_CPU == JNC_CPU_X86)
#		define JNC_CDECL   __attribute__ ((cdecl))
#		define JNC_STDCALL __attribute__ ((stdcall))
#	else
#		define JNC_CDECL
#		define JNC_STDCALL
#	endif
#	define JNC_SELECT_ANY  __attribute__ ((weak))
#	define JNC_NO_VTABLE
#	define JNC_EXPORT __attribute__ ((visibility ("default")))
#
#	ifdef NDEBUG
#		undef _DEBUG
#		undef _JNC_DEBUG
#		define _JNC_DEBUG_STRING ""
#		define _JNC_DEBUG_SUFFIX ""
#	else
#		define _DEBUG
#		define _JNC_DEBUG 1
#		define _JNC_DEBUG_STRING "Debug"
#		define _JNC_DEBUG_SUFFIX " Debug"
#	endif
#endif

//.............................................................................

// gcc-specific attributes

#if (_JNC_CPP == JNC_CPP_GCC)
#	define JNC_GCC_ALIGN(n) __attribute__((aligned (n)))
#	define JNC_GCC_MSC_STRUCT __attribute__((ms_struct))
#	define JNC_NO_ASAN __attribute__((no_sanitize_address))
#	if (defined (__has_feature))
#		if (__has_feature (address_sanitizer))
#	 		define _JNC_ASAN 1
#		endif
#	elif (defined (__SANITIZE_ADDRESS__))
# 		define _JNC_ASAN 1
#	else
#		undef _JNC_ASAN
#	endif
#else
#	define JNC_GCC_ALIGN(n)
#	define JNC_GCC_MSC_STRUCT
#	undef _JNC_ASAN
#	define JNC_NO_ASAN
#endif

//.............................................................................

// common type aliases

// stdint.h defines:
//
// int8_t
// uint8_t
// int16_t
// uint16_t
// int32_t
// uint32_t
// int64_t
// uint64_t
// intptr_t
// uintptr_t

typedef int               bool_t;
typedef unsigned int      uint_t;
typedef unsigned char     uchar_t;
typedef unsigned short    ushort_t;
typedef unsigned long     ulong_t;

typedef uint8_t           byte_t;
typedef uint16_t          word_t;
typedef uint64_t          qword_t;

#if (_JNC_CPP == JNC_CPP_MSC)
typedef ulong_t           dword_t;
#else
typedef uint32_t          dword_t;
#endif

#if (_JNC_PTR_BITNESS == 64)
#	if (_JNC_CPP == JNC_CPP_GCC)
typedef __int128          int128_t;
typedef unsigned __int128 uint128_t;
typedef int128_t          intdptr_t;
typedef uint128_t         uintdptr_t;
#	endif
#else
typedef int64_t           intdptr_t;
typedef uint64_t          uintdptr_t;
#endif

typedef void*             handle_t;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef char              utf8_t;

#if (WCHAR_MAX <= 0xffff)
typedef wchar_t           utf16_t;
typedef int32_t           utf32_t;
#else
typedef int16_t           utf16_t;
typedef wchar_t           utf32_t;
#endif

//.............................................................................

// when compiling core libraries, we want to use actual implementation classes
// as to avoid unncecessary casts; otherwise, use opaque struct pointers

#ifdef _JNC_CORE

namespace jnc {
namespace ct {

class ModuleItemDecl;
class ModuleItem;
class Attribute;
class AttributeBlock;
class Namespace;
class GlobalNamespace;
class Alias;
class Variable;
class Function;
class Property;
class Typedef;
class Type;
class NamedType;
class BaseTypeSlot;
class DerivableType;
class ArrayType;
class BitFieldType;
class FunctionArg;
class FunctionType;
class PropertyType;
class EnumConst;
class EnumType;
class StructField;
class StructType;
class UnionType;
class ClassType;
class MulticastClassType;
class McSnapshotClassType;
class DataPtrType;
class ClassPtrType;
class FunctionPtrType;
class PropertyPtrType;
class Unit;
class Module;
class GcShadowStackFrameMap;

} // namespace ct

namespace rt {

class Runtime;
class GcHeap;

} // namespace rt
} // namespace jnc

typedef axl::sl::ListLink jnc_ListLink;
typedef axl::sl::Guid jnc_Guid;
typedef axl::err::ErrorHdr jnc_Error;
typedef jnc::ct::ModuleItemDecl jnc_ModuleItemDecl;
typedef jnc::ct::ModuleItem jnc_ModuleItem;
typedef jnc::ct::Attribute jnc_Attribute;
typedef jnc::ct::AttributeBlock jnc_AttributeBlock;
typedef jnc::ct::Namespace jnc_Namespace;
typedef jnc::ct::GlobalNamespace jnc_GlobalNamespace;
typedef jnc::ct::Alias jnc_Alias;
typedef jnc::ct::Variable jnc_Variable;
typedef jnc::ct::Function jnc_Function;
typedef jnc::ct::Property jnc_Property;
typedef jnc::ct::Typedef jnc_Typedef;
typedef jnc::ct::Type jnc_Type;
typedef jnc::ct::NamedType jnc_NamedType;
typedef jnc::ct::BaseTypeSlot jnc_BaseTypeSlot;
typedef jnc::ct::DerivableType jnc_DerivableType;
typedef jnc::ct::ArrayType jnc_ArrayType;
typedef jnc::ct::BitFieldType jnc_BitFieldType;
typedef jnc::ct::FunctionArg jnc_FunctionArg;
typedef jnc::ct::FunctionType jnc_FunctionType;
typedef jnc::ct::PropertyType jnc_PropertyType;
typedef jnc::ct::EnumConst jnc_EnumConst;
typedef jnc::ct::EnumType jnc_EnumType;
typedef jnc::ct::StructField jnc_StructField;
typedef jnc::ct::StructType jnc_StructType;
typedef jnc::ct::UnionType jnc_UnionType;
typedef jnc::ct::ClassType jnc_ClassType;
typedef jnc::ct::MulticastClassType jnc_MulticastClassType;
typedef jnc::ct::McSnapshotClassType jnc_McSnapshotClassType;
typedef jnc::ct::DataPtrType jnc_DataPtrType;
typedef jnc::ct::ClassPtrType jnc_ClassPtrType;
typedef jnc::ct::FunctionPtrType jnc_FunctionPtrType;
typedef jnc::ct::PropertyPtrType jnc_PropertyPtrType;
typedef jnc::ct::Unit jnc_Unit;
typedef jnc::ct::Module jnc_Module;
typedef jnc::rt::Runtime jnc_Runtime;
typedef jnc::rt::GcHeap jnc_GcHeap;
typedef jnc::ct::GcShadowStackFrameMap jnc_GcShadowStackFrameMap;

#	define JNC_GUID_INITIALIZER AXL_SL_GUID_INITIALIZER
#	define JNC_DEFINE_GUID AXL_SL_DEFINE_GUID
#	define jnc_g_nullGuid axl::sl::g_nullGuid

namespace jnc {

axl::sl::String*
getTlsStringBuffer ();

} // namespace jnc

#else // _JNC_CORE

typedef struct jnc_Error jnc_Error;
typedef struct jnc_ModuleItemDecl jnc_ModuleItemDecl;
typedef struct jnc_ModuleItem jnc_ModuleItem;
typedef struct jnc_Attribute jnc_Attribute;
typedef struct jnc_AttributeBlock jnc_AttributeBlock;
typedef struct jnc_Namespace jnc_Namespace;
typedef struct jnc_GlobalNamespace jnc_GlobalNamespace;
typedef struct jnc_Alias jnc_Alias;
typedef struct jnc_Variable jnc_Variable;
typedef struct jnc_Function jnc_Function;
typedef struct jnc_Property jnc_Property;
typedef struct jnc_Typedef jnc_Typedef;
typedef struct jnc_Type jnc_Type;
typedef struct jnc_NamedType jnc_NamedType;
typedef struct jnc_BaseTypeSlot jnc_BaseTypeSlot;
typedef struct jnc_DerivableType jnc_DerivableType;
typedef struct jnc_ArrayType jnc_ArrayType;
typedef struct jnc_BitFieldType jnc_BitFieldType;
typedef struct jnc_FunctionArg jnc_FunctionArg;
typedef struct jnc_FunctionType jnc_FunctionType;
typedef struct jnc_PropertyType jnc_PropertyType;
typedef struct jnc_EnumConst jnc_EnumConst;
typedef struct jnc_EnumType jnc_EnumType;
typedef struct jnc_StructField jnc_StructField;
typedef struct jnc_StructType jnc_StructType;
typedef struct jnc_UnionType jnc_UnionType;
typedef struct jnc_ClassType jnc_ClassType;
typedef struct jnc_MulticastClassType jnc_MulticastClassType;
typedef struct jnc_McSnapshotClassType jnc_McSnapshotClassType;
typedef struct jnc_DataPtrType jnc_DataPtrType;
typedef struct jnc_ClassPtrType jnc_ClassPtrType;
typedef struct jnc_FunctionPtrType jnc_FunctionPtrType;
typedef struct jnc_PropertyPtrType jnc_PropertyPtrType;
typedef struct jnc_Unit jnc_Unit;
typedef struct jnc_Module jnc_Module;
typedef struct jnc_Runtime jnc_Runtime;
typedef struct jnc_GcHeap jnc_GcHeap;
typedef struct jnc_GcShadowStackFrameMap jnc_GcShadowStackFrameMap;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#	ifdef _AXL_SL_LISTBASE_H

typedef axl::sl::ListLink jnc_ListLink;

#	else // _AXL_SL_LISTBASE_H

typedef struct jnc_ListLink jnc_ListLink;

struct jnc_ListLink
{
	jnc_ListLink* m_next;
	jnc_ListLink* m_prev;
};

#	endif // _AXL_SL_LISTBASE_H

#	ifdef _AXL_SL_GUID_H

typedef axl::sl::Guid jnc_Guid;

#		define JNC_GUID_INITIALIZER AXL_SL_GUID_INITIALIZER
#		define JNC_DEFINE_GUID AXL_SL_DEFINE_GUID

#	else // _AXL_SL_GUID_H

typedef struct jnc_Guid jnc_Guid;

struct jnc_Guid
{
	union
	{
		struct
		{
			uint32_t m_data1;
			uint16_t m_data2;
			uint16_t m_data3;
			uint8_t m_data4 [8];
		};

		struct
		{
			uint32_t m_long1;
			uint32_t m_long2;
			uint32_t m_long3;
			uint32_t m_long4;
		};
	};
};

#		ifdef __cplusplus
#			define JNC_GUID_SPECIFIER extern JNC_SELECT_ANY const
#		else
#			define JNC_GUID_SPECIFIER JNC_SELECT_ANY const
#		endif

#		define JNC_GUID_INITIALIZER(l, s1, s2, b1, b2, b3, b4, b5, b6, b7, b8) \
			{ { { l, s1, s2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } } } }

#		define JNC_DEFINE_GUID(n, l, s1, s2, b1, b2, b3, b4, b5, b6, b7, b8) \
			JNC_GUID_SPECIFIER jnc_Guid n = \
			JNC_GUID_INITIALIZER (l, s1, s2, b1, b2,  b3,  b4,  b5,  b6,  b7,  b8)
#	endif // _AXL_SL_GUID_H

#endif // _JNC_CORE

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef struct jnc_ExtensionLib jnc_ExtensionLib;
typedef struct jnc_GcStats jnc_GcStats;
typedef struct jnc_GcSizeTriggers jnc_GcSizeTriggers;

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

typedef jnc_ListLink ListLink;
typedef jnc_Guid Guid;
typedef jnc_Error Error;
typedef jnc_ModuleItemDecl ModuleItemDecl;
typedef jnc_ModuleItem ModuleItem;
typedef jnc_Attribute Attribute;
typedef jnc_AttributeBlock AttributeBlock;
typedef jnc_Namespace Namespace;
typedef jnc_GlobalNamespace GlobalNamespace;
typedef jnc_Alias Alias;
typedef jnc_Variable Variable;
typedef jnc_Function Function;
typedef jnc_Property Property;
typedef jnc_Typedef Typedef;
typedef jnc_Type Type;
typedef jnc_NamedType NamedType;
typedef jnc_BaseTypeSlot BaseTypeSlot;
typedef jnc_DerivableType DerivableType;
typedef jnc_ArrayType ArrayType;
typedef jnc_BitFieldType BitFieldType;
typedef jnc_FunctionArg FunctionArg;
typedef jnc_FunctionType FunctionType;
typedef jnc_PropertyType PropertyType;
typedef jnc_EnumConst EnumConst;
typedef jnc_EnumType EnumType;
typedef jnc_StructField StructField;
typedef jnc_StructType StructType;
typedef jnc_UnionType UnionType;
typedef jnc_ClassType ClassType;
typedef jnc_MulticastClassType MulticastClassType;
typedef jnc_McSnapshotClassType McSnapshotClassType;
typedef jnc_DataPtrType DataPtrType;
typedef jnc_ClassPtrType ClassPtrType;
typedef jnc_FunctionPtrType FunctionPtrType;
typedef jnc_PropertyPtrType PropertyPtrType;
typedef jnc_Unit Unit;
typedef jnc_Module Module;
typedef jnc_Runtime Runtime;
typedef jnc_GcHeap GcHeap;
typedef jnc_GcShadowStackFrameMap GcShadowStackFrameMap;
typedef jnc_ExtensionLib ExtensionLib;
typedef jnc_GcStats GcStats;
typedef jnc_GcSizeTriggers GcSizeTriggers;

//.............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
