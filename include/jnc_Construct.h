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

#define _JNC_CONSTRUCT_H

#include "jnc_Def.h"

#ifdef __cplusplus

namespace jnc {

//..............................................................................

template <typename T>
void
construct(T* p) {
	new(p)T;
}

template <
	typename T,
	typename Arg
>
void
construct(
	T* p,
	Arg arg
) {
	new(p) T(arg);
}

template <
	typename T,
	typename Arg1,
	typename Arg2
>
void
construct(
	T* p,
	Arg1 arg1,
	Arg2 arg2
) {
	new(p) T(arg1, arg2);
}

template <
	typename T,
	typename Arg1,
	typename Arg2,
	typename Arg3
>
void
construct(
	T* p,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
) {
	new(p) T(arg1, arg2, arg3);
}

template <
	typename T,
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4
>
void
construct(
	T* p,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
) {
	new(p) T(arg1, arg2, arg3, arg4);
}

template <
	typename T,
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4,
	typename Arg5
>
void
construct(
	T* p,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4,
	Arg5 arg5
) {
	new(p) T(arg1, arg2, arg3, arg4, arg5);
}

template <typename T>
void
destruct(T* p) {
	p->~T();
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus
