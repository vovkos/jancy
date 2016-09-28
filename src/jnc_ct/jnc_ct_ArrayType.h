// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_ImportType.h"
#include "jnc_ct_DataPtrType.h"
#include "jnc_ArrayType.h"

namespace jnc {
namespace ct {

//.............................................................................

class ArrayType: public Type
{
	friend class TypeMgr;
	friend class Parser;

protected:
	Type* m_elementType;
	Type* m_rootType;
	size_t m_elementCount;

	sl::BoxList <Token> m_elementCountInitializer;
	Unit* m_parentUnit;
	Namespace* m_parentNamespace;

public:
	ArrayType ();

	Type*
	getElementType ()
	{
		return m_elementType;
	}

	Type*
	getRootType ();

	size_t
	getElementCount ()
	{
		return m_elementCount;
	}

	sl::ConstBoxList <Token>
	getElementCountInitializer ()
	{
		return m_elementCountInitializer;
	}

	static
	sl::String
	createSignature (
		Type* elementType,
		size_t elementCount
		)
	{
		return sl::String::format_s (
			"A%d%s",
			elementCount,
			elementType->getSignature ().cc ()
			);
	}

	virtual
	void
	markGcRoots (
		const void* p,
		rt::GcHeap* gcHeap
		);

protected:
	virtual
	bool
	calcLayout ();
	
	virtual
	void
	prepareTypeString ();

	virtual
	void
	prepareDoxyLinkedText ();

	sl::String
	createDimensionString ();

	virtual
	void
	prepareLlvmType ()
	{
		ASSERT (m_elementCount != -1);
		m_llvmType = llvm::ArrayType::get (m_elementType->getLlvmType (), m_elementCount);
	}

	virtual
	void
	prepareLlvmDiType ();
};

//.............................................................................

} // namespace ct
} // namespace jnc
