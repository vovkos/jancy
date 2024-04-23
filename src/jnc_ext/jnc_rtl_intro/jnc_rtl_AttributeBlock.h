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
	AttributeBlock* m_dynamicAttributeBlock;

public:
	Attribute(ct::Attribute* attribute):
		ModuleItemBase(attribute),
		ModuleItemDecl(attribute),
		ModuleItemInitializer(attribute) {}

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	static
	Variant
	JNC_CDECL
	getValue(Attribute* self) {
		self->m_item->ensureVariantReady();
		return self->m_item->getVariant();
	}
};

//..............................................................................

class AttributeBlock:
	public ModuleItemBase<ct::AttributeBlock>,
	public ModuleItemDecl {

public:
	AttributeBlock(ct::AttributeBlock* block):
		ModuleItemBase(block),
		ModuleItemDecl(block) {}

	~AttributeBlock() {
		if (m_item->getFlags() & ct::AttributeBlockFlag_Dynamic)
			delete m_item;
	}

	size_t
	JNC_CDECL
	getAttributeCount() {
		return m_item->getAttributeArray().getCount();
	}

	Attribute*
	JNC_CDECL
	getAttribute(size_t i) {
		return i < m_item->getAttributeArray().getCount() ? getAttributeImpl(m_item->getAttributeArray()[i]): NULL;
	}

	Attribute*
	JNC_CDECL
	findAttribute(String name) {
		return getAttributeImpl(m_item->findAttribute(name >> toAxl));
	}

protected:
	Attribute*
	JNC_CDECL
	getAttributeImpl(ct::Attribute* attribute) {
		return attribute && (attribute->getFlags() & ct::AttributeFlag_Dynamic) ?
			(Attribute*)(void*)createIntrospectionClass(attribute, StdType_Attribute) :
			rtl::getAttribute(attribute);
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
