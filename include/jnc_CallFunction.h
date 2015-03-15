// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#define _JNC_CALLFUNCTION

#include "jnc_Function.h"

namespace jnc {

//.............................................................................

template <typename RetVal>
bool
callFunction (
	Function* function,
	RetVal* retVal
	)
{
	typedef
	RetVal
	FFunction ();

	FFunction* pf = (FFunction*) function->getMachineCode ();
	ASSERT (pf);

	AXL_MT_BEGIN_LONG_JMP_TRY ()
	{
		*retVal = pf ();
	}
	AXL_MT_LONG_JMP_CATCH ()
	{
		return false;
	}
	AXL_MT_END_LONG_JMP_TRY ()

	return true;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <
	typename RetVal,
	typename Arg
	>
bool
callFunction (
	Function* function,
	RetVal* retVal,
	Arg arg
	)
{
	typedef
	RetVal
	FFunction (Arg);

	FFunction* pf = (FFunction*) function->getMachineCode ();
	ASSERT (pf);

	AXL_MT_BEGIN_LONG_JMP_TRY ()
	{
		*retVal = pf (arg);
	}
	AXL_MT_LONG_JMP_CATCH ()
	{
		return false;
	}
	AXL_MT_END_LONG_JMP_TRY ()

	return true;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <
	typename RetVal,
	typename Arg1,
	typename Arg2
	>
bool
callFunction (
	Function* function,
	RetVal* retVal,
	Arg1 arg1,
	Arg2 arg2
	)
{
	typedef
	RetVal
	FFunction (
		Arg1,
		Arg2
		);

	FFunction* pf = (FFunction*) function->getMachineCode ();
	ASSERT (pf);

	AXL_MT_BEGIN_LONG_JMP_TRY ()
	{
		*retVal = pf (arg1, arg2);
	}
	AXL_MT_LONG_JMP_CATCH ()
	{
		return false;
	}
	AXL_MT_END_LONG_JMP_TRY ()

	return true;
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
	Function* function,
	RetVal* retVal,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	typedef
	RetVal
	FFunction (
		Arg1,
		Arg2,
		Arg3
		);

	FFunction* pf = (FFunction*) function->getMachineCode ();
	ASSERT (pf);

	AXL_MT_BEGIN_LONG_JMP_TRY ()
	{
		*retVal = pf (arg1, arg2, arg3);
	}
	AXL_MT_LONG_JMP_CATCH ()
	{
		return false;
	}
	AXL_MT_END_LONG_JMP_TRY ()

	return true;
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
	FFunction (
		Arg1,
		Arg2,
		Arg3,
		Arg4
		);

	FFunction* pf = (FFunction*) function->getMachineCode ();
	ASSERT (pf);

	AXL_MT_BEGIN_LONG_JMP_TRY ()
	{
		*retVal = pf (arg1, arg2, arg3, arg4);
	}
	AXL_MT_LONG_JMP_CATCH ()
	{
		return false;
	}
	AXL_MT_END_LONG_JMP_TRY ()

	return true;
}

//.............................................................................

inline
bool
callVoidFunction (Function* function)
{
	int retVal;
	return callFunction <int> (function, &retVal);
}

template <typename Arg>
bool
callVoidFunction (
	Function* function,
	Arg arg
	)
{
	int retVal;
	return callFunction <int, Arg> (function, &retVal, arg);
}

template <
	typename Arg1,
	typename Arg2
	>
bool
callVoidFunction (
	Function* function,
	Arg1 arg1,
	Arg2 arg2
	)
{
	int retVal;
	return callFunction <int, Arg1, Arg2> (function, &retVal, arg1, arg2);
}

template <
	typename Arg1,
	typename Arg2,
	typename Arg3
	>
bool
callVoidFunction (
	Function* function,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	int retVal;
	return callFunction <int, Arg1, Arg2, Arg3> (
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
	Function* function,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg3 arg4
	)
{
	int retVal;
	return callFunction <int, Arg1, Arg2, Arg3, Arg4> (
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
