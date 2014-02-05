// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_UnOp.h"

namespace jnc {

class CFunctionType;
class CFunctionPtrType;

//.............................................................................

// ordered from the worst to the best

enum ECast
{
	ECast_None,
	ECast_Explicit,
	ECast_ImplicitCrossFamily,
	ECast_Implicit,
	ECast_Identitiy,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

err::CError
SetCastError (
	const CValue& OpValue,
	CType* pType
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// these are sometimes needed for inlining casts before jnc_Module.h is include

bool
CastOperator (
	CModule* pModule,
	const CValue& OpValue,
	CType* pType,
	CValue* pOpValue
	);

inline
bool
CastOperator (
	CModule* pModule,
	CValue* pOpValue,
	CType* pType
	)
{
	return CastOperator (pModule, *pOpValue, pType, pOpValue);
}

//.............................................................................

class CCastOperator
{	
	friend class COperatorMgr;

protected:
	CModule* m_pModule;
	uint_t m_OpFlags;

public:
	CCastOperator ();

	CModule*
	GetModule ()
	{
		return m_pModule;
	}

	int 
	GetOpFlags ()
	{
		return m_OpFlags;
	}

	virtual
	ECast
	GetCastKind (
		const CValue& OpValue,
		CType* pType
		) = 0;

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
		) = 0;

	bool
	Cast (
		EStorage StorageKind,
		const CValue& OpValue,
		CType* pType,
		CValue* pResultValue
		);
};

//.............................................................................

// fail by default

class CCast_Default: public CCastOperator
{
public:
	virtual
	ECast
	GetCastKind (
		const CValue& OpValue,
		CType* pType
		)
	{
		return ECast_None;
	}

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

// simple copy

class CCast_Copy: public CCastOperator
{
public:
	virtual
	ECast
	GetCastKind (
		const CValue& OpValue,
		CType* pType
		)
	{
		return OpValue.GetType ()->Cmp (pType) == 0 ? ECast_Identitiy : ECast_Implicit;
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

// master cast chooses particular implementation

class CCast_Master: public CCastOperator
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
		);

	virtual
	CCastOperator*
	GetCastOperator (
		const CValue& OpValue,
		CType* pType
		) = 0;
};

//.............................................................................

// master cast capable of performing superposition of casts

class CCast_SuperMaster: public CCastOperator
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
		);

	virtual
	bool
	GetCastOperators (
		const CValue& OpValue,
		CType* pType,
		CCastOperator** ppOperator1,
		CCastOperator** ppOperator2,
		CType** ppIntermediateType
		) = 0;
};

//.............................................................................

} // namespace jnc {

