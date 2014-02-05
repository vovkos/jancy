// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_StructType.h"

namespace jnc {

class CUnionType;

//.............................................................................

// union cannot be a child, but it can be a parent

class CUnionType: public CDerivableType 
{
	friend class CTypeMgr;
	friend class CParser;

protected:
	rtl::CStdListT <CStructField> m_FieldList;
	rtl::CArrayT <CStructField*> m_FieldArray;
	CStructField* m_pInitializedField;
	CStructType* m_pStructType;
	
public:
	CUnionType ();

	CStructType*
	GetStructType ()
	{
		ASSERT (m_pStructType);
		return m_pStructType;
	}

	rtl::CConstListT <CStructField>
	GetFieldList ()
	{
		return m_FieldList;
	}

	virtual
	CStructField*
	GetFieldByIndex (size_t Index);

	bool
	InitializeField (const CValue& ThisValue);

	virtual
	bool
	Compile ();

	virtual 
	void
	GcMark (
		CRuntime* pRuntime,
		void* p
		)
	{
		ASSERT (false); // unions are POD and hence are never GC roots
	}

protected:
	virtual
	CStructField*
	CreateFieldImpl (
		const rtl::CString& Name,
		CType* pType,
		size_t BitCount = 0,
		uint_t PtrTypeFlags = 0,
		rtl::CBoxListT <CToken>* pConstructor = NULL,
		rtl::CBoxListT <CToken>* pInitializer = NULL
		);

	virtual 
	void
	PrepareTypeString ()
	{
		m_TypeString.Format ("union %s", m_Tag.cc ()); // thanks a lot gcc
	}

	virtual 
	void
	PrepareLlvmType ()
	{
		m_pLlvmType = GetStructType ()->GetLlvmType ();
	}
	
	virtual 
	void
	PrepareLlvmDiType ();

	virtual
	bool
	CalcLayout ();

	bool
	CompileDefaultPreConstructor ();
};

//.............................................................................

} // namespace jnc {
