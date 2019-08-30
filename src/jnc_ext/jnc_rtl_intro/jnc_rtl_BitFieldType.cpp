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
#include "jnc_rtl_BitFieldType.h"
#include "jnc_Construct.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	BitFieldType,
	"jnc.BitFieldType",
	sl::g_nullGuid,
	-1,
	BitFieldType,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(BitFieldType)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<BitFieldType, ct::BitFieldType*>))
	JNC_MAP_CONST_PROPERTY("m_baseType", &BitFieldType::getBaseType);
	JNC_MAP_CONST_PROPERTY("m_bitOffset", &BitFieldType::getBitOffset);
	JNC_MAP_CONST_PROPERTY("m_bitCount", &BitFieldType::getBitCount);
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

} // namespace rtl
} // namespace jnc
