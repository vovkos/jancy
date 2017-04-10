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
#include "jnc_ct_Namespace.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_StructType.h"
#include "jnc_ct_ClassType.h"

namespace jnc {
namespace ct {

//..............................................................................

void
Namespace::clear ()
{
	m_itemArray.clear ();
	m_itemMap.clear ();
	m_friendSet.clear ();
	m_dualPtrTypeTupleMap.clear ();
	m_usingSet.clear ();
}

sl::String
Namespace::createQualifiedName (const sl::StringRef& name)
{
	if (m_qualifiedName.isEmpty ())
		return name;

	sl::String qualifiedName = m_qualifiedName;

	if (!name.isEmpty ())
	{
		qualifiedName.append ('.');
		qualifiedName.append (name);
	}

	return qualifiedName;
}

ModuleItem*
Namespace::findItemByName (const sl::StringRef& name)
{
	if (name.find ('.') == -1)
		return findItem (name);

	QualifiedName qualifiedName;
	qualifiedName.parse (name);
	return findItem (qualifiedName);
}

ModuleItem*
Namespace::getItemByName (const sl::StringRef& name)
{
	ModuleItem* item = findItemByName (name);

	if (!item)
	{
		err::setFormatStringError ("'%s' not found", name.sz ());
		return NULL;
	}

	return item;
}

ModuleItem*
Namespace::findItem (const sl::StringRef& name)
{
	sl::StringHashTableIterator <ModuleItem*> it = m_itemMap.find (name);
	if (!it)
		return NULL;

	ModuleItem* item = it->m_value;
	if (!item || item->getItemKind () != ModuleItemKind_Lazy)
		return item;

	LazyModuleItem* lazyItem = (LazyModuleItem*) item;
	ASSERT (!(lazyItem->m_flags & LazyModuleItemFlag_Touched));

	it->m_value = NULL; // many lazy std-types are parsed, so remove it from namespace
	lazyItem->m_flags |= LazyModuleItemFlag_Touched;

	item = lazyItem->getActualItem ();
	if (!item)
		return NULL;

	if (!it->m_value)
	{
		m_itemArray.append (item);
		it->m_value = item;
	}

	ASSERT (it->m_value == item);
	return item;
}

ModuleItem*
Namespace::findItem (const QualifiedName& name)
{
	ModuleItem* item = findItem (name.getFirstName ());
	if (!item)
		return NULL;

	sl::BoxIterator <sl::String> nameIt = name.getNameList ().getHead ();
	for (; nameIt; nameIt++)
	{
		Namespace* nspace = item->getNamespace ();
		if (!nspace)
			return NULL;

		item = nspace->findItem (*nameIt);
		if (!item)
			return NULL;
	}

	return item;
}

ModuleItem*
Namespace::findItemTraverse (
	const QualifiedName& name,
	MemberCoord* coord,
	uint_t flags
	)
{
	ModuleItem* item = findItemTraverse (name.getFirstName (), coord, flags);
	if (!item)
		return NULL;

	sl::BoxIterator <sl::String> nameIt = name.getNameList ().getHead ();
	for (; nameIt; nameIt++)
	{
		Namespace* nspace = item->getNamespace ();
		if (!nspace)
			return NULL;

		item = nspace->findItem (*nameIt);
		if (!item)
			return NULL;
	}

	return item;
}

ModuleItem*
Namespace::findItemTraverseImpl (
	const sl::StringRef& name,
	MemberCoord* coord,
	uint_t flags
	)
{
	ModuleItem* item;

	if (!(flags & TraverseKind_NoThis))
	{
		item = findItem (name);
		if (item)
			return item;
	}

	if (!(flags & TraverseKind_NoUsingNamespaces))
	{
		item = m_usingSet.findItem (name);
		if (item)
			return item;
	}

	if (!(flags & TraverseKind_NoParentNamespace) && m_parentNamespace)
	{
		item = m_parentNamespace->findItemTraverse (name, coord, flags & ~TraverseKind_NoThis);
		if (item)
			return item;
	}

	return NULL;
}

bool
Namespace::addItem (
	const sl::StringRef& name,
	ModuleItem* item
	)
{
	sl::StringHashTableIterator <ModuleItem*> it = m_itemMap.visit (name);
	if (it->m_value)
	{
		setRedefinitionError (name);
		return false;
	}

	if (item->getItemKind () != ModuleItemKind_Lazy)
		m_itemArray.append (item);

	it->m_value = item;
	return true;
}

size_t
Namespace::addFunction (Function* function)
{
	ModuleItem* oldItem = findItem (function->m_name);
	if (!oldItem)
	{
		addItem (function);
		return 0;
	}

	if (oldItem->getItemKind () != ModuleItemKind_Function)
	{
		setRedefinitionError (function->m_name);
		return -1;
	}

	return ((Function*) oldItem)->addOverload (function);
}

Const*
Namespace::createConst (
	const sl::StringRef& name,
	const Value& value
	)
{
	ASSERT (value.getValueKind () == ValueKind_Const && value.getType ());

	Module* module = value.getType ()->getModule ();
	sl::String qualifiedName = createQualifiedName (name);

	Const* cnst = module->m_constMgr.createConst (name, qualifiedName, value);
	bool result = addItem (cnst);
	if (!result)
		return NULL;

	return cnst;
}

bool
Namespace::exposeEnumConsts (EnumType* type)
{
	bool result;

	sl::Array <EnumConst*> constArray = type->getConstArray ();

	size_t count = constArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		EnumConst* enumConst = constArray [i];
		result = addItem (enumConst);
		if (!result)
			return false;
	}

