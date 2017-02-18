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

#include "jnc_ct_Value.h"

namespace jnc {
namespace ct {

class FunctionType;
class FunctionPtrType;
class PropertyType;
class PropertyPtrType;

//..............................................................................

class Closure: public ref::RefCount
{
protected:
	sl::BoxList <Value> m_argValueList;
	Value* m_thisArgValue;
	size_t m_thisArgIdx;

public:
	Closure ()
	{
		m_thisArgValue = NULL;
		m_thisArgIdx = -1;
	}

	sl::BoxList <Value>*
	getArgValueList ()
	{
		return &m_argValueList;
	}

	bool
	isMemberClosure ()
	{
		return m_thisArgIdx != -1;
	}

	size_t
	getThisArgIdx ()
	{
		return m_thisArgIdx;
	}

	Value
	getThisArgValue ();

	void
	setThisArgIdx (size_t thisArgIdx);

	void
	insertThisArgValue (const Value& thisValue);

	bool
	isSimpleClosure ()
	{
		return isMemberClosure () && m_argValueList.getCount () == 1;
	}

	size_t
	append (const Value& argValue);

	size_t
	append (const sl::ConstBoxList <Value>& argValueList);

	bool
	apply (sl::BoxList <Value>* argValueList);

	Type*
	getClosureType (Type* type);

	FunctionPtrType*
	getFunctionClosureType (Function* function); // choose the best overload

	FunctionPtrType*
	getFunctionClosureType (FunctionPtrType* ptrType);

	PropertyPtrType*
	getPropertyClosureType (PropertyPtrType* ptrType);

protected:
	bool
	getArgTypeArray (
		Module* module,
		sl::Array <FunctionArg*>* argArray
		);
};

//..............................................................................

} // namespace ct
} // namespace jnc
