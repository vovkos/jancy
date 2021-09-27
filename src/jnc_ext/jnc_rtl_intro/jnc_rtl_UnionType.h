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
#include "jnc_ct_UnionType.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(UnionType)

//..............................................................................

class UnionType: public DerivableTypeBase<ct::UnionType> {
public:
	UnionType(ct::UnionType* type):
		DerivableTypeBase(type) {}

	StructType*
	JNC_CDECL
	getStructType() {
		return (StructType*)rtl::getType(m_item->getStructType());
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
