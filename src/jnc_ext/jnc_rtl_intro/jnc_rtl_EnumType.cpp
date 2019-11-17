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
#include "jnc_rtl_EnumType.h"
#include "jnc_Construct.h"
#include "jnc_rt_Runtime.h"
#include "jnc_Runtime.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	EnumConst,
	"jnc.EnumConst",
	sl::g_nullGuid,
	-1,
	EnumConst,
	&EnumConst::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(EnumConst)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<EnumConst, ct::EnumConst*>))
	JNC_MAP_CONST_PROPERTY("m_parentType", &EnumConst::getParentType)
	JNC_MAP_CONST_PROPERTY("m_name", &EnumConst::getName)
	JNC_MAP_CONST_PROPERTY("m_value", &EnumConst::getValue)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	EnumType,
	"jnc.EnumType",
	sl::g_nullGuid,
	-1,
	EnumType,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(EnumType)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<EnumType, ct::EnumType*>))
	JNC_MAP_CONST_PROPERTY("m_baseType", &EnumType::getBaseType)
	JNC_MAP_CONST_PROPERTY("m_constCount", &EnumType::getConstCount)
	JNC_MAP_CONST_PROPERTY("m_constArray", &EnumType::getConst)
	JNC_MAP_FUNCTION("findConst", &EnumType::findConst)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
JNC_CDECL
EnumConst::markOpaqueGcRoots(jnc::GcHeap* gcHeap)
{
	gcHeap->markDataPtr(m_namePtr);
}

DataPtr
JNC_CDECL
EnumConst::getName(EnumConst* self)
{
	if (!self->m_namePtr.m_p)
		self->m_namePtr = createForeignStringPtr(self->m_item->getName(), false);

	return self->m_namePtr;
}

//..............................................................................

} // namespace rtl
} // namespace jnc
