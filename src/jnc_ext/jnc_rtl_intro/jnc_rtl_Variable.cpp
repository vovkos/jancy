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
#include "jnc_rtl_Variable.h"
#include "jnc_Construct.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	Variable,
	"jnc.Variable",
	sl::g_nullGuid,
	-1,
	Variable,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(Variable)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<Variable, ct::Variable*>))
	JNC_MAP_CONST_PROPERTY("m_type", &Variable::getType)
	JNC_MAP_CONST_PROPERTY("m_ptrTypeFlags", &Variable::getPtrTypeFlags)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	Const,
	"jnc.Const",
	sl::g_nullGuid,
	-1,
	Const,
	&Const::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(Const)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<Const, ct::Const*>))
	JNC_MAP_CONST_PROPERTY("m_value", &Const::getValue)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

Variant
JNC_CDECL
Const::getValue(Const* self)
{
	if (self->m_value.m_type)
		return self->m_value;

	const ct::Value& value = self->m_item->getValue();
	if (value.getValueKind() != ct::ValueKind_Const)
		return g_nullVariant;

	self->m_value.create(value.getConstData(), value.getType());
	return self->m_value;
}

//..............................................................................

} // namespace rtl
} // namespace jnc
