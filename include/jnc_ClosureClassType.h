// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ClassType.h"
#include "jnc_ThunkFunction.h"
#include "jnc_Closure.h"

namespace jnc {

//.............................................................................

class CClosureClassType: public CClassType
{
	friend class CTypeMgr;

protected:
	uint64_t m_WeakMask;

public: // tmp
	rtl::CArrayT <size_t> m_ClosureMap;

public:
	CClosureClassType ()
	{
		m_WeakMask = 0;
	}

	uint64_t 
	GetWeakMask ()
	{
		return m_WeakMask;
	}

	static
	rtl::CString
	CreateSignature (
		CType* pTargetType, // function or property
		CType* pThunkType, // function or property
		CType* const* ppArgTypeArray,
		const size_t* pClosureMap,
		size_t ClosureArgCount,
		uint64_t WeakMask
		);

	virtual 
	bool
	Compile () = 0;

	jnc::TIfaceHdr* 
	Strengthen (jnc::TIfaceHdr* p);

protected:
	void
	BuildArgValueList (
		const CValue& ClosureValue,
		const CValue* pThunkArgValueArray,
		size_t ThunkArgCount,
		rtl::CBoxListT <CValue>* pArgValueList
		);
};

//.............................................................................

class CFunctionClosureClassType: public CClosureClassType
{
	friend class CTypeMgr;

protected:
	CFunction* m_pThunkFunction;

public:
	CFunctionClosureClassType ();

	CFunction*
	GetThunkFunction ()
	{
		return m_pThunkFunction;
	}

	virtual 
	bool
	Compile ();
};

//.............................................................................

class CPropertyClosureClassType: public CClosureClassType
{
	friend class CTypeMgr;

protected:
	CProperty* m_pThunkProperty;

public:
	CPropertyClosureClassType ();

	CProperty*
	GetThunkProperty ()
	{
		return m_pThunkProperty;
	}

	virtual 
	bool
	Compile ();

protected:
	bool
	CompileAccessor (CFunction* pAccessor);
};

//.............................................................................

class CDataClosureClassType: public CClassType
{
	friend class CTypeMgr;

protected:
	CProperty* m_pThunkProperty;

public:
	CDataClosureClassType ();

	CProperty*
	GetThunkProperty ()
	{
		return m_pThunkProperty;
	}

	static
	rtl::CString
	CreateSignature (
		CType* pTargetType,
		CPropertyType* pThunkType
		);

	virtual 
	bool
	Compile ();

protected:
	bool
	CompileGetter (CFunction* pGetter);

	bool
	CompileSetter (CFunction* pSetter);
};

//.............................................................................

} // namespace jnc {
