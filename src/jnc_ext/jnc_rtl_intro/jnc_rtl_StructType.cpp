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
	StructField,
	"jnc.StructField",
	sl::g_nullGuid,
	-1,
	StructField,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(StructField)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<StructField, ct::StructField*>))
	JNC_MAP_CONST_PROPERTY("m_type", &StructField::getType)
	JNC_MAP_CONST_PROPERTY("m_ptrTypeFlags", &StructField::getPtrTypeFlags)
	JNC_MAP_CONST_PROPERTY("m_offset", &StructField::getOffset)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

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
	JNC_MAP_CONST_PROPERTY("m_fieldActualSize", &StructType::getFieldActualSize)
	JNC_MAP_CONST_PROPERTY("m_fieldAlignedSize", &StructType::getFieldAlignedSize)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

} // namespace rtl
} // namespace jnc
