// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_StructType.h"

namespace jnc {

class UnionType;

//.............................................................................

// union cannot be a child, but it can be a parent

class UnionType: public DerivableType 
{
	friend class TypeMgr;
	friend class Parser;

protected:
	rtl::StdList <StructField> m_fieldList;
	rtl::Array <StructField*> m_fieldArray;
	StructField* m_initializedField;
	StructType* m_structType;
	
public:
	UnionType ();

	StructType*
	getStructType ()
	{
		ASSERT (m_structType);
		return m_structType;
	}

	rtl::ConstList <StructField>
	getFieldList ()
	{
		return m_fieldList;
	}

	virtual
	StructField*
	getFieldByIndex (size_t index);

	bool
	initializeField (const Value& thisValue);

	virtual
	bool
	compile ();

	virtual 
	void
	gcMark (
		Runtime* runtime,
		void* p
		)
	{
		ASSERT (false); // unions are POD and hence are never GC roots
	}

protected:
	virtual
	StructField*
	createFieldImpl (
		const rtl::String& name,
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0,
		rtl::BoxList <Token>* constructor = NULL,
		rtl::BoxList <Token>* initializer = NULL
		);

	virtual 
	void
	prepareTypeString ()
	{
		m_typeString.format ("union %s", m_tag.cc ()); // thanks a lot gcc
	}

	virtual 
	void
	prepareLlvmType ()
	{
		m_llvmType = getStructType ()->getLlvmType ();
	}
	
	virtual 
	void
	prepareLlvmDiType ();

	virtual
	bool
	calcLayout ();

	bool
	compileDefaultPreConstructor ();
};

//.............................................................................

} // namespace jnc {
