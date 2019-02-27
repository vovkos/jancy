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

#include "jnc_ct_FunctionType.h"
#include "jnc_ct_Value.h"
#include "jnc_ct_CastOp.h"

namespace jnc {
namespace ct {

//..............................................................................

class FunctionTypeOverload
{
protected:
	uint_t m_flags;
	FunctionType* m_type;
	sl::Array<FunctionType*> m_overloadArray;

public:
	FunctionTypeOverload()
	{
		m_flags = 0;
		m_type = NULL;
	}

	FunctionTypeOverload(FunctionType* type)
	{
		m_type = type;
	}

	FunctionTypeOverload(
		FunctionType* const* typeArray,
		size_t count
		)
	{
		copy(typeArray, count);
	}

	operator FunctionType* ()
	{
		return m_type;
	}

	FunctionTypeOverload&
	operator = (FunctionType* type)
	{
		m_type = type;
		m_overloadArray.clear();
		return *this;
	}

	bool
	isEmpty() const
	{
		return m_type == NULL;
	}

	bool
	isOverloaded() const
	{
		return !m_overloadArray.isEmpty();
	}

	size_t
	getOverloadCount() const
	{
		return m_type ? m_overloadArray.getCount() + 1 : 0;
	}

	FunctionType*
	getOverload(size_t overloadIdx = 0) const
	{
		return
			overloadIdx == 0 ? m_type :
			overloadIdx <= m_overloadArray.getCount() ? m_overloadArray[overloadIdx - 1] : NULL;
	}

	size_t
	findOverload(FunctionType* type) const;

	size_t
	findShortOverload(FunctionType* type) const;

	size_t
	chooseOverload(
		FunctionArg* const* argArray,
		size_t argCount,
		CastKind* castKind = NULL
		) const;

	size_t
	chooseOverload(
		const Value* argArray,
		size_t argCount,
		CastKind* castKind = NULL
		) const;

	size_t
	chooseOverload(
		const sl::Array<FunctionArg*>& argArray,
		CastKind* castKind = NULL
		) const
	{
		return chooseOverload(argArray, argArray.getCount(), castKind);
	}

	size_t
	chooseOverload(
		const sl::ConstBoxList<Value>& argList,
		CastKind* castKind = NULL
		) const;

	size_t
	chooseSetterOverload(
		const Value& argValue,
		CastKind* castKind = NULL
		) const;

	size_t
	chooseSetterOverload(
		FunctionType* functionType,
		CastKind* castKind = NULL
		) const
	{
		return chooseSetterOverload(functionType->getArgArray().getBack()->getType(), castKind);
	}

	size_t
	addOverload(FunctionType* type);

	void
	copy(
		FunctionType* const* typeArray,
		size_t count
		);

	bool
	ensureLayout();
};

//..............................................................................

} // namespace ct
} // namespace jnc
