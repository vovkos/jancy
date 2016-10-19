// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

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

enum PropertyTypeFlag
{
	PropertyTypeFlag_Const    = 0x010000,
	PropertyTypeFlag_Bindable = 0x020000,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_INLINE
PropertyTypeFlag
getFirstPropertyTypeFlag (uint_t flags)
{
	return (PropertyTypeFlag) (1 << sl::getLoBitIdx (flags));
}

const char*
getPropertyTypeFlagString (PropertyTypeFlag flag);

sl::String
getPropertyTypeFlagString (uint_t flags);

JNC_INLINE
const char*
getFirstPropertyTypeFlagString (uint_t flags)
{
	return getPropertyTypeFlagString (getFirstPropertyTypeFlag (flags));
}

//..............................................................................

enum PropertyPtrTypeKind
{
	PropertyPtrTypeKind_Normal = 0,
	PropertyPtrTypeKind_Weak,
	PropertyPtrTypeKind_Thin,
	PropertyPtrTypeKind__Count,
};

const char*
getPropertyPtrTypeKindString (PropertyPtrTypeKind ptrTypeKind);

//..............................................................................

class PropertyType: public Type
{
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
	PropertyType ();

	bool
	isReadOnly ()
	{
		return m_setterType.isEmpty ();
	}

	bool
	isIndexed ()
	{
		return !m_getterType->getArgArray ().isEmpty ();
	}

	bool
	isMemberPropertyType ()
	{
		return m_getterType->isMemberMethodType ();
	}

	Type*
	getThisArgType ()
	{
		return m_getterType->getThisArgType ();
	}

	DerivableType*
	getThisTargetType ()
	{
		return m_getterType->getThisTargetType ();
	}

	FunctionType*
	getGetterType ()
	{
		return m_getterType;
	}

	FunctionTypeOverload*
	getSetterType ()
	{
		return &m_setterType;
	}

	FunctionType*
	getBinderType ()
	{
		return m_binderType;
	}

	Type*
	getReturnType ()
	{
		ASSERT (m_getterType);
		return m_getterType->getReturnType ();
	}

	PropertyType*
	getMemberPropertyType (ClassType* type);

	PropertyType*
	getStdObjectMemberPropertyType ();

	PropertyType*
	getShortType  ();

	PropertyPtrType*
	getPropertyPtrType (
		Namespace* nspace,
		TypeKind typeKind,
		PropertyPtrTypeKind ptrTypeKind = PropertyPtrTypeKind_Normal,
		uint_t flags = 0
		);

	PropertyPtrType*
	getPropertyPtrType (
		TypeKind typeKind,
		PropertyPtrTypeKind ptrTypeKind = PropertyPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return getPropertyPtrType (NULL, typeKind, ptrTypeKind, flags);
	}

	PropertyPtrType*
	getPropertyPtrType (
		PropertyPtrTypeKind ptrTypeKind = PropertyPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return getPropertyPtrType (TypeKind_PropertyPtr, ptrTypeKind, flags);
	}

	StructType*
	getVTableStructType ();

	const sl::String&
	getBindableEventName ()
	{
		return m_bindableEventName;
	}

	sl::String
	getTypeModifierString ();

	static
	sl::String
	createSignature (
		FunctionType* getterType,
		const FunctionTypeOverload& setterType,
		uint_t flags
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
	prepareDoxyTypeString ();

	virtual
	void
	prepareLlvmType ()
	{
		ASSERT (false);
	}
};

//..............................................................................

struct SimplePropertyTypeTuple: sl::ListLink
{
	PropertyType* m_propertyTypeArray [3] [2] [2]; // call-conv-family x const x bindable
};

//..............................................................................

} // namespace ct
} // namespace jnc
