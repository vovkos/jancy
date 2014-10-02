// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_FunctionTypeOverload.h"
#include "jnc_StructType.h"

namespace jnc {

class PropertyPtrType;
class ClassType;

struct PropertyPtrTypeTuple;

//.............................................................................

enum PropertyTypeFlagKind
{
	PropertyTypeFlagKind_Const    = 0x010000,
	PropertyTypeFlagKind_Bindable = 0x020000,
	PropertyTypeFlagKind_Throws   = 0x040000,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
PropertyTypeFlagKind
getFirstPropertyTypeFlag (uint_t flags)
{
	return (PropertyTypeFlagKind) (1 << rtl::getLoBitIdx (flags));
}

const char*
getPropertyTypeFlagString (PropertyTypeFlagKind flag);

rtl::String
getPropertyTypeFlagString (uint_t flags);

inline
const char*
getFirstPropertyTypeFlagString (uint_t flags)
{
	return getPropertyTypeFlagString (getFirstPropertyTypeFlag (flags));
}

//.............................................................................

enum PropertyPtrTypeKind
{
	PropertyPtrTypeKind_Normal = 0,
	PropertyPtrTypeKind_Weak,
	PropertyPtrTypeKind_Thin,
	PropertyPtrTypeKind__Count,
};

const char*
getPropertyPtrTypeKindString (PropertyPtrTypeKind ptrTypeKind);

//.............................................................................

class PropertyType: public Type
{
	friend class TypeMgr;

protected:
	FunctionType* m_getterType;
	FunctionTypeOverload m_setterType;
	FunctionType* m_binderType;
	PropertyType* m_stdObjectMemberPropertyType;
	PropertyType* m_shortType;
	StructType* m_pVTableStructType;
	PropertyPtrTypeTuple* m_propertyPtrTypeTuple;

	rtl::String m_bindableEventName;
	rtl::String m_typeModifierString;

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

	NamedType*
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

	rtl::String
	getBindableEventName ()
	{
		return m_bindableEventName;
	}

	rtl::String
	getTypeModifierString ();

	static
	rtl::String
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
	prepareLlvmType ()
	{
		ASSERT (false);
	}
};

//.............................................................................

struct SimplePropertyTypeTuple: rtl::ListLink
{
	PropertyType* m_propertyTypeArray [3] [2] [2]; // call-conv-family x const x bindable
};

//.............................................................................

} // namespace jnc {
