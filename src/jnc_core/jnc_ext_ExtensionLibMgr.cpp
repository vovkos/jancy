#include "pch.h"
#include "jnc_ext_ExtensionLibMgr.h"
#include "jnc_ext_ExtensionLib.h"
#include "jnc_ext_StdExtensionLibHost.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ext {

//.............................................................................

ExtensionLibMgr::ExtensionLibMgr ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);
}

void
ExtensionLibMgr::clear ()
{
	m_libArray.clear ();
	m_itemCache.clear ();
	m_dynamicLibList.clear ();
}

bool
ExtensionLibMgr::addLib (ExtensionLib* lib)
{
	bool result = lib->forcedExport (m_module);
	if (!result)
		return false;

	m_libArray.append (lib);
	return true;
}

ExtensionLib*
ExtensionLibMgr::loadDynamicLib (const char* fileName)
{
	mt::DynamicLibrary* dynamicLib = m_dynamicLibList.insertTail ().p ();
	bool result = dynamicLib->load (fileName);
	if (!result)
		return NULL;

	ExtensionLibMainFunc* mainFunc = (ExtensionLibMainFunc*) dynamicLib->getFunction (g_extensionLibMainFuncName);
	if (!mainFunc)
		return NULL;

	ExtensionLib* extensionLib = mainFunc (getStdExtensionLibHost ());
	if (!extensionLib)
	{
		err::setFormatStringError ("cannot get extension lib in '%s'", fileName);
		return NULL;
	}
	
	result = addLib (extensionLib);
	if (!result)
		return NULL;

	return extensionLib;
}

bool
ExtensionLibMgr::mapFunctions ()
{
	size_t count = m_libArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		ExtensionLib* lib = m_libArray [i];
		bool result = lib->mapFunctions (m_module);
		if (!result)
			return false;
	}

	return true;
}

sl::StringSlice
ExtensionLibMgr::findSourceFile (const char* fileName)
{
	size_t count = m_libArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		ExtensionLib* lib = m_libArray [i];
		sl::StringSlice source = lib->findSourceFile (fileName);
		if (!source.isEmpty ())
			return source;
	}

	return sl::StringSlice ();
}

const OpaqueClassTypeInfo*
ExtensionLibMgr::findOpaqueClassTypeInfo (const char* qualifiedName)
{
	size_t count = m_libArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		ExtensionLib* lib = m_libArray [i];
		const OpaqueClassTypeInfo* typeInfo = lib->findOpaqueClassTypeInfo (qualifiedName);
		if (typeInfo)
			return typeInfo;
	}

	return NULL;
}

ct::ModuleItem*
ExtensionLibMgr::findItem (
	const char* name,
	size_t libCacheSlot,
	size_t itemCacheSlot
	)
{
	ASSERT (m_module);

	if (libCacheSlot == -1 || itemCacheSlot == -1) // no caching for this item
		return m_module->m_namespaceMgr.getGlobalNamespace ()->getItemByName (name);

	size_t count = m_itemCache.getCount ();
	if (count <= libCacheSlot)
		m_itemCache.setCount (libCacheSlot + 1);

	sl::Array <ModuleItem*>* itemArray = m_itemCache [libCacheSlot];
	if (!itemArray)
	{
		itemArray = AXL_MEM_NEW (sl::Array <ModuleItem*>);
		m_itemCache [libCacheSlot] = itemArray;
	}

	count = itemArray->getCount ();
	if (count <= itemCacheSlot)
		itemArray->setCount (itemCacheSlot + 1);

	ModuleItem* item = (*itemArray) [itemCacheSlot];
	if (item)
		return item;

	item = m_module->m_namespaceMgr.getGlobalNamespace ()->getItemByName (name);
	(*itemArray) [itemCacheSlot] = item;
	return item;	
}

//.............................................................................

} // namespace ext
} // namespace jnc