	return true;
}

bool
Namespace::generateMemberDocumentation (
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml,
	bool useSectionDef
	)
{
	bool result;

	static char compoundFileHdr [] =
		"<?xml version='1.0' encoding='UTF-8' standalone='no'?>\n"
		"<doxygen>\n";

	static char compoundFileTerm [] = "</doxygen>\n";

	itemXml->clear ();

	sl::String sectionDef;
	sl::String memberXml;

	sl::Array <EnumType*> unnamedEnumTypeArray;

	size_t count = m_itemArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		ModuleItem* item = m_itemArray [i];
		ModuleItemDecl* decl = item->getDecl ();
		if (!decl)
			continue;

		Unit* unit = decl->getParentUnit ();
		if (!unit || unit->getLib ()) // don't document imported libraries
			continue;

		Namespace* itemNamespace = item->getNamespace ();
		if (itemNamespace == this)
			continue;

		if (item->getItemKind () == ModuleItemKind_EnumConst)
		{
			EnumType* enumType = ((EnumConst*) item)->getParentEnumType ();
			if (enumType != this) // otherwise, we are being called on behalf of EnumType
			{
				if (!enumType->getName ().isEmpty ()) // exposed enum, not unnamed
					continue;

				unnamedEnumTypeArray.append (enumType);

				// skip all other enum consts of this unnamed enum

				for (i++; i < count; i++)
				{
					item = m_itemArray [i];				
					if (item->getItemKind () != ModuleItemKind_EnumConst ||
						((EnumConst*) item)->getParentEnumType () != enumType)
						break;
				}

				i--; // undo last increment
				continue;
			}
		}

		result = item->generateDocumentation (outputDir, &memberXml, indexXml);
		if (!result)
			return false;

		if (memberXml.isEmpty ())
			continue;

		DoxyBlock* doxyBlock = item->getDoxyBlock ();
		DoxyGroup* doxyGroup = doxyBlock->getGroup ();
		if (doxyGroup)
			doxyGroup->addItem (item);

		ModuleItemKind itemKind = item->getItemKind ();
		bool isCompoundFile =
			itemKind == ModuleItemKind_Namespace ||
			itemKind == ModuleItemKind_Type && ((Type*) item)->getTypeKind () != TypeKind_Enum;

		if (!isCompoundFile)
		{
			sectionDef.append (memberXml);
			sectionDef.append ('\n');
		}
		else
		{
			sl::String refId = doxyBlock->getRefId ();
			sl::String fileName = sl::String (outputDir) + "/" + refId + ".xml";

			io::File compoundFile;
			result =
				compoundFile.open (fileName, io::FileFlag_Clear) &&
				compoundFile.write (compoundFileHdr, lengthof (compoundFileHdr)) != -1 &&
				compoundFile.write (memberXml, memberXml.getLength ()) != -1 &&
				compoundFile.write (compoundFileTerm, lengthof (compoundFileTerm)) != -1;

			if (!result)
				return false;

			const char* elemName = itemKind == ModuleItemKind_Namespace ? "innernamespace" : "innerclass";
			itemXml->appendFormat ("<%s refid='%s'/>", elemName, refId.sz ());
			itemXml->append ('\n');
		}
	}

	count = unnamedEnumTypeArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		ModuleItem* item = unnamedEnumTypeArray [i];
		result = item->generateDocumentation (outputDir, &memberXml, indexXml);
		if (!result)
			return false;

		ASSERT (!memberXml.isEmpty ());
		sectionDef.append (memberXml);
		sectionDef.append ('\n');
	}

	if (!sectionDef.isEmpty ())
		if (!useSectionDef)
		{
			itemXml->append (sectionDef);
		}
		else
		{
			itemXml->append ("<sectiondef>\n");
			itemXml->append (sectionDef);
			itemXml->append ("</sectiondef>\n");
		}

	return true;
}

//..............................................................................

sl::String
GlobalNamespace::createDoxyRefId ()
{
	sl::String refId;

	if (this == m_module->m_namespaceMgr.getGlobalNamespace ())
	{
		refId = "global_namespace";
	}
	else
	{
		refId.format ("namespace_%s", m_qualifiedName.sz ());
		refId.makeLowerCase ();
	}

	return m_module->m_doxyMgr.adjustRefId (refId);
}

bool
GlobalNamespace::generateDocumentation (
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
	)
{
	DoxyBlock* doxyBlock = getDoxyBlock ();

	const char* kind;
	const char* name;

	if (this == m_module->m_namespaceMgr.getGlobalNamespace ())
	{
		kind = "file";
		name = "global";
	}
	else
	{
		kind = "namespace";
		name = getQualifiedName ();
	}

	indexXml->appendFormat (
		"<compound kind='%s' refid='%s'><name>%s</name></compound>\n",
		kind,
		doxyBlock->getRefId ().sz (),
		name
		);

	itemXml->format (
		"<compounddef kind='%s' id='%s' language='Jancy'>\n"
		"<compoundname>%s</compoundname>\n",
		kind,
		doxyBlock->getRefId ().sz (),
		name
		);

	sl::String memberXml;
	Namespace::generateMemberDocumentation (outputDir, &memberXml, indexXml, true);
	itemXml->append (memberXml);

	sl::String footnoteXml = doxyBlock->getFootnoteString ();
	if (!footnoteXml.isEmpty ())
	{
		itemXml->append ("<sectiondef>\n");
		itemXml->append (footnoteXml);
		itemXml->append ("</sectiondef>\n");
	}

	itemXml->append (doxyBlock->getDescriptionString ());
	itemXml->append (getDoxyLocationString ());
	itemXml->append ("</compounddef>\n");

	return true;
}

//..............................................................................


} // namespace ct
} // namespace jnc
