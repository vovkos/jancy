#include "pch.h"
#include "jnc_ct_Namespace.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_StructType.h"
#include "jnc_ct_ClassType.h"

namespace jnc {
namespace ct {

//.............................................................................

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
Namespace::createQualifiedName (const char* name)
{
	if (m_qualifiedName.isEmpty ())
		return name;

	sl::String qualifiedName = m_qualifiedName;

	if (name && *name)
	{
		qualifiedName.append ('.');
		qualifiedName.append (name);
	}

	return qualifiedName;
}

ModuleItem*
Namespace::findItemByName (const char* name)
{
	if (!strchr (name, '.'))
		return findItem (name);

	QualifiedName qualifiedName;
	qualifiedName.parse (name);
	return findItem (qualifiedName);
}

ModuleItem*
Namespace::getItemByName (const char* name)
{
	ModuleItem* item = findItemByName (name);

	if (!item)
	{
		err::setFormatStringError ("'%s' not found", name);
		return NULL;
	}

	return item;
}

ModuleItem*
Namespace::findItem (const char* name)
{
	sl::StringHashTableMapIterator <ModuleItem*> it = m_itemMap.find (name);
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
	const char* name,
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
	const char* name,
	ModuleItem* item
	)
{
	sl::StringHashTableMapIterator <ModuleItem*> it = m_itemMap.visit (name);
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
	const sl::String& name,
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


sl::String
Namespace::generateMemberDocumentation (const char* outputDir)
{
	static char xmlHdr [] = "<?xml version='1.0' encoding='UTF-8' standalone='no'?>\n";

	sl::String resultDocumentation;

	size_t count = m_itemArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		ModuleItem* item = m_itemArray [i];
		Namespace* itemNamespace = item->getNamespace ();
		if (itemNamespace == this)
			continue;

		sl::String itemDocumentation = item->generateDocumentation (outputDir);
		if (itemDocumentation.isEmpty ())
			continue;
		
		ModuleItemKind itemKind = item->getItemKind ();
		if (itemKind != ModuleItemKind_Namespace &&
			itemKind != ModuleItemKind_Type)
		{
			resultDocumentation.append (itemDocumentation);
			resultDocumentation.append ('\n');
		}
		else
		{
			sl::String refId = item->getDox ()->getRefId ();
			sl::String fileName = outputDir;
			fileName += '/';
			fileName += refId;
			fileName += ".xml";
			
			io::File compoundFile;
			compoundFile.open (fileName);
			compoundFile.write (xmlHdr, lengthof (xmlHdr));
			compoundFile.write (itemDocumentation, itemDocumentation.getLength ());

			const char* elemName = itemKind == ModuleItemKind_Namespace ? "innernamespace" : "innerclass";
			resultDocumentation.appendFormat ("<%s refid='%s'/>", elemName, refId.cc ());
			resultDocumentation.append ('\n');
		}
	}

	return resultDocumentation;
}

//.............................................................................

sl::String
GlobalNamespace::generateDocumentation (const char* outputDir)
{
	sl::String documentation;

	documentation.format ("<compounddef kind='namespace' refid='%s'>\n", getDox ()->getRefId ().cc ());
	documentation += Namespace::generateMemberDocumentation (outputDir);

	documentation.append (createDoxDescriptionString ());
	documentation.append (createDoxLocationString ());

	documentation += "</compounddef>\n";
	
	return documentation;
}

//.............................................................................


} // namespace ct
} // namespace jnc
