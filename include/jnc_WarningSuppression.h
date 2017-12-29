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

// no pragma once -- this is to allow multiple inclusions of this file for re-enabling
// warning suppression (GCC loses warning suppression set up from precompiled headers)

#if (_JNC_CPP_MSC)
#	pragma warning (disable: 4146) // warning C4146: unary minus operator applied to unsigned type, result still unsigned
#	pragma warning (disable: 4267) // warning C4267: 'var' : conversion from 'size_t' to 'type', possible loss of data
#	pragma warning (disable: 4355) // warning C4355: 'this' : used in base member initializer list
#endif

#if (_JNC_CPP_GCC)
#	pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#	ifdef __cplusplus
#		pragma GCC diagnostic ignored "-Winvalid-offsetof"
#	endif
#	pragma GCC diagnostic ignored "-Wmultichar"
#	pragma GCC diagnostic ignored "-Wformat"
#	if (__cplusplus >= 201103L)
#		pragma GCC diagnostic ignored "-Wnarrowing"
#	endif
#endif

#if (_JNC_CPP_CLANG)
#	pragma GCC diagnostic ignored "-Wdangling-else"
#	pragma GCC diagnostic ignored "-Wincompatible-ms-struct"
#	pragma GCC diagnostic ignored "-Wlogical-op-parentheses"
#	pragma GCC diagnostic ignored "-Wswitch"
#endif

//..............................................................................
