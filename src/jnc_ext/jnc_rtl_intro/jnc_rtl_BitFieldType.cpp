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
#include "jnc_rtl_BitFieldType.h"
#include "jnc_ct_BitFieldType.h"
#include "jnc_ct_Module.h"
#include "jnc_Construct.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	BitFieldType,
	"jnc.BitFieldType",
	sl::g_nullGuid,
	-1,
	BitFieldType,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(BitFieldType)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<BitFieldType, ct::BitFieldType*>))
	JNC_MAP_CONST_PROPERTY("m_baseType", &BitFieldType::getBaseType);
	JNC_MAP_CONST_PROPERTY("m_bitOffset", &BitFieldType::getBitOffset);
	JNC_MAP_CONST_PROPERTY("m_bitCount", &BitFieldType::getBitCount);
	JNC_MAP_FUNCTION("extract", &BitFieldType::extract);
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

Variant
BitFieldType::extract(
	BitFieldType* self,
	DataPtr ptr
) {
	if (!ptr.m_p)
		return g_nullVariant;

	ct::Value value(ptr.m_p, self->m_item);
	bool result = self->m_item->getModule()->m_operatorMgr.extractBitField(value, self->m_item, &value);
	ASSERT(result);

	Variant variant;
	result = variant.create(value.getConstData(), self->m_item->getBaseType());
	ASSERT(result);

	return variant;
}

//..............................................................................

} // namespace rtl
} // namespace jnc
