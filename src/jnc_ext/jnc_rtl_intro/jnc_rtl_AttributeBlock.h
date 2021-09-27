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
#include "jnc_ct_AttributeBlock.h"
#include "jnc_rt_GcHeap.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(Attribute)
JNC_DECLARE_OPAQUE_CLASS_TYPE(AttributeBlock)

//..............................................................................

class Attribute:
	public ModuleItemBase<ct::Attribute>,
	public ModuleItemDecl,
	public ModuleItemInitializer {
protected:
	Variant m_value;

public:
	Attribute(ct::Attribute* attribute):
		ModuleItemBase(attribute),
		ModuleItemDecl(attribute),
		ModuleItemInitializer(attribute) {}

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
		gcHeap->markVariant(m_value);
	}

	bool
	JNC_CDECL
	hasValue() {
		return !m_item->getValue().isEmpty();
	}

	static
	Variant
	JNC_CDECL
	getValue(Attribute* self);
};

//..............................................................................

class AttributeBlock:
	public ModuleItemBase<ct::AttributeBlock>,
	public ModuleItemDecl {
public:
	AttributeBlock(ct::AttributeBlock* block):
		ModuleItemBase(block),
		ModuleItemDecl(block) {}

	size_t
	JNC_CDECL
	getAttributeCount() {
		return m_item->getAttributeArray().getCount();
	}

	Attribute*
	JNC_CDECL
	getAttribute(size_t index) {
		size_t count = m_item->getAttributeArray().getCount();
		return index < count ? rtl::getAttribute(m_item->getAttributeArray()[index]) : NULL;
	}

	Attribute*
	JNC_CDECL
	findAttribute(DataPtr namePtr) {
		return rtl::getAttribute(m_item->findAttribute((char*)namePtr.m_p));
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
