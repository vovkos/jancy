// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Property.h"

namespace jnc {

//.............................................................................

class ThunkProperty: public Property
{
	friend class FunctionMgr;

protected:
	rtl::String m_signature;
	Property* m_targetProperty;

public:
	ThunkProperty ();

	bool
	create (
		Property* targetProperty,
		PropertyType* thunkPropertyType,
		bool hasUnusedClosure
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DataThunkProperty: public Property
{
	friend class FunctionMgr;

protected:
	rtl::String m_signature;
	Variable* m_targetVariable;

public:
	DataThunkProperty ();

	virtual
	bool 
	compile ();

protected:
	bool 
	compileGetter ();

	bool 
	compileSetter (Function* setter);
};

//.............................................................................

} // namespace jnc {
