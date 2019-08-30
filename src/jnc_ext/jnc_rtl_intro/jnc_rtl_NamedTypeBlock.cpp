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
#include "jnc_rtl_NamedTypeBlock.h"
#include "jnc_Construct.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	NamedTypeBlock,
	"jnc.NamedTypeBlock",
	sl::g_nullGuid,
	-1,
	NamedTypeBlock,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(NamedTypeBlock)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<NamedTypeBlock, ct::NamedTypeBlock*>))
	JNC_MAP_CONST_PROPERTY("m_staticConstructor", &NamedTypeBlock::getStaticConstructor)
	JNC_MAP_CONST_PROPERTY("m_staticDestructor", &NamedTypeBlock::getStaticDestructor)
	JNC_MAP_CONST_PROPERTY("m_constructor", &NamedTypeBlock::getConstructor)
	JNC_MAP_CONST_PROPERTY("m_destructor", &NamedTypeBlock::getDestructor)
	JNC_MAP_CONST_PROPERTY("m_memberFieldCount", &NamedTypeBlock::getMemberFieldCount)
	JNC_MAP_CONST_PROPERTY("m_memberFieldArray", &NamedTypeBlock::getMemberField)
	JNC_MAP_CONST_PROPERTY("m_memberMethodCount", &NamedTypeBlock::getMemberMethodCount)
	JNC_MAP_CONST_PROPERTY("m_memberMethodArray", &NamedTypeBlock::getMemberMethod)
	JNC_MAP_CONST_PROPERTY("m_memberPropertyCount", &NamedTypeBlock::getMemberPropertyCount)
	JNC_MAP_CONST_PROPERTY("m_memberPropertyArray", &NamedTypeBlock::getMemberProperty)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

} // namespace rtl
} // namespace jnc
