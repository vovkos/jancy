//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#pragma once

#include "jnc_ClassType.h"
#include "jnc_ct_DerivableType.h"

namespace jnc {
namespace ct {

class ClassPtrType;
struct ClassPtrTypeTuple;
class StructType;
class Function;
class Property;

//..............................................................................

class ClassType: public DerivableType {
	friend class TypeMgr;
	friend class Parser;
	friend class Property;
	friend class StructType;
	friend class AsyncSequencerFunction;

protected:
	ClassTypeKind m_classTypeKind;

	StructType* m_vtableStructType;
	StructType* m_ifaceStructType;
	StructType* m_classStructType;

	sl::Array<BaseTypeSlot*> m_classBaseTypeArray;
	sl::Array<Field*> m_classFieldArray;
	sl::Array<Function*> m_virtualMethodArray;
	sl::Array<Function*> m_overrideMethodArray;
	sl::Array<Property*> m_virtualPropertyArray;

	sl::Array<Function*> m_vtable;
	Variable* m_vtableVariable;

	ClassPtrTypeTuple* m_classPtrTypeTuple;
	const OpaqueClassTypeInfo* m_opaqueClassTypeInfo;

public:
	ClassType();

	ClassTypeKind
	getClassTypeKind() {
		return m_classTypeKind;
	}

	StructType*
	getIfaceStructType() {
		ASSERT(m_ifaceStructType);
		return m_ifaceStructType;
	}

	StructType*
	getClassStructType() {
		ASSERT(m_classStructType);
		return m_classStructType;
	}

	ClassPtrType*
	getClassPtrType(
		TypeKind typeKind,
		ClassPtrTypeKind ptrTypeKind = ClassPtrTypeKind_Normal,
		uint_t flags = 0
	);

	ClassPtrType*
	getClassPtrType(
		ClassPtrTypeKind ptrTypeKind = ClassPtrTypeKind_Normal,
		uint_t flags = 0
	) {
		return getClassPtrType(TypeKind_ClassPtr, ptrTypeKind, flags);
	}

	virtual
	Type*
	getThisArgType(uint_t ptrTypeFlags) {
		return (Type*)getClassPtrType(ClassPtrTypeKind_Normal, ptrTypeFlags);
	}

	const OpaqueClassTypeInfo*
	getOpaqueClassTypeInfoFunc() {
		return m_opaqueClassTypeInfo;
	}

	virtual
	bool
	addMethod(Function* function);

	virtual
	bool
	addProperty(Property* prop);

	bool
	hasVtable() {
		return !m_vtable.isEmpty();
	}

	sl::Array<BaseTypeSlot*>
	getClassBaseTypeArray() {
		return m_classBaseTypeArray;
	}

	sl::Array<Field*>
	getClassFieldArray() {
		return m_classFieldArray;
	}

	sl::Array<Function*>
	getVirtualMethodArray() {
		return m_virtualMethodArray;
	}

	sl::Array<Property*>
	getVirtualPropertyArray() {
		return m_virtualPropertyArray;
	}

	StructType*
	getVtableStructType();

	Variable*
	getVtableVariable() {
		return m_vtableVariable;
	}

	virtual
	bool
	require() {
		return ensureLayout() && ensureCreatable() && requireConstructor();
	}

	virtual
	bool
	requireExternalReturn() {
		return ensureLayout() &&
			((m_flags & ClassTypeFlag_HasAbstractMethods) || // OK to return abstract classes
			ensureCreatable() && requireConstructor());
	}

	virtual
	void
	markGcRoots(
		const void* p,
		rt::GcHeap* gcHeap
	);

	bool
	ensureCreatable() {
		return (m_flags & ClassTypeFlag_Creatable) ? true : prepareForOperatorNew();
	}

protected:
	void
	markGcRootsImpl(
		IfaceHdr* iface,
		rt::GcHeap* gcHeap
	);

	virtual
	Field*
	createFieldImpl(
		const sl::StringRef& name,
		Type* type,
		size_t bitCount,
		uint_t ptrTypeFlags,
		sl::List<Token>* constructor,
		sl::List<Token>* initializer
	);

	virtual
	sl::StringRef
	createItemString(size_t index);

	virtual
	void
	prepareLlvmType();

	virtual
	void
	prepareLlvmDiType();

	virtual
	void
	prepareTypeVariable() {
		prepareSimpleTypeVariable(StdType_ClassType);
	}

	virtual
	bool
	calcLayout();

	bool
	addVirtualFunction(Function* function);

	bool
	overrideVirtualFunction(Function* function);

	bool
	ensureClassFieldsCreatable();

	virtual
	bool
	prepareForOperatorNew();
};

inline
ClassType::ClassType() {
	m_typeKind = TypeKind_Class;
	m_classTypeKind = ClassTypeKind_Normal;
	m_flags = TypeFlag_NoStack;
	m_ifaceStructType = NULL;
	m_classStructType = NULL;
	m_vtableStructType = NULL;
	m_classPtrTypeTuple = NULL;
	m_vtableVariable = NULL;
	m_opaqueClassTypeInfo = NULL;
	m_constructorThinThisFlag = 0;
}

//..............................................................................

} // namespace ct
} // namespace jnc
