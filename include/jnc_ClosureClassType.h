// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ClassType.h"
#include "jnc_ThunkFunction.h"
#include "jnc_Closure.h"

namespace jnc {

//.............................................................................

class ClosureClassType: public ClassType
{
	friend class TypeMgr;

protected:
	uint64_t m_weakMask;

public: // tmp
	rtl::Array <size_t> m_closureMap;

public:
	ClosureClassType ()
	{
		m_weakMask = 0;
	}

	uint64_t 
	getWeakMask ()
	{
		return m_weakMask;
	}

	static
	rtl::String
	createSignature (
		Type* targetType, // function or property
		Type* thunkType, // function or property
		Type* const* argTypeArray,
		const size_t* closureMap,
		size_t closureArgCount,
		uint64_t weakMask
		);

	virtual 
	bool
	compile () = 0;

	jnc::IfaceHdr* 
	strengthen (jnc::IfaceHdr* p);

protected:
	void
	buildArgValueList (
		const Value& closureValue,
		const Value* thunkArgValueArray,
		size_t thunkArgCount,
		rtl::BoxList <Value>* argValueList
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
	rtl::String
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

} // namespace jnc {
