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

#include "jnc_ct_FunctionTypeOverload.h"
#include "jnc_ct_StructType.h"
#include "jnc_PropertyType.h"

namespace jnc {
namespace ct {

class PropertyPtrType;
class ClassType;

struct PropertyPtrTypeTuple;

//..............................................................................

class PropertyType: public Type {
	friend class TypeMgr;

protected:
	FunctionType* m_getterType;
	FunctionTypeOverload m_setterType;
	FunctionType* m_binderType;
	PropertyType* m_stdObjectMemberPropertyType;
	PropertyType* m_shortType;
	StructType* m_vtableStructType;
	PropertyPtrTypeTuple* m_propertyPtrTypeTuple;

	sl::String m_bindableEventName;

public:
	PropertyType();

	bool
	isConst() {
		return m_setterType.isEmpty();
	}

	bool
	isIndexed() {
		return !m_getterType->getArgArray().isEmpty();
	}

	bool
	isMemberPropertyType() {
		return m_getterType->isMemberMethodType();
	}

	Type*
	getThisArgType() {
		return m_getterType->getThisArgType();
	}

	DerivableType*
	getThisTargetType() {
		return m_getterType->getThisTargetType();
	}

	FunctionType*
	getGetterType() {
		return m_getterType;
	}

	FunctionTypeOverload*
	getSetterType() {
		return &m_setterType;
	}

	FunctionType*
	getBinderType() {
		return m_binderType;
	}

	Type*
	getReturnType() {
		ASSERT(m_getterType);
		return m_getterType->getReturnType();
	}

	PropertyType*
	getMemberPropertyType(ClassType* type);

	PropertyType*
	getStdObjectMemberPropertyType();

	PropertyType*
	getShortType();

	PropertyPtrType*
	getPropertyPtrType(
		TypeKind typeKind,
		PropertyPtrTypeKind ptrTypeKind = PropertyPtrTypeKind_Normal,
		uint_t flags = 0
	);

	PropertyPtrType*
	getPropertyPtrType(
		PropertyPtrTypeKind ptrTypeKind = PropertyPtrTypeKind_Normal,
		uint_t flags = 0
	) {
		return getPropertyPtrType(TypeKind_PropertyPtr, ptrTypeKind, flags);
	}

	StructType*
	getVtableStructType();

	const sl::String&
	getBindableEventName() {
		return m_bindableEventName;
	}

	sl::String
	getTypeModifierString();

	static
	sl::String
	createSignature(
		FunctionType* getterType,
		const FunctionTypeOverload& setterType,
		uint_t flags
	);

protected:
	virtual
	void
	prepareSignature() {
		m_signature = createSignature(m_getterType, m_setterType, m_flags);
	}

	virtual
	void
	prepareTypeString();

	virtual
	void
	prepareDoxyLinkedText();

	virtual
	void
	prepareDoxyTypeString();

	virtual
	void
	prepareLlvmType() {
		ASSERT(false);
	}

	virtual
	void
	prepareTypeVariable() {
		prepareSimpleTypeVariable(StdType_PropertyType);
	}

	virtual
	bool
	resolveImports();
};

//..............................................................................

struct SimplePropertyTypeTuple: sl::ListLink {
	PropertyType* m_propertyTypeArray[3][2][2]; // call-conv-family x const x bindable
};

//..............................................................................

} // namespace ct
} // namespace jnc
