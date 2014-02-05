// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_FunctionType.h"
#include "jnc_Value.h"
#include "jnc_CastOp.h"

namespace jnc {

//.............................................................................

class CFunctionTypeOverload
{
protected:
	uint_t m_Flags;
	CFunctionType* m_pType;
	rtl::CArrayT <CFunctionType*> m_OverloadArray;

public:
	CFunctionTypeOverload ()
	{
		m_Flags = 0;
		m_pType = NULL;
	}

	CFunctionTypeOverload (CFunctionType* pType)
	{
		m_pType = pType;
	}

	CFunctionTypeOverload (
		CFunctionType* const* ppTypeArray,
		size_t Count
		)
	{
		Copy (ppTypeArray, Count);
	}

	operator CFunctionType* ()
	{
		return m_pType;
	}

	CFunctionTypeOverload&
	operator = (CFunctionType* pType)
	{
		m_pType = pType;
		m_OverloadArray.Clear ();
		return *this;
	}

	bool
	IsEmpty () const
	{
		return m_pType == NULL;
	}

	bool
	IsOverloaded () const
	{
		return !m_OverloadArray.IsEmpty ();
	}

	size_t
	GetOverloadCount () const
	{
		return m_pType ? m_OverloadArray.GetCount () + 1 : 0;
	}

	CFunctionType*
	GetOverload (size_t Overload = 0) const
	{
		return
			Overload == 0 ? m_pType :
			Overload <= m_OverloadArray.GetCount () ? m_OverloadArray [Overload - 1] : NULL;
	}

	size_t
	FindOverload (CFunctionType* pType) const;

	size_t
	FindShortOverload (CFunctionType* pType) const;

	size_t
	ChooseOverload (
		CFunctionArg* const* pArgArray,
		size_t ArgCount,
		ECast* pCastKind = NULL
		) const;

	size_t
	ChooseOverload (
		const rtl::CArrayT <CFunctionArg*>& ArgArray,
		ECast* pCastKind = NULL
		) const
	{
		return ChooseOverload (ArgArray, ArgArray.GetCount (), pCastKind);
	}

	size_t
	ChooseOverload (
		const rtl::CConstBoxListT <CValue>& ArgList,
		ECast* pCastKind = NULL
		) const;

	size_t
	ChooseSetterOverload (
		const CValue& ArgValue,
		ECast* pCastKind = NULL
		) const;

	size_t
	ChooseSetterOverload (
		CFunctionType* pFunctionType,
		ECast* pCastKind = NULL
		) const
	{
		return ChooseSetterOverload (pFunctionType->GetArgArray ().GetBack ()->GetType (), pCastKind);
	}

	bool
	AddOverload (CFunctionType* pType);

	void
	Copy (
		CFunctionType* const* ppTypeArray,
		size_t Count
		);

	bool
	EnsureLayout ();
};

//.............................................................................

} // namespace jnc {
