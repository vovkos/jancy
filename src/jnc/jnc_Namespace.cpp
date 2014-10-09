#include "pch.h"
#include "jnc_Namespace.h"
#include "jnc_Module.h"
#include "jnc_StructType.h"
#include "jnc_ClassType.h"

namespace jnc {

//.............................................................................

Namespace*
getItemNamespace (ModuleItem* item)
{
	ModuleItemKind itemKind = item->getItemKind ();
	switch (itemKind)
	{
	case ModuleItemKind_Namespace:
		return (GlobalNamespace*) item;

	case ModuleItemKind_Property:
		return (Property*) item;

	case ModuleItemKind_Type:
		break;

	case ModuleItemKind_Typedef:
		item = ((Typedef*) item)->getType ();
		break;

	default:
		return NULL;
	}

	Type* type = (Type*) item;
	TypeKind typeKind = type->getTypeKind ();
	switch (typeKind)
	{
	case TypeKind_Enum:
	case TypeKind_Struct:
	case TypeKind_Union:
	case TypeKind_Class:
		return ((NamedType*) type);

	default:
		return NULL;
	}
}

//.............................................................................

const char*
getNamespaceKindString (NamespaceKind namespaceKind)
{
	static const char* stringTable [NamespaceKind__Count] =
	{
		"undefined-namespace-kind",  // ENamespace_Undefined = 0,
		"global",                    // ENamespace_Global,
		"scope",                     // ENamespace_Scope,
		"named-type",                // ENamespace_Type,
		"named-type-extension",      // ENamespace_TypeExtension,
		"property",                  // ENamespace_Property,
		"property-template",         // ENamespace_PropertyTemplate,
	};

	return (size_t) namespaceKind < NamespaceKind__Count ?
		stringTable [namespaceKind] :
		stringTable [NamespaceKind_Undefined];
}

//.............................................................................

void
Namespace::clear ()
{
	m_itemArray.clear ();
	m_itemMap.clear ();
}

rtl::String
Namespace::createQualifiedName (const char* name)
{
	if (m_qualifiedName.isEmpty ())
		return name;

	rtl::String qualifiedName = m_qualifiedName;

	if (name && *name)
	{
		qualifiedName.append ('.');
		qualifiedName.append (name);
	}

	return qualifiedName;
}

ModuleItem*
Namespace::getItemByName (const char* name)
{
	ModuleItem* item;

	if (!strchr (name, '.'))
	{
		item = findItem (name);
	}
	else
	{
		QualifiedName qualifiedName;
		qualifiedName.parse (name);
		item = findItem (qualifiedName);
	}

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
	rtl::StringHashTableMapIterator <ModuleItem*> it = m_itemMap.find (name);
	if (!it)
		return NULL;

	ModuleItem* item = it->m_value;
	if (!item || item->getItemKind () != ModuleItemKind_Lazy)
		return item;

	LazyModuleItem* lazyItem = (LazyModuleItem*) item;
	ASSERT (!(lazyItem->m_flags & LazyModuleItemFlag_Touched));

	it->m_value = NULL; // many lazy std-types are parsed, so 
	lazyItem->m_flags |= LazyModuleItemFlag_Touched;
	item = lazyItem->getActualItem ();

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

	rtl::BoxIterator <rtl::String> nameIt = name.getNameList ().getHead ();
	for (; nameIt; nameIt++)
	{
		Namespace* nspace = getItemNamespace (item);
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

	rtl::BoxIterator <rtl::String> nameIt = name.getNameList ().getHead ();
	for (; nameIt; nameIt++)
	{
		Namespace* nspace = getItemNamespace (item);
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

	if (!(flags & TraverseKind_NoParentNamespace) && m_parentNamespace)
	{
		item = m_parentNamespace->findItemTraverse (name, coord, flags & ~TraverseKind_NoThis);
		if (item)
			return item;
	}

	return NULL;
}

Function*
Namespace::getFunctionByName (
	const char* name,
	size_t overloadIdx
	)
{
	Function* function = getFunctionByName (name);
	return function ? function->getOverload (overloadIdx) : NULL;
}

bool
Namespace::addItem (
	ModuleItem* item,
	ModuleItemDecl* decl
	)
{
	rtl::StringHashTableMapIterator <ModuleItem*> it = m_itemMap.visit (decl->m_name);
	if (it->m_value)
	{
		setRedefinitionError (decl->m_name);
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

bool
Namespace::exposeEnumConsts (EnumType* type)
{
	bool result;

	rtl::Iterator <EnumConst> constIt = type->getConstList ().getHead ();
	for (; constIt; constIt++)
	{
		result = addItem (*constIt);
		if (!result)
			return false;
	}

	return true;
}

//.............................................................................

} // namespace jnc {
