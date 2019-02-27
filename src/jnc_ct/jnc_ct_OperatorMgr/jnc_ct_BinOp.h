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

#include "jnc_ct_UnOp.h"

namespace jnc {
namespace ct {

//..............................................................................

class BinaryOperator
{
	friend class OperatorMgr;

protected:
	Module* m_module;
	BinOpKind m_opKind;
	uint_t m_opFlags1;
	uint_t m_opFlags2;

public:
	BinaryOperator();

	Module*
	getModule()
	{
		return m_module;
	}

	BinOpKind
	getOpKind()
	{
		return m_opKind;
	}

	int
	getOpFlags1()
	{
		return m_opFlags1;
	}

	int
	getOpFlags2()
	{
		return m_opFlags2;
	}

	bool
	getResultType(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
		);

	virtual
	Type*
	getResultType(
		const Value& opValue1,
		const Value& opValue2
		) = 0;

	virtual
	bool
	op(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
		) = 0;

	err::Error
	setOperatorError(
		Type* opType1,
		Type* opType2
		)
	{
		return err::setFormatStringError(
			"binary '%s' cannot be applied to '%s' and '%s'",
			getBinOpKindString(m_opKind),
			opType1->getTypeString().sz(),
			opType2->getTypeString().sz()
			);
	}

	err::Error
	setOperatorError(
		const Value& opValue1,
		const Value& opValue2
		)
	{
		return setOperatorError(opValue1.getType(), opValue2.getType());
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
