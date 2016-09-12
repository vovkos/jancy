#include "pch.h"
#include "jnc_ct_DoxyMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

sl::String
DoxyBlock::createDoxyDescriptionString ()
{
	sl::String string;

	if (!m_briefDescription.isEmpty ())
	{
		string.append ("<briefdescription><para>\n");
		string.append (m_briefDescription);
		string.append ("</para></briefdescription>\n");
	}

	if (!m_detailedDescription.isEmpty ())
	{
		string.append ("<detaileddescription><para>\n");
		string.append (m_detailedDescription);
		string.append ("</para></detaileddescription>\n");
	}

	return string;
}

//.............................................................................

bool
DoxyGroup::generateDocumentation (
	const char* outputDir,
	sl::String* itemXml,
	sl::String* indexXml
	)
{
	indexXml->appendFormat (
		"<compound kind='group' refid='%s'><name>%s</name></compound>\n", 
		m_refId.cc (), 
		m_name.cc ()
		);

	itemXml->format (
		"<compounddef kind='group' id='%s'>\n"
		"<compoundname>%s</compoundname>\n"
		"<title>%s</title>\n", 
		m_refId.cc (), 
		m_name.cc (),
		m_title.cc ()
		);

	sl::String sectionDef;

	size_t count = m_itemArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		ModuleItem* item = m_itemArray [i];
		ModuleItemDecl* decl = item->getDecl ();
		if (!decl)
			continue;

		ModuleItemKind itemKind = item->getItemKind ();

		bool isCompoundFile = 
			itemKind == ModuleItemKind_Namespace ||
			itemKind == ModuleItemKind_Type && ((Type*) item)->getTypeKind () != TypeKind_Enum;

		sl::String refId = item->getDoxyBlock ()->getRefId ();

		if (!isCompoundFile)
		{
			sectionDef.appendFormat ("<memberdef id='%s'/>", refId.cc ());
			sectionDef.append ('\n');
		}
		else
		{
			const char* elemName = itemKind == ModuleItemKind_Namespace ? "innernamespace" : "innerclass";
			sl::String refId = item->getDoxyBlock ()->getRefId ();
			itemXml->appendFormat ("<%s refid='%s'/>", elemName, refId.cc ());
			itemXml->append ('\n');
		}
	}

	if (!sectionDef.isEmpty ())
	{
		itemXml->append ("<sectiondef>\n");
		itemXml->append (sectionDef);
		itemXml->append ("</sectiondef>\n");
	}

	sl::BoxIterator <DoxyGroup*> groupIt = m_groupList.getHead ();
	for (; groupIt; groupIt++)
	{
		DoxyGroup* group = *groupIt;
		itemXml->appendFormat ("<innergroup refid='%s'/>", group->m_refId.cc ());
		itemXml->append ('\n');
	}

	itemXml->append (createDoxyDescriptionString ());
	itemXml->append ("</compounddef>\n");

	return true;
}

//.............................................................................

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
	sl::StringHashTableMapIterator <DoxyGroup*> it = m_groupMap.visit (name);
	if (it->m_value)
		return it->m_value;

	sl::String refId;
	refId.format ("group_%s", name.cc ());
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

sl::String
DoxyMgr::adjustRefId (const sl::StringRef& refId)
{
	sl::StringHashTableMapIterator <size_t> it = m_refIdMap.visit (refId);
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
DoxyMgr::setBlockTarget (
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
DoxyMgr::resolveBlockTargets ()
{
	bool result = true;

	GlobalNamespace* globalNspace = m_module->m_namespaceMgr.getGlobalNamespace ();
	Namespace* prevNspace = NULL;

	sl::Iterator <Target> it = m_targetList.getHead ();
	for (; it; it++)
	{
		Target* target = *it;
		
		ModuleItem* item = prevNspace && target->m_targetName.find ('.') == -1 ?
			prevNspace->findItem (target->m_targetName) :
			NULL;

		if (!item)
		{
			item = globalNspace->findItemByName (target->m_targetName);
			if (!item)
			{
				result = false;
				continue;
			}
		}

		if (item->m_doxyBlock && item->m_doxyBlock->m_group && !target->m_block->m_group)
			target->m_block->m_group = item->m_doxyBlock->m_group;
		
		item->m_doxyBlock = target->m_block;

		if (item->getItemKind () != ModuleItemKind_Property)
		{
			Namespace* itemNspace = item->getNamespace ();
			if (itemNspace)
				prevNspace = itemNspace;
		}
	}

	if (!result)		
		err::setStringError ("documentation target(s) not found");

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

				m_groupMap.eraseByKey (groupIt->m_name);
				m_groupList.erase (groupIt);
				isGroupDeleted = true;
			}

			groupIt = nextIt;
		}
	} while (isGroupDeleted);
}

bool
DoxyMgr::generateGroupDocumentation (
	const char* outputDir,
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

//.............................................................................

} // namespace ct
} // namespace jnc
