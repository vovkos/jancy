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
#include "jnc_rtl_DynamicStructType.h"
#include "jnc_Construct.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	DynamicStructType,
	"jnc.DynamicStructType",
	sl::g_nullGuid,
	-1,
	DynamicStructType,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(DynamicStructType)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<DynamicStructType, ct::DynamicStructType*>))
	JNC_MAP_CONST_PROPERTY("m_fieldAlignment", &DynamicStructType::getFieldAlignment)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

} // namespace rtl
} // namespace jnc
