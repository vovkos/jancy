#include "pch.h"
#include "jnc_ct_UsingSet.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

ModuleItem* UsingSet::findItem (const sl::StringRef& name)
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
	const sl::StringRef& name
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

} // namespace ct
} // namespace jnc
