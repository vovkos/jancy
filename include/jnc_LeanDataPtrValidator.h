// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Value.h"

namespace jnc {

//.............................................................................

enum ELeanDataPtrValidator
{
	ELeanDataPtrValidator_Undefined,
	ELeanDataPtrValidator_Simple,
	ELeanDataPtrValidator_Complex,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CLeanDataPtrValidator: public ref::CRefCount
{
	friend class CValue;
	friend class CConstMgr;

protected:
	ELeanDataPtrValidator m_ValidatorKind;

	CValue m_ScopeValidatorValue;
	CValue m_RangeBeginValue;
	CValue m_SizeValue;

public:
	CLeanDataPtrValidator ()
	{
		m_ValidatorKind = ELeanDataPtrValidator_Undefined;
	}

	ELeanDataPtrValidator 
	GetValidatorKind ()
	{
		return m_ValidatorKind;
	}

	CValue 
	GetScopeValidator ()
	{
		return m_ScopeValidatorValue;
	}

	CValue 
	GetRangeBegin ()
	{
		return m_RangeBeginValue;
	}

	CValue
	GetSizeValue ()
	{
		return m_SizeValue;
	}
};

//.............................................................................

} // namespace jnc {
