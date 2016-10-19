// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_Type.h"

namespace jnc {
namespace ct {

//..............................................................................

class DataPtrType: public Type
{
	friend class TypeMgr;

protected:
	DataPtrTypeKind m_ptrTypeKind;
	Type* m_targetType;
	Namespace* m_anchorNamespace; // for dual pointers

public:
	DataPtrType ();

	DataPtrTypeKind
	getPtrTypeKind ()
	{
		return m_ptrTypeKind;
	}

	Type*
	getTargetType ()
	{
		return m_targetType;
	}

	Namespace*
	getAnchorNamespace ()
	{
		return m_anchorNamespace;
	}

	bool
	isConstPtrType ();

	DataPtrType*
	getCheckedPtrType ()
	{
		return !(m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getDataPtrType (m_typeKind, m_ptrTypeKind, m_flags | PtrTypeFlag_Safe) :
			this;
	}

	DataPtrType*
	getUnCheckedPtrType ()
	{
		return (m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getDataPtrType (m_typeKind, m_ptrTypeKind, m_flags & ~PtrTypeFlag_Safe) :
			this;
	}

	DataPtrType*
	getUnConstPtrType ()
	{
		return (m_flags & PtrTypeFlag_Const) ?
			m_targetType->getDataPtrType (m_typeKind, m_ptrTypeKind, m_flags & ~PtrTypeFlag_Const) :
			this;
	}

	static
	sl::String
	createSignature (
		Type* baseType,
		TypeKind typeKind,
		DataPtrTypeKind ptrTypeKind,
		uint_t flags
		);

	virtual
	void
	markGcRoots (
		const void* p,
		rt::GcHeap* gcHeap
		);

protected:
	virtual
	void
	prepareTypeString ();

	virtual
	void
	prepareDoxyLinkedText ();

	virtual
	void
	prepareLlvmType ();

	virtual
	void
	prepareLlvmDiType ();

	sl::String
	getPointerStringSuffix ();
};

//..............................................................................

struct DataPtrTypeTuple: sl::ListLink
{
	DataPtrType* m_ptrTypeArray [2] [3] [2] [2] [2]; // ref x kind x const x volatile x safe
};

//..............................................................................

} // namespace ct
} // namespace jnc
