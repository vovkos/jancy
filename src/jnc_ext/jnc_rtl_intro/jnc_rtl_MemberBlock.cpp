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
#include "jnc_rtl_MemberBlock.h"
#include "jnc_Construct.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	MemberBlock,
	"jnc.MemberBlock",
	sl::g_nullGuid,
	-1,
	MemberBlock,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(MemberBlock)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<MemberBlock, ct::MemberBlock*>))
	JNC_MAP_CONST_PROPERTY("m_staticConstructor", &MemberBlock::getStaticConstructor)
	JNC_MAP_CONST_PROPERTY("m_constructor", &MemberBlock::getConstructor)
	JNC_MAP_CONST_PROPERTY("m_destructor", &MemberBlock::getDestructor)
	JNC_MAP_CONST_PROPERTY("m_staticVariableCount", &MemberBlock::getStaticVariableCount)
	JNC_MAP_CONST_PROPERTY("m_staticVariableArray", &MemberBlock::getStaticVariable)
	JNC_MAP_CONST_PROPERTY("m_fieldCount", &MemberBlock::getFieldCount)
	JNC_MAP_CONST_PROPERTY("m_fieldArray", &MemberBlock::getField)
	JNC_MAP_CONST_PROPERTY("m_methodCount", &MemberBlock::getMethodCount)
	JNC_MAP_CONST_PROPERTY("m_methodArray", &MemberBlock::getMethod)
	JNC_MAP_CONST_PROPERTY("m_propertyCount", &MemberBlock::getPropertyCount)
	JNC_MAP_CONST_PROPERTY("m_propertyArray", &MemberBlock::getProperty)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

} // namespace rtl
} // namespace jnc
