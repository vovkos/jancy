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

#include "jnc_rtl_Type.h"
#include "jnc_ct_PropertyType.h"
#include "jnc_ct_PropertyPtrType.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(PropertyType)
JNC_DECLARE_OPAQUE_CLASS_TYPE(PropertyPtrType)

//..............................................................................

class PropertyType: public TypeBase<ct::PropertyType> {
public:
	PropertyType(ct::PropertyType* type):
		TypeBase(type) {}

	bool
	JNC_CDECL
	isConst() {
		return m_item->isConst();
	}

	bool
	JNC_CDECL
	isIndexed() {
		return m_item->isIndexed();
	}

	FunctionType*
	JNC_CDECL
	getGetterType() {
		return (FunctionType*)rtl::getType(m_item->getGetterType());
	}

	FunctionType*
	JNC_CDECL
	getSetterType() {
		return (FunctionType*)rtl::getType(m_item->getSetterType()->getOverload(0));
	}

	FunctionType*
	JNC_CDECL
	getBinderType() {
		return (FunctionType*)rtl::getType(m_item->getBinderType());
	}

	PropertyPtrType*
	JNC_CDECL
	getPropertyPtrType(
		TypeKind typeKind,
		PropertyPtrTypeKind ptrTypeKind,
		uint_t flags = 0
	) {
		return (PropertyPtrType*)rtl::getType(m_item->getPropertyPtrType(typeKind, ptrTypeKind, flags));
	}
};

//..............................................................................

class PropertyPtrType: public TypeBase<ct::PropertyPtrType> {
public:
	PropertyPtrType(ct::PropertyPtrType* type):
		TypeBase(type) {}

	PropertyPtrTypeKind
	JNC_CDECL
	getPtrTypeKind() {
		return m_item->getPtrTypeKind();
	}

	PropertyType*
	JNC_CDECL
	getTargetType() {
		return (PropertyType*)rtl::getType(m_item->getTargetType());
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
