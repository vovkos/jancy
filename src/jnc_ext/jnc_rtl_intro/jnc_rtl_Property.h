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
#include "jnc_rtl_Namespace.h"
#include "jnc_rtl_NamedTypeBlock.h"
#include "jnc_ct_Property.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(Property)

//..............................................................................

class Property:
	public ModuleItemBase<ct::Property>,
	public Namespace,
	public NamedTypeBlock
{
public:
	Property(ct::Property* prop):
		ModuleItemBase(prop),
		Namespace(prop),
		NamedTypeBlock(prop)
	{
	}

	PropertyKind
	JNC_CDECL
	getPropertyKind()
	{
		return m_item->getPropertyKind();
	}

	PropertyType*
	JNC_CDECL
	getType()
	{
		return (PropertyType*)rtl::getType(m_item->getType());
	}

	Function*
	JNC_CDECL
	getGetter()
	{
		return (Function*)rtl::getFunction(m_item->getGetter());
	}

	Function*
	JNC_CDECL
	getSetter()
	{
		return (Function*)rtl::getFunction(m_item->getSetter());
	}

	Function*
	JNC_CDECL
	getBinder()
	{
		return (Function*)rtl::getFunction(m_item->getBinder());
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
