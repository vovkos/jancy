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

#include "pch.h"
#include "jnc_ct_DynamicStructType.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_ArrayType.h"

namespace jnc {
namespace ct {

//..............................................................................

DynamicStructType::DynamicStructType() {
	m_typeKind = TypeKind_DynamicStruct;
	m_fieldAlignment = 8;
}

//..............................................................................

} // namespace ct
} // namespace jnc
