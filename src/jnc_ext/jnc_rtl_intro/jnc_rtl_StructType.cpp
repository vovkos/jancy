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
#include "jnc_rtl_StructType.h"
#include "jnc_Construct.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	StructType,
	"jnc.StructType",
	sl::g_nullGuid,
	-1,
	StructType,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(StructType)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<StructType, ct::StructType*>))
	JNC_MAP_CONST_PROPERTY("m_fieldAlignment", &StructType::getFieldAlignment)
	JNC_MAP_CONST_PROPERTY("m_fieldSize", &StructType::getFieldSize)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

} // namespace rtl
} // namespace jnc
