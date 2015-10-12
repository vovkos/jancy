// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_StructType.h"

namespace jnc {
namespace ct {

class UnionType;

//.............................................................................

// union cannot be a child, but it can be a parent

class UnionType: public DerivableType 
{
	friend class TypeMgr;
	friend class Parser;

protected:
	StructType* m_structType;
	
public:
	UnionType ();

	StructType*
	getStructType ()
	{
		ASSERT (m_structType);
		return m_structType;
	}

	virtual
	bool
	compile ();

	virtual 
	void
	markGcRoots (
		const void* p,
		rt::GcHeap* gcHeap
		)
	{
		ASSERT (false); // unions are POD and hence are never GC roots
	}

protected:
	virtual
	StructField*
	createFieldImpl (
		const sl::String& name,
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0,
		sl::BoxList <Token>* constructor = NULL,
		sl::BoxList <Token>* initializer = NULL
		);

	virtual 
	void
	prepareTypeString ()
	{
		m_typeString.format ("union %s", m_tag.cc ());
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
};

//.............................................................................

} // namespace ct
} // namespace jnc
