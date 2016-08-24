#include "pch.h"
#include "jnc_ct_DoxyMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

DoxyMgr::DoxyMgr ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);
}

void
DoxyMgr::clear ()
{
	m_doxyBlockList.clear ();
	m_doxyRefIdMap.clear ();
	m_targetList.clear ();
}

DoxyBlock* 
DoxyMgr::createDoxyBlock ()
{
	DoxyBlock* block = AXL_MEM_NEW (DoxyBlock);
	m_doxyBlockList.insertTail (block);
	return  block;
}

sl::String
DoxyMgr::adjustDoxyRefId (const sl::StringRef& refId)
{
	sl::StringHashTableMapIterator <size_t> it = m_doxyRefIdMap.visit (refId);
	if (!it->m_value) // no collisions
	{
		it->m_value = 2; // start with index 2
		return refId;
	}

	sl::String adjustedRefId;
	adjustedRefId.format ("%s_%d", refId.cc (), it->m_value);
	
	it->m_value++;
	return adjustedRefId;
}

void
DoxyMgr::setDoxyBlockTarget (
	DoxyBlock* block,
	const sl::StringRef& targetName
	)
{
	Target* retarget = AXL_MEM_NEW (Target);
	retarget->m_block = block;
	retarget->m_targetName = targetName;
	m_targetList.insertTail (retarget);
}

bool
DoxyMgr::resolveDoxyBlockTargets ()
{
	bool result = true;

	GlobalNamespace* nspace = m_module->m_namespaceMgr.getGlobalNamespace ();

	sl::Iterator <Target> it = m_targetList.getHead ();
	for (; it; it++)
	{
		Target* target = *it;
		ModuleItem* item = nspace->findItemByName (target->m_targetName);
		if (!item)
		{
			result = false;
			continue;
		}

		item->m_doxyBlock = target->m_block;
	}

	if (!result)		
		err::setStringError ("documentation target(s) not found");

	return result;
}

//.............................................................................

} // namespace ct
} // namespace jnc
