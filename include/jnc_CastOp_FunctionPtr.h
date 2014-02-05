// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_CastOp.h"
#include "jnc_FunctionPtrType.h"

namespace jnc {

//.............................................................................

class CCast_FunctionPtr_FromMulticast: public CCastOperator
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
};

//.............................................................................

class CCast_FunctionPtr_Base: public CCastOperator
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

class CCast_FunctionPtr_FromFat: public CCast_FunctionPtr_Base
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

class CCast_FunctionPtr_Weak2Normal: public CCast_FunctionPtr_Base
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

class CCast_FunctionPtr_Thin2Fat: public CCast_FunctionPtr_Base
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
		CFunctionType* pSrcFunctionType,
		CFunctionPtrType* pDstPtrType,
		CValue* pResultValue
		);

	bool
	LlvmCast_DirectThunkNoClosure (
		CFunction* pFunction,
		CFunctionPtrType* pDstPtrType,
		CValue* pResultValue
		);

	bool
	LlvmCast_DirectThunkSimpleClosure (
		CFunction* pFunction,
		const CValue& SimpleClosureObjValue,
		CFunctionPtrType* pDstPtrType,
		CValue* pResultValue
		);

	bool
	LlvmCast_FullClosure (
		EStorage StorageKind,
		const CValue& OpValue,
		CFunctionType* pSrcFunctionType,
		CFunctionPtrType* pDstPtrType,
		CValue* pResultValue
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CCast_FunctionPtr_Thin2Thin: public CCast_FunctionPtr_Base
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

class CCast_FunctionPtr: public CCast_Master
{
protected:
	CCast_FunctionPtr_FromMulticast m_FromMulticast;
	CCast_FunctionPtr_FromFat m_FromFat;
	CCast_FunctionPtr_Weak2Normal m_Weak2Normal;
	CCast_FunctionPtr_Thin2Fat m_Thin2Fat;
	CCast_FunctionPtr_Thin2Thin m_Thin2Thin;

	CCastOperator* m_OperatorTable [EFunctionPtrType__Count] [EFunctionPtrType__Count];

public:
	CCast_FunctionPtr ();

	virtual
	CCastOperator*
	GetCastOperator (
		const CValue& OpValue,
		CType* pType
		);
};

//.............................................................................

// function ref (EUnOp_Indir => function ptr cast => EUnOp_Addr)

class CCast_FunctionRef: public CCastOperator
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
};

//.............................................................................

} // namespace jnc {
