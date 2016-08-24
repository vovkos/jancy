// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_DerivableType.h"
#include "jnc_ct_StructType.h"
#include "jnc_ct_Function.h"
#include "jnc_ct_Property.h"
#include "jnc_ct_UnOp.h"
#include "jnc_ct_BinOp.h"
#include "jnc_RuntimeStructs.h"
#include "jnc_ClassType.h"

namespace jnc {
namespace ct {

class ClassPtrType;
struct ClassPtrTypeTuple;

//............................................................................

class ClassType: public DerivableType
{
	friend class TypeMgr;
	friend class Parser;
	friend class Property;
	friend class StructType;

protected:
	ClassTypeKind m_classTypeKind;

	StructType* m_ifaceHdrStructType;
	StructType* m_ifaceStructType;
	StructType* m_classStructType;
	StructType* m_vtableStructType;

	sl::Array <BaseTypeSlot*> m_baseTypePrimeArray;
	sl::Array <StructField*> m_classMemberFieldArray;

	MarkOpaqueGcRootsFunc* m_markOpaqueGcRootsFunc;

	sl::Array <Function*> m_virtualMethodArray;
	sl::Array <Function*> m_overrideMethodArray;
	sl::Array <Property*> m_virtualPropertyArray;

	sl::Array <Function*> m_vtable;
	Variable* m_vtableVariable;

	ClassPtrTypeTuple* m_classPtrTypeTuple;

public:
	ClassType ();

	ClassTypeKind
	getClassTypeKind ()
	{
		return m_classTypeKind;
	}

	StructType* 
	getIfaceHdrStructType ()
	{
		ASSERT (m_ifaceHdrStructType);
		return m_ifaceHdrStructType;
	}

	DataPtrType* 
	getIfaceHdrPtrType ()
	{
		ASSERT (m_ifaceHdrStructType);
		return m_ifaceHdrStructType->getDataPtrType_c ();
	}

	StructType*
	getIfaceStructType ()
	{
		ASSERT (m_ifaceStructType);
		return m_ifaceStructType;
	}

	StructType*
	getClassStructType ()
	{
		ASSERT (m_classStructType);
		return m_classStructType;
	}

	ClassPtrType*
	getClassPtrType (
		Namespace* anchorNamespace,
		TypeKind typeKind,
		ClassPtrTypeKind ptrTypeKind = ClassPtrTypeKind_Normal,
		uint_t flags = 0
		);

	ClassPtrType*
	getClassPtrType (
		TypeKind typeKind,
		ClassPtrTypeKind ptrTypeKind = ClassPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return getClassPtrType (NULL, typeKind, ptrTypeKind, flags);
	}

	ClassPtrType*
	getClassPtrType (
		ClassPtrTypeKind ptrTypeKind = ClassPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return getClassPtrType (TypeKind_ClassPtr, ptrTypeKind, flags);
	}

	virtual
	Type*
	getThisArgType (uint_t ptrTypeFlags)
	{
		return (Type*) getClassPtrType (ClassPtrTypeKind_Normal, ptrTypeFlags);
	}

	MarkOpaqueGcRootsFunc*
	getMarkOpaqueGcRootsFunc ()
	{
		return m_markOpaqueGcRootsFunc;
	}

	virtual
	bool
	addMethod (Function* function);

	virtual
	bool
	addProperty (Property* prop);

	bool
	hasVTable ()
	{
		return !m_vtable.isEmpty ();
	}

	sl::Array <BaseTypeSlot*>
	getBaseTypePrimeArray ()
	{
		return m_baseTypePrimeArray;
	}

	sl::Array <StructField*>
	getClassMemberFieldArray ()
	{
		return m_classMemberFieldArray;
	}

	sl::Array <Function*>
	getVirtualMethodArray ()
	{
		return m_virtualMethodArray;
	}

	sl::Array <Property*>
	getVirtualPropertyArray ()
	{
		return m_virtualPropertyArray;
	}

	StructType*
	getVTableStructType ()
	{
		ASSERT (m_vtableStructType);
		return m_vtableStructType;
	}

	Variable*
	getVTableVariable ()
	{
		return m_vtableVariable;
	}

	virtual
	bool
	compile ();

	virtual
	void
	markGcRoots (
		const void* p,
		rt::GcHeap* gcHeap
		);

protected:
	void
	markGcRootsImpl (
		IfaceHdr* iface,
		rt::GcHeap* gcHeap
		);

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
	prepareLlvmType ()
	{
		m_llvmType = getClassStructType ()->getLlvmType ();
	}

	virtual
	void
	prepareLlvmDiType ()
	{
		m_llvmDiType = getClassStructType ()->getLlvmDiType ();
	}

	virtual
	bool
	calcLayout ();

	void
	addVirtualFunction (Function* function);

	bool
	overrideVirtualFunction (Function* function);

	void
	createVTableVariable ();
};

//.............................................................................

} // namespace ct
} // namespace jnc
