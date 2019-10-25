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
#include "jnc_ct_Function.h"
#include "jnc_ct_FunctionType.h"
#include "jnc_Construct.h"

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
	JNC_MAP_CONST_PROPERTY("m_hasValue", &Attribute::hasValue)
	JNC_MAP_CONST_PROPERTY("m_value", &Attribute::getValue)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .


JNC_DEFINE_OPAQUE_CLASS_TYPE(
	AttributeBlock,
	"jnc.AttributeBlock",
	sl::g_nullGuid,
	-1,
	AttributeBlock,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(AttributeBlock)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<AttributeBlock, ct::AttributeBlock*>))
	JNC_MAP_CONST_PROPERTY("m_attributeCount", &AttributeBlock::getAttributeCount)
	JNC_MAP_CONST_PROPERTY("m_attributeArray", &AttributeBlock::getAttribute)
	JNC_MAP_FUNCTION("findAttribute", &AttributeBlock::findAttribute)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

Variant
JNC_CDECL
Attribute::getValue(Attribute* self)
{
	if (self->m_value.m_type)
		return self->m_value;

	const ct::Value& value = self->m_item->getValue();
	ct::ValueKind valueKind = value.getValueKind();
	ct::FunctionPtrType* type;
	ct::Function* function;
	void* p;

	switch (valueKind)
	{
	case ct::ValueKind_Void:
	case ct::ValueKind_Null:
		return g_nullVariant;

	case ct::ValueKind_Const:
		self->m_value.create(value.getConstData(), value.getType());
		break;

	case ct::ValueKind_Function:
		function = value.getFunction();
		p = function->getMachineCode();
		type = function->getType()->getFunctionPtrType(FunctionPtrTypeKind_Thin);
		self->m_value.create(&p, (jnc::Type*)type);
		break;

	default:
		ASSERT(false);
		return g_nullVariant;
	}

	return self->m_value;
}

//..............................................................................

} // namespace rtl
} // namespace jnc
