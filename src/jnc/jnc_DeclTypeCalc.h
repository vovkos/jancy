// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Decl.h"

namespace jnc {

class CArrayType;
class CDataPtrType;
class CClassType;
class CClassPtrType;
class CFunctionType;
class CFunctionPtrType;
class CPropertyType;
class CPropertyPtrType;
class CReactorClassType;
class CImportType;
class CImportPtrType;

//.............................................................................

class CDeclTypeCalc: protected CTypeModifiers
{
protected:
	CModule* m_pModule;
	rtl::CIteratorT <CDeclSuffix> m_Suffix;

public:
	CDeclTypeCalc ()
	{
		m_pModule = NULL;
	}

	CType*
	CalcType (
		CDeclarator* pDeclarator,
		CValue* pElementCountValue,
		uint_t* pFlags
		)
	{
		return CalcType (
			pDeclarator->GetBaseType (),
			pDeclarator,
			pDeclarator->GetPointerPrefixList (),
			pDeclarator->GetSuffixList (),
			pElementCountValue,
			pFlags
			);
	}

	CType*
	CalcType (
		CType* pBaseType,
		CTypeModifiers* pTypeModifiers,
		const rtl::CConstListT <CDeclPointerPrefix>& PointerPrefixList,
		const rtl::CConstListT <CDeclSuffix>& SuffixList,
		CValue* pElementCountValue,
		uint_t* pFlags
		);

	CType*
	CalcPtrType (
		CType* pType,
		uint_t TypeModifiers
		);

	CType*
	CalcIntModType (
		CType* pType,
		uint_t TypeModifiers
		);

	CFunctionType*
	CalcPropertyGetterType (CDeclarator* pDeclarator);

protected:
	bool
	CheckUnusedModifiers ();

	bool
	GetPtrTypeFlags (
		CType* pType,
		uint_t* pFlags
		);

	uint_t
	GetPropertyFlags ();

	CType*
	GetIntegerType (CType* pType);

	CArrayType*
	GetArrayType (CType* pElementType);

	CFunctionType*
	GetFunctionType (CType* pReturnType);

	CPropertyType*
	GetPropertyType (CType* pReturnType);

	CPropertyType*
	GetBindableDataType (CType* pDataType);

	CClassType*
	GetReactorType (CType* pReturnType);

	CClassType*
	GetMulticastType (CType* pLeftType);

	CDataPtrType*
	GetDataPtrType (CType* pDataType);

	CClassPtrType*
	GetClassPtrType (CClassType* pClassType);

	CFunctionPtrType*
	GetFunctionPtrType (CFunctionType* pFunctionType);

	CPropertyPtrType*
	GetPropertyPtrType (CPropertyType* pPropertyType);

	CImportPtrType*
	GetImportPtrType (CNamedImportType* pImportType);

	CImportIntModType*
	GetImportIntModType (CNamedImportType* pImportType);

	CType*
	PrepareReturnType (CType* pType);
};

//.............................................................................

} // namespace jnc {
