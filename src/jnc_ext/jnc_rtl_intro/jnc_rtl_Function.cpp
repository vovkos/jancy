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
#include "jnc_rtl_Function.h"
#include "jnc_Construct.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	Function,
	"jnc.Function",
	sl::g_nullGuid,
	-1,
	Function,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(Function)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<Function, ct::Function*>))
	JNC_MAP_CONST_PROPERTY("m_functionKind", &Function::getFunctionKind)
	JNC_MAP_CONST_PROPERTY("m_type", &Function::getType)
	JNC_MAP_CONST_PROPERTY("m_isMember", &Function::isMember)
	JNC_MAP_CONST_PROPERTY("m_machineCode", &Function::getMachineCode)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	FunctionOverload,
	"jnc.FunctionOverload",
	sl::g_nullGuid,
	-1,
	FunctionOverload,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(FunctionOverload)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<FunctionOverload, ct::FunctionOverload*>))
	JNC_MAP_CONST_PROPERTY("m_functionKind", &FunctionOverload::getFunctionKind)
	JNC_MAP_CONST_PROPERTY("m_overloadCount", &FunctionOverload::getOverloadCount)
	JNC_MAP_CONST_PROPERTY("m_overloadArray", &FunctionOverload::getOverload)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

} // namespace rtl
} // namespace jnc
