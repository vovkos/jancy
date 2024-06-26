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
#include "jnc_ct_AttributeMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

AttributeMgr::AttributeMgr() {
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);
}

Attribute*
AttributeMgr::createAttribute(
	const sl::StringRef& name,
	sl::List<Token>* initializer
) {
	Attribute* attribute = new Attribute;
	attribute->m_module = m_module;
	attribute->m_name = name;

	if (initializer)
		sl::takeOver(&attribute->m_initializer, initializer);

	m_attributeList.insertTail(attribute);
	return attribute;
}

AttributeBlock*
AttributeMgr::createAttributeBlock() {
	AttributeBlock* attributeBlock = new AttributeBlock;
	attributeBlock->m_module = m_module;
	m_attributeBlockList.insertTail(attributeBlock);
	return attributeBlock;
}

AttributeBlock*
AttributeMgr::createDynamicAttributeBlock(ModuleItemDecl* decl) {
	AttributeBlock* attributeBlock = new AttributeBlock;
	attributeBlock->m_module = m_module;
	attributeBlock->m_parentUnit = decl->getParentUnit();
	attributeBlock->m_parentNamespace = decl->getParentNamespace();
	attributeBlock->m_flags |= AttributeBlockFlag_Dynamic | AttributeBlockFlag_ValuesReady;

	AttributeBlock* existingBlock = decl->getAttributeBlock();
	if (!existingBlock)
		attributeBlock->m_pos = decl->getPos();
	else {
		ASSERT(existingBlock->getFlags() & AttributeBlockFlag_ValuesReady);
		attributeBlock->m_pos = existingBlock->getPos();
		attributeBlock->addAttributeBlock(existingBlock);
	}

	return attributeBlock;
}

//..............................................................................

} // namespace ct
} // namespace jnc
