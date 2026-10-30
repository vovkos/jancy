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
#include "jnc_rtl_AttributeBlock.h"
#include "jnc_Construct.h"
#include "jnc_rt_Runtime.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	Attribute,
	"jnc.Attribute",
	sl::g_nullGuid,
	-1,
	Attribute,
	&Attribute::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(Attribute)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<Attribute, ct::Attribute*>))
	JNC_MAP_CONST_PROPERTY("m_value", &Attribute::getValue)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	AttributeBlock,
	"jnc.AttributeBlock",
	sl::g_nullGuid,
	-1,
	AttributeBlock,
	&AttributeBlock::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(AttributeBlock)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<AttributeBlock, ct::AttributeBlock*>))
	JNC_MAP_DESTRUCTOR(&jnc::destruct<AttributeBlock>)
	JNC_MAP_CONST_PROPERTY("m_attributeCount", &AttributeBlock::getAttributeCount)
	JNC_MAP_CONST_PROPERTY("m_attributeArray", &AttributeBlock::getAttribute)
	JNC_MAP_FUNCTION("findAttribute", &AttributeBlock::findAttribute)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
JNC_CDECL
Attribute::markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
	if (m_item->getFlags() & ct::AttributeFlag_Dynamic)
		gcHeap->markVariant(m_item->getVariant());
}

//..............................................................................

void
JNC_CDECL
AttributeBlock::markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
	if (!(m_item->getFlags() & ct::AttributeBlockFlag_Dynamic))
		return;

	// dynamic values must be marked even if no rtl::Attribute wrapper is created yet

	const sl::Array<ct::Attribute*>& attributeArray = m_item->getAttributeArray();
	size_t count = attributeArray.getCount();
	for (size_t i = 0; i < count; i++) {
		ct::Attribute* attribute = attributeArray[i];
		if (attribute->getFlags() & ct::AttributeFlag_Dynamic)
			gcHeap->markVariant(attribute->getVariant());
	}
}

//..............................................................................

} // namespace rtl
} // namespace jnc
