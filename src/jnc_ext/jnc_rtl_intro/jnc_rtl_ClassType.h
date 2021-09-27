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

#include "jnc_rtl_DerivableType.h"
#include "jnc_ct_ClassType.h"
#include "jnc_ct_ClassPtrType.h"
#include "jnc_ct_StructType.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(ClassType)
JNC_DECLARE_OPAQUE_CLASS_TYPE(ClassPtrType)

//..............................................................................

class ClassType: public DerivableTypeBase<ct::ClassType> {
public:
	ClassType(ct::ClassType* type):
		DerivableTypeBase(type) {}

	uint_t
	JNC_CDECL
	getClassTypeKind() {
		return m_item->getClassTypeKind();
	}

	StructType*
	JNC_CDECL
	getIfaceStructType() {
		return (StructType*)rtl::getType(m_item->getIfaceStructType());
	}

	StructType*
	JNC_CDECL
	getClassStructType() {
		return (StructType*)rtl::getType(m_item->getClassStructType());
	}

	ClassPtrType*
	JNC_CDECL
	getClassPtrType(
		TypeKind typeKind,
		ClassPtrTypeKind ptrTypeKind,
		uint_t flags
	) {
		return (ClassPtrType*)rtl::getType(m_item->getClassPtrType(typeKind, ptrTypeKind, flags));
	}
};

//..............................................................................

class ClassPtrType: public TypeBase<ct::ClassPtrType> {
public:
	ClassPtrType(ct::ClassPtrType* type):
		TypeBase(type) {}

	ClassPtrTypeKind
	JNC_CDECL
	getPtrTypeKind() {
		return m_item->getPtrTypeKind();
	}

	ClassType*
	JNC_CDECL
	getTargetType() {
		return (ClassType*)rtl::getType(m_item->getTargetType());
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
