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
#include "jnc_rtl_ArrayType.h"
#include "jnc_Construct.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	ArrayType,
	"jnc.ArrayType",
	sl::g_nullGuid,
	-1,
	ArrayType,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(ArrayType)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<ArrayType, ct::ArrayType*>))
	JNC_MAP_CONST_PROPERTY("m_elementType", &ArrayType::getElementType)
	JNC_MAP_CONST_PROPERTY("m_rootType", &ArrayType::getRootType)
	JNC_MAP_CONST_PROPERTY("m_elementCount", &ArrayType::getElementCount)
	JNC_MAP_CONST_PROPERTY("m_getDynamicSizeFunction", &ArrayType::getGetDynamicSizeFunction)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

} // namespace rtl
} // namespace jnc
