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
#include "jnc_ct_ArrayType.h"
#include "jnc_ct_Function.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(ArrayType)

//..............................................................................

class ArrayType: public TypeBase<ct::ArrayType> {
public:
	ArrayType(ct::ArrayType* type):
		TypeBase(type) {}

	Type*
	JNC_CDECL
	getElementType() {
		return rtl::getType(m_item->getElementType());
	}

	Type*
	JNC_CDECL
	getRootType() {
		return rtl::getType(m_item->getRootType());
	}

	size_t
	JNC_CDECL
	getElementCount() {
		return m_item->getElementCount();
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
