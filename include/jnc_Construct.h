// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2015. All rights reserved
// Author: Vladimir Gladkov

#pragma once 

#define _JNC_CONSTRUCT_H

#include "jnc_Def.h"

#ifdef __cplusplus

namespace jnc {

//.............................................................................

template <typename T>
void 
construct (T* p)
{
	new (p) T;
}

template <
	typename T,
	typename Arg
	>
void 
construct (
	T* p,
	Arg arg
	)
{
	new (p) T (arg);
}

template <
	typename T,
	typename Arg1,
	typename Arg2
	>
void 
construct (
	T* p,
	Arg1 arg1,
	Arg2 arg2
	)
{
	new (p) T (arg1, arg2);
}

template <
	typename T,
	typename Arg1,
	typename Arg2,
	typename Arg3
	>
void 
construct (
	T* p,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	new (p) T (arg1, arg2, arg3);
}

template <
	typename T,
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4
	>
void 
construct (
	T* p,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	new (p) T (arg1, arg2, arg3, arg4);
}

template <typename T>
void 
destruct (T* p)
{
	p->~T ();
}

//.............................................................................

} // namespace jnc

#endif // __cplusplus
