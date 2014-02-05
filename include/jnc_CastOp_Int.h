// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_CastOp.h"

namespace jnc {

//.............................................................................

// integer truncation

class CCast_IntTrunc: public CCastOperator
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

// integer extensions

class CCast_IntExt: public CCastOperator
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

class CCast_IntExt_u: public CCastOperator
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

// big endian <-> little endian

class CCast_SwapByteOrder: public CCastOperator
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

class CCast_IntFromBeInt: public CCast_SuperMaster
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

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CCast_BeInt: public CCast_SuperMaster
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

// floating point -> integer

class CCast_IntFromFp: public CCastOperator
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
	LlvmCast (
		EStorage StorageKind,
		const CValue& OpValue,
		CType* pType,
		CValue* pResultValue
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CCast_IntFromFp32: public CCast_IntFromFp
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

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CCast_IntFromFp64: public CCast_IntFromFp
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

// enum <-> integer

class CCast_IntFromEnum: public CCast_SuperMaster
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

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CCast_Enum: public CCast_SuperMaster
{
public:
	CCast_Enum ()
	{
		m_OpFlags = EOpFlag_KeepEnum;
	}

	virtual
	ECast
	GetCastKind (
		const CValue& OpValue,
		CType* pType
		);

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

// pointer <-> integer

class CCast_IntFromPtr: public CCastOperator
{
public:
	virtual
	ECast
	GetCastKind (
		const CValue& OpValue,
		CType* pType
		)
	{
		ASSERT (OpValue.GetType ()->GetSize () >= sizeof (intptr_t));
		return ECast_Explicit;
	}

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

class CCast_PtrFromInt: public CCastOperator
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

// integer master cast

class CCast_Int: public CCast_Master
{
protected:
	CCast_IntTrunc m_Trunc;
	CCast_IntExt m_Ext;
	CCast_IntExt_u m_Ext_u;
	CCast_IntFromBeInt m_FromBeInt;
	CCast_IntFromFp32 m_FromFp32;
	CCast_IntFromFp64 m_FromFp64;
	CCast_IntFromEnum m_FromEnum;
	CCast_IntFromPtr m_FromPtr;

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
