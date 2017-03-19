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
#include "jnc_ct_DoxyMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

DoxyMgr::DoxyMgr ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);
}

void
DoxyMgr::clear ()
{
	m_blockList.clear ();
	m_groupList.clear ();
	m_refIdMap.clear ();
	m_groupMap.clear ();
	m_targetList.clear ();
}

DoxyGroup*
DoxyMgr::getGroup (const sl::StringRef& name)
{
	sl::StringHashTableIterator <DoxyGroup*> it = m_groupMap.visit (name);
	if (it->m_value)
		return it->m_value;

	sl::String refId;
	refId.format ("group_%s", name.sz ());
	refId.replace ('-', '_');

	DoxyGroup* group = AXL_MEM_NEW (DoxyGroup);
	group->m_name = name;
	group->m_refId = adjustRefId (refId);

	m_groupList.insertTail (group);
	it->m_value = group;
	return  group;
}

DoxyBlock*
DoxyMgr::createBlock ()
{
	DoxyBlock* block = AXL_MEM_NEW (DoxyBlock);
	m_blockList.insertTail (block);
	return  block;
}

DoxyFootnote*
DoxyMgr::createFootnote ()
{
	DoxyFootnote* footnote = AXL_MEM_NEW (DoxyFootnote);
	m_blockList.insertTail (footnote);
	return footnote;
}

sl::String
DoxyMgr::adjustRefId (const sl::StringRef& refId)
{
	sl::StringHashTableIterator <size_t> it = m_refIdMap.visit (refId);
	if (!it->m_value) // no collisions
	{
		it->m_value = 2; // start with index 2
		return refId;
	}

	sl::String adjustedRefId;
	adjustedRefId.format ("%s_%d", refId.sz (), it->m_value);

	it->m_value++;
	return adjustedRefId;
}

void
DoxyMgr::setBlockTarget (
	DoxyBlock* block,
	DoxyTokenKind tokenKind,
	const sl::StringRef& itemName,
	size_t overloadIdx
	)
{
	Target* target = AXL_MEM_NEW (Target);
	target->m_block = block;
	target->m_tokenKind = tokenKind;
	target->m_itemName = itemName;
	target->m_overloadIdx = overloadIdx;
	m_targetList.insertTail (target);
}

bool
DoxyMgr::resolveBlockTargets ()
{
	bool result = true;

	GlobalNamespace* globalNspace = m_module->m_namespaceMgr.getGlobalNamespace ();
	Namespace* prevNspace = NULL;

	sl::Iterator <Target> it = m_targetList.getHead ();
	for (; it; it++)
	{
		Target* target = *it;
		ModuleItem* item = NULL;

		if (prevNspace && target->m_itemName.find ('.') == -1)
		{
			if (target->m_tokenKind == DoxyTokenKind_Function &&
				prevNspace->getNamespaceKind () == NamespaceKind_Type &&
				((NamedType*) prevNspace)->getTypeKindFlags () & TypeKindFlag_Derivable)
			{
				DerivableType* type = (DerivableType*) prevNspace;

				if (target->m_itemName == "construct")
				{
					item = type->getConstructor ();
					if (!item)
					{
						result = false;
						continue;
					}
				}
				else if (target->m_itemName == "destruct")
				{
					item = type->getDestructor ();
					if (!item)
					{
						result = false;
						continue;
					}
				}
			}

			if (!item)
				item = prevNspace->findItem (target->m_itemName);
		}

		if (!item)
		{
			item = globalNspace->findItemByName (target->m_itemName);
			if (!item)
			{
				result = false;
				continue;
			}
		}

		if (target->m_overloadIdx && item->getItemKind () == ModuleItemKind_Function)
		{
			Function* overload = ((Function*) item)->getOverload (target->m_overloadIdx);
			if (overload)
				item = overload;
		}

		if (item->m_doxyBlock && item->m_doxyBlock->m_group && !target->m_block->m_group)
			target->m_block->m_group = item->m_doxyBlock->m_group;

		item->setDoxyBlock (target->m_block);

		if (item->getItemKind () != ModuleItemKind_Property)
		{
			Namespace* itemNspace = item->getNamespace ();
			if (itemNspace)
				prevNspace = itemNspace;
		}
	}

	if (!result)
		err::setError ("documentation target(s) not found");

	return result;
}

void
DoxyMgr::deleteEmptyGroups ()
{
	bool isGroupDeleted;

	do
	{
		isGroupDeleted = false;

		sl::Iterator <DoxyGroup> groupIt = m_groupList.getHead ();
		while (groupIt)
		{
			sl::Iterator <DoxyGroup> nextIt = groupIt.getNext ();

			if (groupIt->isEmpty ())
			{
				if (groupIt->m_group)
					groupIt->m_group->m_groupList.remove (groupIt->m_parentGroupListIt);

				m_groupMap.eraseKey (groupIt->m_name);
				m_groupList.erase (groupIt);
				isGroupDeleted = true;
			}

			groupIt = nextIt;
		}
	} while (isGroupDeleted);
}

bool
DoxyMgr::generateGroupDocumentation (
	const sl::StringRef& outputDir,
	sl::String* indexXml
	)
{
	bool result;

	static char compoundFileHdr [] =
		"<?xml version='1.0' encoding='UTF-8' standalone='no'?>\n"
		"<doxygen>\n";

	static char compoundFileTerm [] = "</doxygen>\n";

	sl::String itemXml;

	sl::Iterator <DoxyGroup> groupIt = m_groupList.getHead ();
	for (; groupIt; groupIt++)
	{
		result = groupIt->generateDocumentation (outputDir, &itemXml, indexXml);
		if (!result)
			return false;

		sl::String refId = groupIt->getRefId ();
		sl::String fileName = sl::String (outputDir) + "/" + refId + ".xml";

		io::File compoundFile;
		result =
			compoundFile.open (fileName, io::FileFlag_Clear) &&
			compoundFile.write (compoundFileHdr, lengthof (compoundFileHdr)) != -1 &&
			compoundFile.write (itemXml, itemXml.getLength ()) != -1 &&
			compoundFile.write (compoundFileTerm, lengthof (compoundFileTerm)) != -1;

		if (!result)
			return false;
	}

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
