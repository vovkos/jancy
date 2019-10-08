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

#include "jnc_Function.h"
#include "jnc_OpKind.h"

namespace jnc {
namespace ct {

class DerivableType;
class ClassType;
class PropertyType;
class Property;
class Scope;

//..............................................................................

// shared between Function, FunctionOverload and Orphan

class FunctionName
{
	friend class Parser;

protected:
	FunctionKind m_functionKind;

	union
	{
		UnOpKind m_unOpKind;
		BinOpKind m_binOpKind;
		Type* m_castOpType;
		Function* m_asyncLauncher;
	};

	uint_t m_thisArgTypeFlags;

public:
	FunctionName()
	{
		m_functionKind = FunctionKind_Undefined;
		m_castOpType = NULL;
		m_thisArgTypeFlags = 0;
	}

	FunctionKind
	getFunctionKind()
	{
		return m_functionKind;
	}

	UnOpKind
	getUnOpKind()
	{
		ASSERT(m_functionKind == FunctionKind_UnaryOperator);
		return m_unOpKind;
	}

	BinOpKind
	getBinOpKind()
	{
		ASSERT(m_functionKind == FunctionKind_BinaryOperator);
		return m_binOpKind;
	}

	Type*
	getCastOpType()
	{
		ASSERT(m_functionKind == FunctionKind_CastOperator);
		return m_castOpType;
	}

	Function*
	getAsyncLauncher()
	{
		ASSERT(m_functionKind == FunctionKind_AsyncSequencer);
		return m_asyncLauncher;
	}

	uint_t
	getThisArgTypeFlags()
	{
		return m_thisArgTypeFlags;
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
