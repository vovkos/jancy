// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ImportType.h"
#include "jnc_Scope.h"

namespace jnc {

class CScope;

//.............................................................................

class CFunctionArg: 
	public CUserModuleItem,
	public CModuleItemInitializer
{
	friend class CTypeMgr;
	friend class CFunction;
	friend class CClassType;
	friend class COrphan;

protected:
	CType* m_pType;
	CImportType* m_pType_i;
	uint_t m_PtrTypeFlags;

public:
	CFunctionArg ();

	CType*
	GetType ()
	{
		return m_pType;
	}

	CImportType*
	GetType_i ()
	{
		return m_pType_i;
	}

	uint_t
	GetPtrTypeFlags ()
	{
		return m_PtrTypeFlags;
	}

protected:
	virtual
	bool
	CalcLayout ();
};

//.............................................................................

struct TFunctionArgTuple: rtl::TListLink
{
	CFunctionArg* m_ArgArray [2] [2] [2]; // this x const x volatile
};

//.............................................................................

} // namespace jnc {
