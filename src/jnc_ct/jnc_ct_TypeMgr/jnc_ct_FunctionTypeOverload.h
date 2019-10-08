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
	mutable uint_t m_flags;
	FunctionType* m_type; // avoid array allocations when possible
	sl::Array<FunctionType*> m_overloadArray;

public:
	FunctionTypeOverload()
	{
		m_flags = 0;
		m_type = NULL;
	}

	FunctionTypeOverload(FunctionType* type)
	{
		m_flags = 0;
		m_type = type;
	}

	FunctionTypeOverload(
		FunctionType* const* typeArray,
		size_t count
		)
	{
		m_flags = 0;
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

	Module*
	getModule() const
	{
		ASSERT(m_type);
		return m_type->getModule();
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
		Closure* closure,
		FunctionArg* const* argArray,
		size_t argCount,
		CastKind* castKind = NULL
		) const;

	size_t
	chooseOverload(
		FunctionArg* const* argArray,
		size_t argCount,
		CastKind* castKind = NULL
		) const
	{
		return chooseOverload(NULL, argArray, argCount, castKind);
	}

	size_t
	chooseOverload(
		Closure* closure,
		const sl::ArrayRef<FunctionArg*>& argArray,
		CastKind* castKind = NULL
		) const
	{
		return chooseOverload(closure, argArray, argArray.getCount(), castKind);
	}

	size_t
	chooseOverload(
		const sl::ArrayRef<FunctionArg*>& argArray,
		CastKind* castKind = NULL
		) const
	{
		return chooseOverload(NULL, argArray, argArray.getCount(), castKind);
	}

	size_t
	chooseOverload(
		const Value* argArray,
		size_t argCount,
		CastKind* castKind = NULL
		) const;

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

protected:
	bool
	ensureLayout() const
	{
		return (m_flags & ModuleItemFlag_LayoutReady) ? true : prepareLayout();
	}

	bool
	prepareLayout() const;
};

//..............................................................................

} // namespace ct
} // namespace jnc
