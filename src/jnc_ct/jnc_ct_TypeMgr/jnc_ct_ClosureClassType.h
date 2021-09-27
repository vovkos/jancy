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

#include "jnc_ct_ClassType.h"
#include "jnc_ct_Property.h"

namespace jnc {
namespace ct {

//..............................................................................

class ClosureClassType: public ClassType {
	friend class TypeMgr;

public:
	sl::Array<size_t> m_closureMap;
	size_t m_thisArgFieldIdx;

public:
	ClosureClassType();

	size_t
	getThisArgFieldIdx() {
		return m_thisArgFieldIdx;
	}

	static
	sl::String
	createSignature(
		Type* targetType, // function or property
		Type* thunkType, // function or property
		Type* const* argTypeArray,
		const size_t* closureMap,
		size_t closureArgCount,
		size_t thisArgIdx
	);

	IfaceHdr*
	strengthen(IfaceHdr* p);

protected:
	void
	buildArgValueList(
		const Value& closureValue,
		const Value* thunkArgValueArray,
		size_t thunkArgCount,
		sl::BoxList<Value>* argValueList
	);
};

//..............................................................................

class FunctionClosureClassType: public ClosureClassType {
	friend class TypeMgr;

protected:
	class ThunkFunction: public CompilableFunction {
	public:
		virtual
		bool
		compile() {
			return ((FunctionClosureClassType*)m_parentNamespace)->compileThunkFunction(this);
		}
	};

protected:
	Function* m_thunkFunction;

public:
	FunctionClosureClassType();

	Function*
	getThunkFunction() {
		return m_thunkFunction;
	}

protected:
	bool
	compileThunkFunction(Function* function);
};

//..............................................................................

class PropertyClosureClassType: public ClosureClassType {
	friend class TypeMgr;

protected:
	class ThunkProperty: public Property {
	protected:
		virtual
		Function*
		createAccessor(
			FunctionKind functionKind,
			FunctionType* type
		);
	};

	class Accessor: public CompilableFunction {
	public:
		virtual
		bool
		compile() {
			return ((PropertyClosureClassType*)((Property*)m_parentNamespace)->getParentNamespace())->compileAccessor(this);
		}
	};

protected:
	Property* m_thunkProperty;

public:
	PropertyClosureClassType();

	Property*
	getThunkProperty() {
		return m_thunkProperty;
	}

protected:
	bool
	compileAccessor(Function* accessor);
};

//..............................................................................

class DataClosureClassType: public ClassType {
	friend class TypeMgr;

protected:
	class ThunkProperty: public Property {
	protected:
		virtual
		Function*
		createAccessor(
			FunctionKind functionKind,
			FunctionType* type
		);
	};

	class Getter: public CompilableFunction {
	public:
		virtual
		bool
		compile() {
			return ((DataClosureClassType*)((Property*)m_parentNamespace)->getParentNamespace())->compileGetter(this);
		}
	};

	class Setter: public CompilableFunction {
	public:
		virtual
		bool
		compile() {
			return ((DataClosureClassType*)((Property*)m_parentNamespace)->getParentNamespace())->compileSetter(this);
		}
	};

protected:
	Property* m_thunkProperty;

public:
	DataClosureClassType();

	Property*
	getThunkProperty() {
		return m_thunkProperty;
	}

	static
	sl::String
	createSignature(
		Type* targetType,
		PropertyType* thunkType
	);

protected:
	bool
	compileGetter(Function* getter);

	bool
	compileSetter(Function* setter);
};

//..............................................................................

} // namespace ct
} // namespace jnc
