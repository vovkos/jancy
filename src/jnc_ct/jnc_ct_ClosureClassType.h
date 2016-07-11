// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_ClassType.h"
#include "jnc_ct_ThunkFunction.h"
#include "jnc_ct_Closure.h"

namespace jnc {
namespace ct {

//.............................................................................

class ClosureClassType: public ClassType
{
	friend class TypeMgr;

public:
	sl::Array <size_t> m_closureMap;
	size_t m_thisArgFieldIdx;

public:
	ClosureClassType ()
	{
		m_flags |= ClassTypeFlag_Closure;
		m_thisArgFieldIdx = -1;
	}

	size_t
	getThisArgFieldIdx ()
	{
		return m_thisArgFieldIdx;
	}

	static
	sl::String
	createSignature (
		Type* targetType, // function or property
		Type* thunkType, // function or property
		Type* const* argTypeArray,
		const size_t* closureMap,
		size_t closureArgCount,
		size_t thisArgIdx
		);

	virtual 
	bool
	compile () = 0;

	IfaceHdr* 
	strengthen (IfaceHdr* p);

protected:
	void
	buildArgValueList (
		const Value& closureValue,
		const Value* thunkArgValueArray,
		size_t thunkArgCount,
		sl::BoxList <Value>* argValueList
		);
};

//.............................................................................

class FunctionClosureClassType: public ClosureClassType
{
	friend class TypeMgr;

protected:
	Function* m_thunkFunction;

public:
	FunctionClosureClassType ();

	Function*
	getThunkFunction ()
	{
		return m_thunkFunction;
	}

	virtual 
	bool
	compile ();
};

//.............................................................................

class PropertyClosureClassType: public ClosureClassType
{
	friend class TypeMgr;

protected:
	Property* m_thunkProperty;

public:
	PropertyClosureClassType ();

	Property*
	getThunkProperty ()
	{
		return m_thunkProperty;
	}

	virtual 
	bool
	compile ();

protected:
	bool
	compileAccessor (Function* accessor);
};

//.............................................................................

class DataClosureClassType: public ClassType
{
	friend class TypeMgr;

protected:
	Property* m_thunkProperty;

public:
	DataClosureClassType ();

	Property*
	getThunkProperty ()
	{
		return m_thunkProperty;
	}

	static
	sl::String
	createSignature (
		Type* targetType,
		PropertyType* thunkType
		);

	virtual 
	bool
	compile ();

protected:
	bool
	compileGetter (Function* getter);

	bool
	compileSetter (Function* setter);
};

//.............................................................................

} // namespace ct
} // namespace jnc
