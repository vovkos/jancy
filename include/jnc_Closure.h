// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Value.h"

namespace jnc {

class CFunctionType;
class CFunctionPtrType;
class CPropertyType;
class CPropertyPtrType;

//.............................................................................

class CClosure: public ref::CRefCount
{
	friend class CValue;

protected:
	rtl::CBoxListT <CValue> m_ArgValueList;

public:
	rtl::CBoxListT <CValue>*
	GetArgValueList ()
	{
		return &m_ArgValueList;
	}

	bool
	IsMemberClosure ()
	{
		return
			!m_ArgValueList.IsEmpty () &&
			!m_ArgValueList.GetHead ()->IsEmpty () &&
			m_ArgValueList.GetHead ()->GetType ()->GetTypeKind () == EType_ClassPtr;
	}

	CValue
	GetThisValue ()
	{
		ASSERT (IsMemberClosure ());
		return *m_ArgValueList.GetHead ();
	}

	bool
	IsSimpleClosure ()
	{
		return IsMemberClosure () && m_ArgValueList.GetCount () == 1;
	}

	size_t
	Append (const rtl::CConstBoxListT <CValue>& ArgValueList);

	bool
	Apply (rtl::CBoxListT <CValue>* pArgValueList);

	CType*
	GetClosureType (CType* pType);

	CFunctionPtrType*
	GetFunctionClosureType (CFunction* pFunction); // choose the best overload

	CFunctionPtrType*
	GetFunctionClosureType (CFunctionPtrType* pPtrType);

	CPropertyPtrType*
	GetPropertyClosureType (CPropertyPtrType* pPtrType);

protected:
	bool
	GetArgTypeArray (
		CModule* pModule,
		rtl::CArrayT <CFunctionArg*>* pArgArray
		);
};

//.............................................................................

} // namespace jnc {
