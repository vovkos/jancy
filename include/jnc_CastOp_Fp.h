// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_CastOp.h"

namespace jnc {

//.............................................................................

// floating point truncation

class CCast_FpTrunc: public CCastOperator
{
public:
	virtual
	ECast
	GetCastKind (
		const CValue& OpValue,
		CType* pType
		)
	{
		return ECast_Explicit;
	}

	virtual
	bool
	ConstCast (
		const CValue& OpValue,
		CType* pType,
		void* pDst
		)
	{
		*(float*) pDst = (float) *(double*) OpValue.GetConstData ();
		return true;
	}

	virtual
	bool
	LlvmCast (
		EStorage StorageKind,
		const CValue& OpValue,
		CType* pType,
		CValue* pResultValue
		);
};

//.............................................................................

// floating point extension

class CCast_FpExt: public CCastOperator
{
public:
	virtual
	ECast
	GetCastKind (
		const CValue& OpValue,
		CType* pType
		)
	{
		return ECast_Implicit;
	}

	virtual
	bool
	ConstCast (
		const CValue& OpValue,
		CType* pType,
		void* pDst
		)
	{
		*(double*) pDst = *(float*) OpValue.GetConstData ();
		return true;
	}

	virtual
	bool
	LlvmCast (
		EStorage StorageKind,
		const CValue& OpValue,
		CType* pType,
		CValue* pResultValue
		);
};

//.............................................................................

// signed integer -> floating point

class CCast_FpFromInt: public CCastOperator
{
public:
	virtual
	ECast
	GetCastKind (
		const CValue& OpValue,
		CType* pType
		)
	{
		return OpValue.GetType ()->GetSize () < pType->GetSize () ? ECast_ImplicitCrossFamily : ECast_Explicit;
	}

	virtual
	bool
	LlvmCast (
		EStorage StorageKind,
		const CValue& OpValue,
		CType* pType,
		CValue* pResultValue
		);

	virtual
	bool
	ConstCast (
		const CValue& OpValue,
		CType* pType,
		void* pDst
		);

protected:
	void
	ConstCast_Fp32 (
		const CValue& SrcValue,
		float* pFp32
		);

	void
	ConstCast_Fp64 (
		const CValue& SrcValue,
		double* pFp64
		);
};

//.............................................................................

// unsigned integer -> floating point

class CCast_FpFromInt_u: public CCastOperator
{
public:
	virtual
	ECast
	GetCastKind (
		const CValue& OpValue,
		CType* pType
		)
	{
		return OpValue.GetType ()->GetSize () < pType->GetSize () ? ECast_ImplicitCrossFamily : ECast_Explicit;
	}

	virtual
	bool
	LlvmCast (
		EStorage StorageKind,
		const CValue& OpValue,
		CType* pType,
		CValue* pResultValue
		);

	virtual
	bool
	ConstCast (
		const CValue& OpValue,
		CType* pType,
		void* pDst
		);

protected:
	void
	ConstCast_Fp32 (
		const CValue& SrcValue,
		float* pFp32
		);

	void
	ConstCast_Fp64 (
		const CValue& SrcValue,
		double* pFp64
		);
};

//.............................................................................

// bigendian integer -> floating point

class CCast_FpFromBeInt: public CCast_SuperMaster
{
public:
	virtual
	bool
	GetCastOperators (
		const CValue& OpValue,
		CType* pType,
		CCastOperator** ppFirstOperator,
		CCastOperator** ppSecondOperator,
		CType** ppIntermediateType
		);
};

//.............................................................................

// enum -> floating point

class CCast_FpFromEnum: public CCast_SuperMaster
{
public:
	virtual
	bool
	GetCastOperators (
		const CValue& OpValue,
		CType* pType,
		CCastOperator** ppFirstOperator,
		CCastOperator** ppSecondOperator,
		CType** ppIntermediateType
		);
};

//.............................................................................

// floating point master cast

class CCast_Fp: public CCast_Master
{
protected:
	CCast_FpTrunc m_Trunc;
	CCast_FpExt m_Ext;
	CCast_FpFromInt m_FromInt;
	CCast_FpFromInt_u m_FromInt_u;
	CCast_FpFromBeInt m_FromBeInt;
	CCast_FpFromEnum m_FromEnum;

public:
	virtual
	CCastOperator*
	GetCastOperator (
		const CValue& OpValue,
		CType* pType
		);
};

//.............................................................................

} // namespace jnc {
