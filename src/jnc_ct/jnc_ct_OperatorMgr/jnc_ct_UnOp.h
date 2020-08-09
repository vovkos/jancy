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

#include "jnc_OpKind.h"
#include "jnc_ct_Value.h"

namespace jnc {
namespace ct {

//..............................................................................

enum OpFlag
{
	OpFlag_KeepDataRef           = 0x0001,
	OpFlag_KeepClassRef	         = 0x0002,
	OpFlag_KeepFunctionRef       = 0x0004,
	OpFlag_KeepPropertyRef       = 0x0008,
	OpFlag_KeepDerivableRef      = 0x0010,
	OpFlag_KeepVariantRef        = 0x0020,
	OpFlag_KeepBool              = 0x0040,
	OpFlag_KeepEnum              = 0x0080,
	OpFlag_ArrayRefToPtr         = 0x0100,
	OpFlag_LoadArrayRef          = 0x0200,
	OpFlag_EnsurePtrTargetLayout = 0x0400,

	OpFlag_KeepRef =
		OpFlag_KeepDataRef |
		OpFlag_KeepClassRef |
		OpFlag_KeepFunctionRef |
		OpFlag_KeepPropertyRef,
};

//..............................................................................

bool
hasCodeGen(Module* module);

Type*
getPrimitiveType(
	Module* module,
	TypeKind typeKind
	);

//..............................................................................

class UnaryOperator
{
	friend class OperatorMgr;

protected:
	Module* m_module;
	UnOpKind m_opKind;
	uint_t m_opFlags;

public:
	UnaryOperator();

	Module*
	getModule()
	{
		return m_module;
	}

	UnOpKind
	getOpKind()
	{
		return m_opKind;
	}

	int
	getOpFlags()
	{
		return m_opFlags;
	}

	virtual
	bool
	op(
		const Value& opValue,
		Value* resultValue
		) = 0;

	err::Error
	setOperatorError(Type* opType)
	{
		return err::setFormatStringError(
			"unary '%s' cannot be applied to '%s'",
			getUnOpKindString(m_opKind),
			opType->getTypeString().sz()
			);
	}

	err::Error
	setOperatorError(const Value& opValue)
	{
		return setOperatorError(opValue.getType());
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
