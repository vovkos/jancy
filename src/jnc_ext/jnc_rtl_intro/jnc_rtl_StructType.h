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
#include "jnc_ct_StructType.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(StructType)

//..............................................................................

class StructType: public DerivableTypeBase<ct::StructType> {
public:
	StructType(ct::StructType* type):
		DerivableTypeBase(type) {}

	size_t
	JNC_CDECL
	getFieldAlignment() {
		return m_item->getFieldAlignment();
	}

	size_t
	JNC_CDECL
	getFieldSize() {
		return m_item->getFieldSize();
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
