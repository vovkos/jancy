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
#include "jnc_rtl_Field.h"
#include "jnc_Construct.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	Field,
	"jnc.Field",
	sl::g_nullGuid,
	-1,
	Field,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(Field)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<Field, ct::Field*>))
	JNC_MAP_CONST_PROPERTY("m_type", &Field::getType)
	JNC_MAP_CONST_PROPERTY("m_ptrTypeFlags", &Field::getPtrTypeFlags)
	JNC_MAP_CONST_PROPERTY("m_offset", &Field::getOffset)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

} // namespace rtl
} // namespace jnc
