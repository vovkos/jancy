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
#include "jnc_rtl_ClassType.h"
#include "jnc_Construct.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	ClassType,
	"jnc.ClassType",
	sl::g_nullGuid,
	-1,
	ClassType,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(ClassType)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<ClassType, ct::ClassType*>))
	JNC_MAP_CONST_PROPERTY("m_classTypeKind", &ClassType::getClassTypeKind)
	JNC_MAP_CONST_PROPERTY("m_ifaceStructType", &ClassType::getIfaceStructType)
	JNC_MAP_CONST_PROPERTY("m_classStructType", &ClassType::getClassStructType)
	JNC_MAP_FUNCTION("getClassPtrType", &ClassType::getClassPtrType)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	ClassPtrType,
	"jnc.ClassPtrType",
	sl::g_nullGuid,
	-1,
	ClassPtrType,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(ClassPtrType)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<ClassPtrType, ct::ClassPtrType*>))
	JNC_MAP_CONST_PROPERTY("m_ptrTypeKind", &ClassPtrType::getPtrTypeKind)
	JNC_MAP_CONST_PROPERTY("m_targetType", &ClassPtrType::getTargetType)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

} // namespace rtl
} // namespace jnc
