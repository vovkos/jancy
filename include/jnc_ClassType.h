// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_DerivableType.h"
#include "jnc_StructType.h"
#include "jnc_Function.h"
#include "jnc_Property.h"
#include "jnc_UnOp.h"
#include "jnc_BinOp.h"
#include "jnc_Box.h"

namespace jnc {

class ClassPtrType;
struct ClassPtrTypeTuple;
struct Box;
struct IfaceHdr;

//.............................................................................

enum ClassTypeKind
{
	ClassTypeKind_Normal = 0,
	ClassTypeKind_StdObject, // StdType_Object
	ClassTypeKind_Multicast,
	ClassTypeKind_McSnapshot,
	ClassTypeKind_Reactor,
	ClassTypeKind_ReactorIface,
	ClassTypeKind_FunctionClosure,
	ClassTypeKind_PropertyClosure,
	ClassTypeKind_DataClosure,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum ClassTypeFlag
{
	ClassTypeFlag_Abstract = 0x010000,
	ClassTypeFlag_Opaque   = 0x020000,
};

//.............................................................................

enum ClassPtrTypeKind
{
	ClassPtrTypeKind_Normal = 0,
	ClassPtrTypeKind_Weak,
	ClassPtrTypeKind__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getClassPtrTypeKindString (ClassPtrTypeKind ptrTypeKind);

//.............................................................................

typedef
void
ClassTypeGcRootEnumProc (
	Runtime* runtime,
	IfaceHdr* iface
	);

//............................................................................

class ClassType: public DerivableType
{
	friend class TypeMgr;
	friend class Parser;
	friend class Property;

protected:
	ClassTypeKind m_classTypeKind;

	StructType* m_ifaceHdrStructType;
	StructType* m_ifaceStructType;
	StructType* m_classStructType;
	StructType* m_vtableStructType;

	rtl::Array <BaseTypeSlot*> m_baseTypePrimeArray;
	rtl::Array <StructField*> m_classMemberFieldArray;

	Function* m_primer;
	Function* m_operatorNew;
	ClassTypeGcRootEnumProc* m_gcRootEnumProc;

	rtl::Array <Function*> m_virtualMethodArray;
	rtl::Array <Function*> m_overrideMethodArray;
	rtl::Array <Property*> m_virtualPropertyArray;

	rtl::Array <Function*> m_vtable;
	Value m_vtablePtrValue;

	ClassPtrTypeTuple* m_classPtrTypeTuple;

public:
	ClassType ();

	bool
	isCreatable ()
	{
		return
			m_classTypeKind != ClassTypeKind_StdObject &&
			!(m_flags & (ClassTypeFlag_Abstract | ClassTypeFlag_Opaque));
	}

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

	Function*
	getPrimer ()
	{
		return m_primer;
	}

	Function*
	getOperatorNew ()
	{
		return m_operatorNew;
	}

	ClassTypeGcRootEnumProc*
	getGcRootEnumProc ()
	{
		return m_gcRootEnumProc;
	}

	void
	setupOpaqueClass (
		size_t size,
		ClassTypeGcRootEnumProc* gcRootEnumProc
		);

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

	rtl::Array <BaseTypeSlot*>
	getBaseTypePrimeArray ()
	{
		return m_baseTypePrimeArray;
	}

	rtl::Array <StructField*>
	getClassMemberFieldArray ()
	{
		return m_classMemberFieldArray;
	}

	rtl::Array <Function*>
	getVirtualMethodArray ()
	{
		return m_virtualMethodArray;
	}

	rtl::Array <Property*>
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

	Value
	getVTablePtrValue ()
	{
		return m_vtablePtrValue;
	}

	virtual
	bool
	compile ();

	virtual
	void
	gcMark (
		Runtime* runtime,
		void* p
		);

protected:
	void
	enumGcRootsImpl (
		Runtime* runtime,
		IfaceHdr* iface
		);

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
		m_typeString.format ("class %s", m_tag.cc ());
	}

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
	createVTablePtr ();

	void
	createPrimer ();

	void
	primeObject (
		ClassType* classType,
		const Value& opValue,
		const Value& rootValue,
		const Value& flagsValue
		);

	void
	primeInterface (
		ClassType* classType,
		const Value& opValue,
		const Value& VTableValue,
		const Value& objectValue,
		const Value& rootValue,
		const Value& flagsValue
		);

	bool
	compilePrimer ();
};

//.............................................................................

inline
bool
isClassType (
	Type* type,
	ClassTypeKind classTypeKind
	)
{
	return
		type->getTypeKind () == TypeKind_Class &&
		((ClassType*) type)->getClassTypeKind () == classTypeKind;
}

inline
bool
isOpaqueClassType (Type* type)
{
	return
		type->getTypeKind () == TypeKind_Class &&
		(type->getFlags () & ClassTypeFlag_Opaque);
}

//.............................................................................

// header of class iface

struct IfaceHdr
{
	void* m_vtable;
	Box* m_object; // back pointer to master header

	// followed by parents, then by iface data fields
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// iface with master header

template <typename T>
class ClassBox:
	public Box,
	public T
{
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef
void
Class_Prime (
	Box* object,
	Box* root,
	uintptr_t flags
	);

typedef
void
Class_Construct (IfaceHdr* iface);

typedef
void
Class_Destruct (IfaceHdr* iface);

//.............................................................................

} // namespace jnc {
