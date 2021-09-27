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
#include "jnc_rtl_FunctionType.h"
#include "jnc_Construct.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	FunctionArg,
	"jnc.FunctionArg",
	sl::g_nullGuid,
	-1,
	FunctionArg,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(FunctionArg)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<FunctionArg, ct::FunctionArg*>))
	JNC_MAP_CONST_PROPERTY("m_type", &FunctionArg::getType)
	JNC_MAP_CONST_PROPERTY("m_ptrTypeFlags", &FunctionArg::getPtrTypeFlags)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	FunctionType,
	"jnc.FunctionType",
	sl::g_nullGuid,
	-1,
	FunctionType,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(FunctionType)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<FunctionType, ct::FunctionType*>))
	JNC_MAP_CONST_PROPERTY("m_returnType", &FunctionType::getReturnType)
	JNC_MAP_CONST_PROPERTY("m_argCount", &FunctionType::getArgCount)
	JNC_MAP_CONST_PROPERTY("m_argArray", &FunctionType::getArg)
	JNC_MAP_CONST_PROPERTY("m_shortType", &FunctionType::getShortType)
	JNC_MAP_FUNCTION("getFunctionPtrType", &FunctionType::getFunctionPtrType)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	FunctionPtrType,
	"jnc.FunctionPtrType",
	sl::g_nullGuid,
	-1,
	FunctionPtrType,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(FunctionPtrType)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<FunctionPtrType, ct::FunctionPtrType*>))
	JNC_MAP_CONST_PROPERTY("m_ptrTypeKind", &FunctionPtrType::getPtrTypeKind)
	JNC_MAP_CONST_PROPERTY("m_targetType", &FunctionPtrType::getTargetType)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

} // namespace rtl
} // namespace jnc
