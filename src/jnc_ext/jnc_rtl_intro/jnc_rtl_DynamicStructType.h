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
#include "jnc_ct_DynamicStructType.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(DynamicStructType)

//..............................................................................

class DynamicStructType: public NamedTypeBase<ct::DynamicStructType> {
public:
	DynamicStructType(ct::DynamicStructType* type):
		NamedTypeBase(type) {}

	size_t
	JNC_CDECL
	getFieldAlignment() {
		return m_item->getFieldAlignment();
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
