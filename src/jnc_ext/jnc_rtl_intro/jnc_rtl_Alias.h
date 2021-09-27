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
#include "jnc_ct_Alias.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(Alias)

//..............................................................................

class Alias:
	public ModuleItemBase<ct::Alias>,
	public ModuleItemDecl,
	public ModuleItemInitializer {
public:
	Alias(ct::Alias* alias):
		ModuleItemBase(alias),
		ModuleItemDecl(alias),
		ModuleItemInitializer(alias) {}

	ModuleItem*
	JNC_CDECL
	getTargetItem() {
		return rtl::getModuleItem(m_item->getTargetItem());
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
