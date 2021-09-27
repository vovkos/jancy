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
#include "jnc_rtl_PropertyType.h"
#include "jnc_Construct.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	PropertyType,
	"jnc.PropertyType",
	sl::g_nullGuid,
	-1,
	PropertyType,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(PropertyType)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<PropertyType, ct::PropertyType*>))
	JNC_MAP_CONST_PROPERTY("m_isConst", &PropertyType::isConst)
	JNC_MAP_CONST_PROPERTY("m_isIndexed", &PropertyType::isIndexed)
	JNC_MAP_CONST_PROPERTY("m_getterType", &PropertyType::getGetterType)
	JNC_MAP_CONST_PROPERTY("m_setterType", &PropertyType::getSetterType)
	JNC_MAP_CONST_PROPERTY("m_binderType", &PropertyType::getBinderType)
	JNC_MAP_FUNCTION("getPropertyPtrType", &PropertyType::getPropertyPtrType)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	PropertyPtrType,
	"jnc.PropertyPtrType",
	sl::g_nullGuid,
	-1,
	PropertyPtrType,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(PropertyPtrType)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<PropertyPtrType, ct::PropertyPtrType*>))
	JNC_MAP_CONST_PROPERTY("m_ptrTypeKind", &PropertyPtrType::getPtrTypeKind)
	JNC_MAP_CONST_PROPERTY("m_targetType", &PropertyPtrType::getTargetType)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

} // namespace rtl
} // namespace jnc
