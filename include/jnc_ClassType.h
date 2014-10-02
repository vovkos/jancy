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

namespace jnc {

class ClassPtrType;
struct ClassPtrTypeTuple;
struct ObjHdr;
struct IfaceHdr;

//.............................................................................

enum ClassTypeKind
{
	ClassTypeKind_Normal = 0,
	ClassTypeKind_StdObject, // EStdType_Object
	ClassTypeKind_Box,
	ClassTypeKind_Multicast,
	ClassTypeKind_McSnapshot,
	ClassTypeKind_Reactor,
	ClassTypeKind_ReactorIface,
	ClassTypeKind_FunctionClosure,
	ClassTypeKind_PropertyClosure,
	ClassTypeKind_DataClosure,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum ClassTypeFlagKind
{
	ClassTypeFlagKind_Abstract = 0x010000,
	ClassTypeFlagKind_Opaque   = 0x020000,
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

//............................................................................

class ClassType: public DerivableType
{
	friend class TypeMgr;
	friend class Parser;
	friend class Property;

protected:
	ClassTypeKind m_classTypeKind;

	StructType* m_ifaceStructType;
	StructType* m_classStructType;

	Function* m_primer;
	Function* m_destructor;
	Function* m_operatorNew;

	// prime arrays

	rtl::Array <BaseTypeSlot*> m_baseTypePrimeArray;
	rtl::Array <StructField*> m_classMemberFieldArray;

	// destruct arrays

	rtl::Array <ClassType*> m_baseTypeDestructArray;
	rtl::Array <StructField*> m_memberFieldDestructArray;
	rtl::Array <Property*> m_memberPropertyDestructArray;

	// vtable

	rtl::Array <Function*> m_virtualMethodArray;
	rtl::Array <Function*> m_overrideMethodArray;
	rtl::Array <Property*> m_virtualPropertyArray;

	StructType* m_pVTableStructType;
	rtl::Array <Function*> m_VTable;
	Value m_VTablePtrValue;

	ClassPtrTypeTuple* m_classPtrTypeTuple;

public:
	ClassType ();

	bool
	isCreatable ()
	{
		return
			m_classTypeKind != ClassTypeKind_StdObject &&
			!(m_flags & (ClassTypeFlagKind_Abstract | ClassTypeFlagKind_Opaque));
	}

	ClassTypeKind
	getClassTypeKind ()
	{
		return m_classTypeKind;
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
	getDestructor ()
	{
		return m_destructor;
	}

	Function*
	getOperatorNew ()
	{
		return m_operatorNew;
	}

	rtl::ConstList <StructField>
	getFieldList ()
	{
		return m_ifaceStructType->getFieldList ();
	}

	virtual
	StructField*
	getFieldByIndex (size_t index);

	virtual
	bool
	addMethod (Function* function);

	virtual
	bool
	addProperty (Property* prop);

	bool
	hasVTable ()
	{
		return !m_VTable.isEmpty ();
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
		ASSERT (m_pVTableStructType);
		return m_pVTableStructType;
	}

	Value
	getVTablePtrValue ()
	{
		return m_VTablePtrValue;
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

	bool
	callBaseTypeDestructors (const Value& thisValue);

	bool
	callMemberFieldDestructors (const Value& thisValue);

	bool
	callMemberPropertyDestructors (const Value& thisValue);

protected:
	void
	enumGcRootsImpl (
		Runtime* runtime,
		IfaceHdr* interface
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
		const Value& scopeLevelValue,
		const Value& rootValue,
		const Value& flagsValue
		);

	void
	primeInterface (
		ClassType* classType,
		const Value& opValue,
		const Value& VTableValue,
		const Value& objectValue,
		const Value& scopeLevelValue,
		const Value& rootValue,
		const Value& flagsValue
		);

	bool
	compileDefaultPreConstructor ();

	bool
	compileDefaultDestructor ();

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
		(type->getFlags () & ClassTypeFlagKind_Opaque);
}

//.............................................................................

// header of class interface

struct IfaceHdr
{
	void* m_pVTable;
	ObjHdr* m_object; // back pointer to master header

	// followed by parents, then by interface data fields
};

// TIfaceHdrTT is a simple trick for allowing multiple inheritance in implementation classes

template <typename T>
struct IfaceHdrT: IfaceHdr
{
}; 

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// interface with master header

template <typename T>
class ObjBox:
	public ObjHdr,
	public T
{
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef
void
FObject_Prime (
	ObjHdr* object,
	size_t scopeLevel,
	ObjHdr* root,
	uintptr_t flags
	);

typedef
void
FObject_Construct (IfaceHdr* interface);

typedef
void
FObject_Destruct (IfaceHdr* interface);

//.............................................................................

} // namespace jnc {
