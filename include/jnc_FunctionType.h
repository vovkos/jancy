// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_CallConv.h"
#include "jnc_FunctionArg.h"

namespace jnc {

class CFunctionPtrType;
class CNamedType;
class CClassType;
class CClassPtrType;
class CReactorClassType;
class CCdeclCallConv_msc64;
class CFunction;

struct TFunctionPtrTypeTuple;

//.............................................................................

enum EFunctionTypeFlag
{
	EFunctionTypeFlag_VarArg      = 0x010000,
	EFunctionTypeFlag_Throws      = 0x040000,
	EFunctionTypeFlag_CoercedArgs = 0x080000,
};

//.............................................................................

enum EFunctionPtrType
{
	EFunctionPtrType_Normal = 0,
	EFunctionPtrType_Weak,
	EFunctionPtrType_Thin,
	EFunctionPtrType__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
GetFunctionPtrTypeKindString (EFunctionPtrType PtrTypeKind);

//.............................................................................

class CFunctionType: public CType
{
	friend class CTypeMgr;
	friend class CClassType;
	friend class CCdeclCallConv_msc64;
	friend class CCdeclCallConv_gcc64;

protected:
	CCallConv* m_pCallConv;
	CType* m_pReturnType;
	CImportType* m_pReturnType_i;
	rtl::CArrayT <CFunctionArg*> m_ArgArray;
	rtl::CBoxListT <CToken> m_ThrowCondition;
	rtl::CString m_ArgSignature;
	rtl::CString m_TypeModifierString;
	rtl::CString m_ArgString;
	CFunctionType* m_pShortType;
	CFunctionType* m_pStdObjectMemberMethodType;
	CFunction* m_pAbstractFunction;
	TFunctionPtrTypeTuple* m_pFunctionPtrTypeTuple;
	CReactorClassType* m_pReactorInterfaceType;

public:
	CFunctionType ();

	CCallConv*
	GetCallConv ()
	{
		return m_pCallConv;
	}

	CType*
	GetReturnType ()
	{
		return m_pReturnType;
	}

	CImportType*
	GetReturnType_i ()
	{
		return m_pReturnType_i;
	}

	rtl::CConstBoxListT <CToken>
	GetThrowCondition ()
	{
		return m_ThrowCondition;
	}

	bool
	IsThrowConditionMatch (CFunctionType* pType);

	rtl::CArrayT <CFunctionArg*>
	GetArgArray ()
	{
		return m_ArgArray;
	}

	rtl::CString
	GetArgSignature ();

	rtl::CString
	GetArgString ();

	rtl::CString
	GetTypeModifierString ();

	bool
	IsMemberMethodType ()
	{
		return !m_ArgArray.IsEmpty () && m_ArgArray [0]->GetStorageKind () == EStorage_This;
	}

	CType*
	GetThisArgType ()
	{
		return IsMemberMethodType () ? m_ArgArray [0]->GetType () : NULL;
	}

	CNamedType*
	GetThisTargetType ();

	CFunctionType*
	GetShortType ()
	{
		return m_pShortType;
	}

	CFunctionType*
	GetMemberMethodType (
		CNamedType* pType,
		uint_t ThisArgFlags = 0
		);

	CFunctionType*
	GetStdObjectMemberMethodType ();

	CFunction*
	GetAbstractFunction ();

	CFunctionPtrType*
	GetFunctionPtrType (
		EType TypeKind,
		EFunctionPtrType PtrTypeKind = EFunctionPtrType_Normal,
		uint_t Flags = 0
		);

	CFunctionPtrType*
	GetFunctionPtrType (
		EFunctionPtrType PtrTypeKind = EFunctionPtrType_Normal,
		uint_t Flags = 0
		)
	{
		return GetFunctionPtrType (EType_FunctionPtr, PtrTypeKind, Flags);
	}

	CClassType*
	GetMulticastType ();

	static
	rtl::CString
	CreateSignature (
		CCallConv* pCallConv,
		CType* pReturnType,
		CType* const* pArgTypeArray,
		size_t ArgCount,
		uint_t Flags
		);

	static
	rtl::CString
	CreateSignature (
		CCallConv* pCallConv,
		CType* pReturnType,
		CFunctionArg* const* pArgArray,
		size_t ArgCount,
		uint_t Flags
		);

	static
	rtl::CString
	CreateArgSignature (
		CType* const* pArgTypeArray,
		size_t ArgCount,
		uint_t Flags
		);

	static
	rtl::CString
	CreateArgSignature (
		CFunctionArg* const* pArgArray,
		size_t ArgCount,
		uint_t Flags
		);

	rtl::CString
	CreateArgSignature ()
	{
		return CreateArgSignature (m_ArgArray, m_ArgArray.GetCount (), m_Flags);
	}

	virtual
	bool
	Compile ();

protected:
	virtual
	void
	PrepareTypeString ();

	virtual
	void
	PrepareLlvmType ();

	virtual
	void
	PrepareLlvmDiType ();

	virtual
	bool
	CalcLayout ();
};

//.............................................................................

} // namespace jnc {
