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

#include "jnc_ct_Property.h"

namespace jnc {
namespace ct {

//..............................................................................

class ThunkProperty: public Property
{
	friend class FunctionMgr;

protected:
	sl::String m_signature;
	Property* m_targetProperty;

public:
	ThunkProperty();

	bool
	create(
		Property* targetProperty,
		PropertyType* thunkPropertyType,
		bool hasUnusedClosure
		);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DataThunkProperty: public Property
{
	friend class FunctionMgr;

protected:
	sl::String m_signature;
	Variable* m_targetVariable;

public:
	DataThunkProperty();

	virtual
	bool
	compile();

protected:
	bool
	compileGetter();

	bool
	compileSetter(Function* setter);
};

//..............................................................................

} // namespace ct
} // namespace jnc
