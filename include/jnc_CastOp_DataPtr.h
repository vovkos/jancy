// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_CastOp.h"
#include "jnc_DataPtrType.h"

namespace jnc {

class CBaseTypeCoord;

//.............................................................................

// array -> ptr

class CCast_DataPtr_FromArray: public CCastOperator
{
public:
	virtual
	ECast
	GetCastKind (
		const CValue& OpValue,
		CType* pType
		);

	virtual
	bool
	ConstCast (
		const CValue& OpValue,
		CType* pType,
		void* pDst
		);

	virtual
	bool
	LlvmCast (
		EStorage StorageKind,
		const CValue& OpValue,
		CType* pType,
		CValue* pResultValue
		)
	{
		SetCastError (OpValue, pType);
		return false;
	}
};

//.............................................................................

// data ptr -> data ptr

class CCast_DataPtr_Base: public CCastOperator
{
public:
	virtual
	ECast
	GetCastKind (
		const CValue& OpValue,
		CType* pType
		);

protected:
	intptr_t
	GetOffset (
		CDataPtrType* pSrcType,
		CDataPtrType* pDstType,
		CBaseTypeCoord* pCoord
		);

	intptr_t
	GetOffsetUnsafePtrValue (
		const CValue& PtrValue, 
		CDataPtrType* pSrcType,
		CDataPtrType* pDstType,
		CValue* pResultValue
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CCast_DataPtr_Normal2Normal: public CCast_DataPtr_Base
{
public:
	virtual
	bool
	ConstCast (
		const CValue& OpValue,
		CType* pType,
		void* pDst
		);

	virtual
	bool
	LlvmCast (
		EStorage StorageKind,
		const CValue& OpValue,
		CType* pType,
		CValue* pResultValue
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CCast_DataPtr_Lean2Normal: public CCast_DataPtr_Base
{
public:
	virtual
	bool
	ConstCast (
		const CValue& OpValue,
		CType* pType,
		void* pDst
		);

	virtual
	bool
	LlvmCast (
		EStorage StorageKind,
		const CValue& OpValue,
		CType* pType,
		CValue* pResultValue
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CCast_DataPtr_Normal2Thin: public CCast_DataPtr_Base
{
public:
	virtual
	bool
	ConstCast (
		const CValue& OpValue,
		CType* pType,
		void* pDst
		);

	virtual
	bool
	LlvmCast (
		EStorage StorageKind,
		const CValue& OpValue,
		CType* pType,
		CValue* pResultValue
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CCast_DataPtr_Lean2Thin: public CCast_DataPtr_Base
{
public:
	virtual
	bool
	ConstCast (
		const CValue& OpValue,
		CType* pType,
		void* pDst
		)
	{
		ASSERT (false); // there are no lean pointer constants
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

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CCast_DataPtr_Thin2Thin: public CCast_DataPtr_Lean2Thin
{
public:
	virtual
	bool
	ConstCast (
		const CValue& OpValue,
		CType* pType,
		void* pDst
		);
};

//.............................................................................

// data ptr master cast

class CCast_DataPtr: public CCast_Master
{
protected:
	CCast_DataPtr_FromArray m_FromArray;
	CCast_DataPtr_Normal2Normal m_Normal2Normal;
	CCast_DataPtr_Lean2Normal m_Lean2Normal;
	CCast_DataPtr_Normal2Thin m_Normal2Thin;
	CCast_DataPtr_Lean2Thin m_Lean2Thin;
	CCast_DataPtr_Thin2Thin m_Thin2Thin;

	CCastOperator* m_OperatorTable [EDataPtrType__Count] [EDataPtrType__Count];

public:
	CCast_DataPtr ();

	virtual
	CCastOperator*
	GetCastOperator (
		const CValue& OpValue,
		CType* pType
		);
};

//.............................................................................

// data ref (EUnOp_Indir => data ptr cast => EUnOp_Addr)

class CCast_DataRef: public CCastOperator
{
public:
	CCast_DataRef ()
	{
		m_OpFlags = EOpFlag_KeepRef;
	}

	virtual
	ECast
	GetCastKind (
		const CValue& OpValue,
		CType* pType
		);

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

} // namespace jnc {
