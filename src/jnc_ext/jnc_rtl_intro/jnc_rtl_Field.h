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
#include "jnc_ct_Field.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(Field)

//..............................................................................

class Field:
	public ModuleItemBase<ct::Field>,
	public ModuleItemDecl,
	public ModuleItemInitializer
{
public:
	Field(ct::Field* field):
		ModuleItemBase(field),
		ModuleItemDecl(field),
		ModuleItemInitializer(field)
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

	size_t
	JNC_CDECL
	getOffset()
	{
		return m_item->getOffset();
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
