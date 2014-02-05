// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Type.h"
#include "jnc_Namespace.h"

namespace jnc {

class CFunctionType;
class CPropertyType;
class CStructType;
class CVariable;

//.............................................................................

class CNamedType:
	public CType,
	public CNamespace
{
	friend class CParser;

protected:
	CNamespace* m_pExtensionNamespace;

public:
	CNamedType ();

	CNamespace*
	GetExtensionNamespace ()
	{
		return m_pExtensionNamespace;
	}

	virtual
	CType*
	GetThisArgType (uint_t PtrTypeFlags)
	{
		return (CType*) GetDataPtrType (EDataPtrType_Normal, PtrTypeFlags);
	}

	CFunctionType*
	GetMemberMethodType (
		CFunctionType* pShortType,
		uint_t ThisArgTypeFlags = 0
		);

	CPropertyType*
	GetMemberPropertyType (CPropertyType* pShortType);

protected:
	virtual
	bool
	CalcLayout ();

	void
	ApplyExtensionNamespace ();

	virtual
	CModuleItem*
	FindItemTraverseImpl (
		const char* pName,
		CMemberCoord* pCoord,
		uint_t Flags
		);
};
//.............................................................................

} // namespace jnc {
