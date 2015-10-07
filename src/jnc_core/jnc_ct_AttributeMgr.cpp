#include "pch.h"
#include "jnc_ct_AttributeMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

AttributeMgr::AttributeMgr ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);
}

AttributeBlock*
AttributeMgr::createAttributeBlock ()
{
	AttributeBlock* attributeBlock = AXL_MEM_NEW (AttributeBlock);
	attributeBlock->m_module = m_module;
	m_attributeBlockList.insertTail (attributeBlock);
	return attributeBlock;
}

//.............................................................................

} // namespace ct
} // namespace jnc
