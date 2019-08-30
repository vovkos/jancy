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
#include "jnc_rtl_Property.h"
#include "jnc_Construct.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	Property,
	"jnc.Property",
	sl::g_nullGuid,
	-1,
	Property,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(Property)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<Property, ct::Property*>))
	JNC_MAP_CONST_PROPERTY("m_propertyKind", &Property::getPropertyKind)
	JNC_MAP_CONST_PROPERTY("m_type", &Property::getType)
	JNC_MAP_CONST_PROPERTY("m_getter", &Property::getGetter)
	JNC_MAP_CONST_PROPERTY("m_setter", &Property::getSetter)
	JNC_MAP_CONST_PROPERTY("m_binder", &Property::getBinder)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

} // namespace rtl
} // namespace jnc
