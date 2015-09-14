#include "pch.h"
#include "jnc_UsingSet.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

ModuleItem* UsingSet::findItem (const char* name)
{
	size_t count = m_globalNamespaceArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		ModuleItem* item = m_globalNamespaceArray [i]->findItem (name);
		if (item)
			return item;
	}

	return NULL;
}
	
ModuleItem* UsingSet::findExtensionItem (
	NamedType* type,
	const char* name
	)
{
	size_t count = m_extensionNamespaceArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		ExtensionNamespace* nspace = m_extensionNamespaceArray [i];
		if (nspace->getType ()->cmp (type) == 0)
		{
			ModuleItem* item = nspace->findItem (name);
			if (item)
				return item;
		}
	}

	return NULL;
}

//.............................................................................

} // namespace jnc {
