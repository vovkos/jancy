// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ClassType.h"

namespace jnc {

//.............................................................................

class ClassPtrType: public Type
{
	friend class TypeMgr;

protected:
	ClassPtrTypeKind m_ptrTypeKind;
	ClassType* m_targetType;
	Namespace* m_anchorNamespace; // for dual pointers

public:
	ClassPtrType ();

	ClassPtrTypeKind
	getPtrTypeKind ()
	{
		return m_ptrTypeKind;
	}

	ClassType*
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

	bool
	isEventPtrType ();

	ClassPtrType*
	getCheckedPtrType ()
	{
		return !(m_flags & PtrTypeFlagKind_Safe) ?
			m_targetType->getClassPtrType (m_typeKind, m_ptrTypeKind, m_flags | PtrTypeFlagKind_Safe) :
			this;
	}

	ClassPtrType*
	getUnCheckedPtrType ()
	{
		return (m_flags & PtrTypeFlagKind_Safe) ?
			m_targetType->getClassPtrType (m_typeKind, m_ptrTypeKind, m_flags & ~PtrTypeFlagKind_Safe) :
			this;
	}

	ClassPtrType*
	getUnConstPtrType ()
	{
		return (m_flags & PtrTypeFlagKind_Const) ?
			m_targetType->getClassPtrType (m_typeKind, m_ptrTypeKind, m_flags & ~PtrTypeFlagKind_Const) :
			this;
	}

	ClassPtrType*
	getNormalPtrType ()
	{
		return (m_ptrTypeKind != ClassPtrTypeKind_Normal) ?
			m_targetType->getClassPtrType (ClassPtrTypeKind_Normal, m_flags) :
			this;
	}

	ClassPtrType*
	getWeakPtrType ()
	{
		return (m_ptrTypeKind != ClassPtrTypeKind_Weak) ?
			m_targetType->getClassPtrType (ClassPtrTypeKind_Weak, m_flags) :
			this;
	}

	ClassPtrType*
	getUnWeakPtrType ()
	{
		return (m_ptrTypeKind == ClassPtrTypeKind_Weak) ?
			m_targetType->getClassPtrType (ClassPtrTypeKind_Normal, m_flags) :
			this;
	}

	static
	rtl::String
	createSignature (
		ClassType* classType,
		TypeKind typeKind,
		ClassPtrTypeKind ptrTypeKind,
		uint_t flags
		);

	virtual
	void
	gcMark (
		Runtime* runtime,
		void* p
		);

protected:
	virtual
	void
	prepareTypeString ();

	virtual
	void
	prepareLlvmType ();

	virtual
	void
	prepareLlvmDiType ();
};

//.............................................................................

struct ClassPtrTypeTuple: rtl::ListLink
{
	ClassPtrType* m_ptrTypeArray [2] [2] [2] [2] [2]; // ref x kind x const x volatile x checked
};

//.............................................................................

inline
bool
isClassPtrType (
	Type* type,
	ClassTypeKind classTypeKind
	)
{
	return
		(type->getTypeKindFlags () & TypeKindFlagKind_ClassPtr) &&
		((ClassPtrType*) type)->getTargetType ()->getClassTypeKind () == classTypeKind;
}

//.............................................................................

} // namespace jnc {
