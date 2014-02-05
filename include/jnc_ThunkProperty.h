// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Property.h"

namespace jnc {

//.............................................................................

class CThunkProperty: public CProperty
{
	friend class CFunctionMgr;

protected:
	rtl::CString m_Signature;
	CProperty* m_pTargetProperty;

public:
	CThunkProperty ();

	bool
	Create (
		CProperty* pTargetProperty,
		CPropertyType* pThunkPropertyType,
		bool HasUnusedClosure
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CDataThunkProperty: public CProperty
{
	friend class CFunctionMgr;

protected:
	rtl::CString m_Signature;
	CVariable* m_pTargetVariable;

public:
	CDataThunkProperty ();

	virtual
	bool 
	Compile ();

protected:
	bool 
	CompileGetter ();

	bool 
	CompileSetter (CFunction* pSetter);
};

//.............................................................................

} // namespace jnc {
