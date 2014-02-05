// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#define _JNC_CALLFUNCTION

#include "jnc_Function.h"

namespace jnc {

//.............................................................................

template <typename TRetVal>
bool
CallFunction (
	CFunction* pFunction,
	TRetVal* pRetVal
	)
{
	typedef
	TRetVal
	FFunction ();

	FFunction* pf = (FFunction*) pFunction->GetMachineCode ();
	ASSERT (pf);

	bool Result = true;

	try
	{
		*pRetVal = pf ();
	}
	catch (err::CError Error)
	{
		err::SetError (Error);
		Result = false;
	}

	return Result;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <
	typename TRetVal,
	typename TArg
	>
bool
CallFunction (
	CFunction* pFunction,
	TRetVal* pRetVal,
	TArg Arg
	)
{
	typedef
	TRetVal
	FFunction (TArg);

	FFunction* pf = (FFunction*) pFunction->GetMachineCode ();
	ASSERT (pf);

	bool Result = true;

	try
	{
		*pRetVal = pf (Arg);
	}
	catch (err::CError Error)
	{
		err::SetError (Error);
		Result = false;
	}

	return Result;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <
	typename TRetVal,
	typename TArg1,
	typename TArg2
	>
bool
CallFunction (
	CFunction* pFunction,
	TRetVal* pRetVal,
	TArg1 Arg1,
	TArg2 Arg2
	)
{
	typedef
	TRetVal
	FFunction (
		TArg1,
		TArg2
		);

	FFunction* pf = (FFunction*) pFunction->GetMachineCode ();
	ASSERT (pf);

	bool Result = true;

	try
	{
		*pRetVal = pf (Arg1, Arg2);
	}
	catch (err::CError Error)
	{
		err::SetError (Error);
		Result = false;
	}

	return Result;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <
	typename TRetVal,
	typename TArg1,
	typename TArg2,
	typename TArg3
	>
bool
CallFunction (
	CFunction* pFunction,
	TRetVal* pRetVal,
	TArg1 Arg1,
	TArg2 Arg2,
	TArg3 Arg3
	)
{
	typedef
	TRetVal
	FFunction (
		TArg1,
		TArg2,
		TArg3
		);

	FFunction* pf = (FFunction*) pFunction->GetMachineCode ();
	ASSERT (pf);

	bool Result = true;

	try
	{
		*pRetVal = pf (Arg1, Arg2, Arg3);
	}
	catch (err::CError Error)
	{
		err::SetError (Error);
		Result = false;
	}

	return Result;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <
	typename TRetVal,
	typename TArg1,
	typename TArg2,
	typename TArg3,
	typename TArg4
	>
bool
CallFunction (
	CFunction* pFunction,
	TRetVal* pRetVal,
	TArg1 Arg1,
	TArg2 Arg2,
	TArg3 Arg3,
	TArg3 Arg4
	)
{
	typedef
	TRetVal
	FFunction (
		TArg1,
		TArg2,
		TArg3,
		TArg4
		);

	FFunction* pf = (FFunction*) pFunction->GetMachineCode ();
	ASSERT (pf);

	bool Result = true;

	try
	{
		*pRetVal = pf (Arg1, Arg2, Arg3, Arg4);
	}
	catch (err::CError Error)
	{
		err::SetError (Error);
		Result = false;
	}

	return Result;
}

//.............................................................................

inline
bool
CallVoidFunction (CFunction* pFunction)
{
	int RetVal;
	return CallFunction <int> (pFunction, &RetVal);
}

template <typename TArg>
bool
CallVoidFunction (
	CFunction* pFunction,
	TArg Arg
	)
{
	int RetVal;
	return CallFunction <int, TArg> (pFunction, &RetVal, Arg);
}

template <
	typename TArg1,
	typename TArg2
	>
bool
CallVoidFunction (
	CFunction* pFunction,
	TArg1 Arg1,
	TArg2 Arg2
	)
{
	int RetVal;
	return CallFunction <int, TArg1, TArg2> (pFunction, &RetVal, Arg1, Arg2);
}

template <
	typename TArg1,
	typename TArg2,
	typename TArg3
	>
bool
CallVoidFunction (
	CFunction* pFunction,
	TArg1 Arg1,
	TArg2 Arg2,
	TArg3 Arg3
	)
{
	int RetVal;
	return CallFunction <int, TArg1, TArg2, TArg3> (
		pFunction,
		&RetVal,
		Arg1,
		Arg2,
		Arg3
		);
}

template <
	typename TArg1,
	typename TArg2,
	typename TArg3,
	typename TArg4
	>
bool
CallVoidFunction (
	CFunction* pFunction,
	TArg1 Arg1,
	TArg2 Arg2,
	TArg3 Arg3,
	TArg3 Arg4
	)
{
	int RetVal;
	return CallFunction <int, TArg1, TArg2, TArg3, TArg4> (
		pFunction,
		&RetVal,
		Arg1,
		Arg2,
		Arg3,
		Arg4
		);
}

//.............................................................................

} // namespace jnc {
