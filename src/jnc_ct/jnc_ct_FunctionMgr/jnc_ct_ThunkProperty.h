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

class ThunkProperty: public Property {
	friend class FunctionMgr;

protected:
	Property* m_targetProperty;

public:
	ThunkProperty() {
		m_propertyKind = PropertyKind_Thunk;
		m_targetProperty = NULL;
	}

	bool
	create(
		Property* targetProperty,
		PropertyType* thunkPropertyType,
		bool hasUnusedClosure
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DataThunkProperty: public Property {
	friend class FunctionMgr;

protected:
	class Getter: public CompilableFunction {
	public:
		Getter() {
			m_functionKind = FunctionKind_Getter;
		}

		virtual
		bool
		compile() {
			return ((DataThunkProperty*)m_parentNamespace)->compileGetter(this);
		}
	};

	class Setter: public CompilableFunction {
	public:
		Setter() {
			m_functionKind = FunctionKind_Setter;
		}

		virtual
		bool
		compile() {
			return ((DataThunkProperty*)m_parentNamespace)->compileSetter(this);
		}
	};

protected:
	Variable* m_targetVariable;

public:
	DataThunkProperty();

protected:
	bool
	compileGetter(Function* function);

	bool
	compileSetter(Function* function);

	virtual
	Function*
	createAccessor(
		FunctionKind functionKind,
		FunctionType* type
	);
};

//..............................................................................

} // namespace ct
} // namespace jnc
