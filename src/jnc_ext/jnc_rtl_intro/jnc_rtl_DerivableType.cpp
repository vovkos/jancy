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
#include "jnc_rtl_DerivableType.h"
#include "jnc_Construct.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	BaseTypeSlot,
	"jnc.BaseTypeSlot",
	sl::g_nullGuid,
	-1,
	BaseTypeSlot,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(BaseTypeSlot)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<BaseTypeSlot, ct::BaseTypeSlot*>))
	JNC_MAP_CONST_PROPERTY("m_type", &BaseTypeSlot::getType);
	JNC_MAP_CONST_PROPERTY("m_offset", &BaseTypeSlot::getOffset);
	JNC_MAP_CONST_PROPERTY("m_vtableIndex", &BaseTypeSlot::getVtableIndex);
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	DerivableType,
	"jnc.DerivableType",
	sl::g_nullGuid,
	-1,
	DerivableType,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(DerivableType)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<DerivableType, ct::DerivableType*>))
	JNC_MAP_CONST_PROPERTY("m_baseTypeCount", &DerivableType::getBaseTypeCount)
	JNC_MAP_CONST_PROPERTY("m_baseTypeArray", &DerivableType::getBaseType)
	JNC_MAP_CONST_PROPERTY("m_unaryOperatorArray", &DerivableType::getUnaryOperator)
	JNC_MAP_CONST_PROPERTY("m_binaryOperatorArray", &DerivableType::getBinaryOperator)
	JNC_MAP_CONST_PROPERTY("m_callOperator", &DerivableType::getCallOperator)
	JNC_MAP_FUNCTION("findBaseTypeOffset", &DerivableType::findBaseTypeOffset)
	JNC_MAP_FUNCTION("findCastOperator", &DerivableType::findCastOperator)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

} // namespace rtl
} // namespace jnc
