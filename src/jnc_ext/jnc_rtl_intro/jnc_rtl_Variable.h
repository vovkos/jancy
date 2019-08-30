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

#pragma once

#include "jnc_rtl_ModuleItem.h"
#include "jnc_ct_Variable.h"
#include "jnc_ct_ConstMgr.h"
#include "jnc_rt_GcHeap.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(Variable)
JNC_DECLARE_OPAQUE_CLASS_TYPE(Const)

//..............................................................................

class Variable:
	public ModuleItemBase<ct::Variable>,
	public ModuleItemDecl,
	public ModuleItemInitializer
{
public:
	Variable(ct::Variable* variable):
		ModuleItemBase(variable),
		ModuleItemDecl(variable),
		ModuleItemInitializer(variable)
	{
	}

	Type*
	JNC_CDECL
	getType()
	{
		return rtl::getType(m_item->getType());
	}

	uint_t
	JNC_CDECL
	getPtrTypeFlags()
	{
		return m_item->getPtrTypeFlags();
	}
};

//..............................................................................

class Const:
	public ModuleItemBase<ct::Const>,
	public ModuleItemDecl
{
protected:
	Variant m_value;

public:
	Const(ct::Const* cnst):
		ModuleItemBase(cnst),
		ModuleItemDecl(cnst)
	{
	}

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap)
	{
		gcHeap->markVariant(m_value);
	}

	static
	Variant
	JNC_CDECL
	getValue(Const* self);
};

//..............................................................................

} // namespace rtl
} // namespace jnc
