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
#include "jnc_ct_BitFieldType.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(BitFieldType)

//..............................................................................

class BitFieldType: public TypeBase<ct::BitFieldType> {
public:
	BitFieldType(ct::BitFieldType* type):
		TypeBase(type) {}

	Type*
	JNC_CDECL
	getBaseType() {
		return rtl::getType(m_item->getBaseType());
	}

	size_t
	JNC_CDECL
	getBitOffset() {
		return m_item->getBitOffset();
	}

	size_t
	JNC_CDECL
	getBitCount() {
		return m_item->getBitCount();
	}

	static
	Variant
	extract(
		BitFieldType* self,
		DataPtr ptr
	);
};

//..............................................................................

} // namespace rtl
} // namespace jnc
