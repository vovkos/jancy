// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Function.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

template <typename RetVal>
bool
callFunction (
	Runtime* runtime,
	Function* function,
	RetVal* retVal
	)
{
	typedef
	RetVal
	TargetFunc ();

	TargetFunc* p = (TargetFunc*) function->getMachineCode ();
	ASSERT (p);

	bool result = true;

	JNC_BEGIN (runtime)
	{
		*retVal = p ();
	}
	JNC_CATCH ()
	{
		result = false;
	}
	JNC_END ()

	return result;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <
	typename RetVal,
	typename Arg
	>
bool
callFunction (
	Runtime* runtime,
	Function* function,
	RetVal* retVal,
	Arg arg
	)
{
	typedef
	RetVal
	TargetFunc (Arg);

	TargetFunc* p = (TargetFunc*) function->getMachineCode ();
	ASSERT (p);

	bool result = true;

	JNC_BEGIN (runtime)
	{
		*retVal = p (arg);
	}
	JNC_CATCH ()
	{
		result = false;
	}
	JNC_END ()

	return result;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <
	typename RetVal,
	typename Arg1,
	typename Arg2
	>
bool
callFunction (
	Runtime* runtime,
	Function* function,
	RetVal* retVal,
	Arg1 arg1,
	Arg2 arg2
	)
{
	typedef
	RetVal
	TargetFunc (
		Arg1,
		Arg2
		);

	TargetFunc* p = (TargetFunc*) function->getMachineCode ();
	ASSERT (p);

	bool result = true;

	JNC_BEGIN (runtime)
	{
		*retVal = p (arg1, arg2);
	}
	JNC_CATCH ()
	{
		result = false;
	}
	JNC_END ()

	return result;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <
	typename RetVal,
	typename Arg1,
	typename Arg2,
	typename Arg3
	>
bool
callFunction (
	Runtime* runtime,
	Function* function,
	RetVal* retVal,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	typedef
	RetVal
	TargetFunc (
		Arg1,
		Arg2,
		Arg3
		);

	TargetFunc* p = (TargetFunc*) function->getMachineCode ();
	ASSERT (p);

	bool result = true;

	JNC_BEGIN (runtime)
	{
		*retVal = p (arg1, arg2, arg3);
	}
	JNC_CATCH ()
	{
		result = false;
	}
	JNC_END ()

	return result;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <
	typename RetVal,
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4
	>
bool
callFunction (
	Runtime* runtime,
	Function* function,
	RetVal* retVal,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg3 arg4
	)
{
	typedef
	RetVal
	TargetFunc (
		Arg1,
		Arg2,
		Arg3,
		Arg4
		);

	TargetFunc* p = (TargetFunc*) function->getMachineCode ();
	ASSERT (p);

	bool result = true;

	JNC_BEGIN (runtime)
	{
		*retVal = p (arg1, arg2, arg3, arg4);
	}
	JNC_CATCH ()
	{
		result = false;
	}
	JNC_END ()

	return result;
}

//.............................................................................

inline
bool
callVoidFunction (
	Runtime* runtime,
	Function* function
	)
{
	int retVal;
	return callFunction <int> (runtime, function, &retVal);
}

template <typename Arg>
bool
callVoidFunction (
	Runtime* runtime,
	Function* function,
	Arg arg
	)
{
	int retVal;
	return callFunction <int, Arg> (runtime, function, &retVal, arg);
}

template <
	typename Arg1,
	typename Arg2
	>
bool
callVoidFunction (
	Runtime* runtime,
	Function* function,
	Arg1 arg1,
	Arg2 arg2
	)
{
	int retVal;
	return callFunction <int, Arg1, Arg2> (
		runtime,
		function, 
		&retVal, 
		arg1, 
		arg2
		);
}

template <
	typename Arg1,
	typename Arg2,
	typename Arg3
	>
bool
callVoidFunction (
	Runtime* runtime,
	Function* function,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	int retVal;
	return callFunction <int, Arg1, Arg2, Arg3> (
		runtime,
		function,
		&retVal,
		arg1,
		arg2,
		arg3
		);
}

template <
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4
	>
bool
callVoidFunction (
	Runtime* runtime,
	Function* function,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg3 arg4
	)
{
	int retVal;
	return callFunction <int, Arg1, Arg2, Arg3, Arg4> (
		runtime,
		function,
		&retVal,
		arg1,
		arg2,
		arg3,
		arg4
		);
}

//.............................................................................

} // namespace jnc {
