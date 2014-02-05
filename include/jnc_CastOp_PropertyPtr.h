// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_CastOp.h"
#include "jnc_PropertyPtrType.h"

namespace jnc {

//.............................................................................

class CCast_PropertyPtr_FromDataPtr: public CCastOperator
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
	LlvmCast (
		EStorage StorageKind,
		const CValue& OpValue,
		CType* pType,
		CValue* pResultValue
		);

protected:
	bool
	LlvmCast_DirectThunk (
		CVariable* pVariable,
		CPropertyPtrType* pDstPtrType,
		CValue* pResultValue
		);

	bool
	LlvmCast_FullClosure (
		EStorage StorageKind,
		const CValue& OpValue,
		CPropertyPtrType* pDstPtrType,
		CValue* pResultValue
		);
};

//.............................................................................

class CCast_PropertyPtr_Base: public CCastOperator
{
public:
	virtual
	ECast
	GetCastKind (
		const CValue& OpValue,
		CType* pType
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CCast_PropertyPtr_FromFat: public CCast_PropertyPtr_Base
{
public:
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

class CCast_PropertyPtr_Thin2Fat: public CCast_PropertyPtr_Base
{
public:
	virtual
	bool
	LlvmCast (
		EStorage StorageKind,
		const CValue& OpValue,
		CType* pType,
		CValue* pResultValue
		);

protected:
	bool
	LlvmCast_NoThunkSimpleClosure (
		const CValue& OpValue,
		const CValue& SimpleClosureObjValue,
		CPropertyType* pSrcPropertyType,
		CPropertyPtrType* pDstPtrType,
		CValue* pResultValue
		);

	bool
	LlvmCast_DirectThunkNoClosure (
		CProperty* pProperty,
		CPropertyPtrType* pDstPtrType,
		CValue* pResultValue
		);

	bool
	LlvmCast_DirectThunkSimpleClosure (
		CProperty* pProperty,
		const CValue& SimpleClosureObjValue,
		CPropertyPtrType* pDstPtrType,
		CValue* pResultValue
		);

	bool
	LlvmCast_FullClosure (
		EStorage StorageKind,
		const CValue& OpValue,
		CPropertyType* pSrcPropertyType,
		CPropertyPtrType* pDstPtrType,
		CValue* pResultValue
		);

	bool
	CreateClosurePropertyPtr (
		CProperty* pProperty,
		const CValue& ClosureValue,
		CPropertyPtrType* pPtrType,
		CValue* pResultValue
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CCast_PropertyPtr_Weak2Normal: public CCast_PropertyPtr_Base
{
public:
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

class CCast_PropertyPtr_Thin2Thin: public CCast_PropertyPtr_Base
{
public:
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

// safe / unsafe fn pointer -> safe fn pointer

class CCast_PropertyPtr: public CCast_Master
{
protected:
	CCast_PropertyPtr_FromDataPtr m_FromDataPtr;
	CCast_PropertyPtr_FromFat m_FromFat;
	CCast_PropertyPtr_Weak2Normal m_Weak2Normal;
	CCast_PropertyPtr_Thin2Fat m_Thin2Fat;
	CCast_PropertyPtr_Thin2Thin m_Thin2Thin;

	CCastOperator* m_OperatorTable [EPropertyPtrType__Count] [EPropertyPtrType__Count];

public:
	CCast_PropertyPtr ();

	virtual
	CCastOperator*
	GetCastOperator (
		const CValue& OpValue,
		CType* pType
		);
};

//.............................................................................

// data ref (EUnOp_Indir => data ptr cast => EUnOp_Addr)

class CCast_PropertyRef: public CCastOperator
{
public:
	CCast_PropertyRef ()
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
