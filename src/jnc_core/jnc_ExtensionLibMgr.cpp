#include "pch.h"
#include "jnc_ExtensionLibMgr.h"
#include "jnc_Module.h"

namespace jnc {

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

bool
ExtensionLibMgr::loadDynamicLib (const char* fileName)
{
	mt::DynamicLibrary* dynamicLib = m_dynamicLibList.insertTail ().p ();
	bool result = dynamicLib->load (fileName);
	if (!result)
		return false;

	GetExtensionLibFunc* getExtensionLib = (GetExtensionLibFunc*) dynamicLib->getFunction ("getExtensionLib");
	if (!getExtensionLib)
		return false;

	ExtensionLib* extensionLib = getExtensionLib (getExtensionLibSlotDb ());
	if (!extensionLib)
	{
		err::setFormatStringError ("cannot get extension lib in '%s'", fileName);
		return false;
	}
	
	return addLib (extensionLib);
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

rtl::StringSlice
ExtensionLibMgr::findSourceFile (const char* fileName)
{
	size_t count = m_libArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		ExtensionLib* lib = m_libArray [i];
		rtl::StringSlice source = lib->findSourceFile (fileName);
		if (!source.isEmpty ())
			return source;
	}

	return rtl::StringSlice ();
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

ModuleItem*
ExtensionLibMgr::findItem (
	size_t libSlot,
	size_t itemSlot,
	const char* name
	)
{
	ASSERT (m_module);

	if (libSlot == -1 || itemSlot == -1) // no caching for this item
		return m_module->m_namespaceMgr.getGlobalNamespace ()->getItemByName (name);

	size_t count = m_itemCache.getCount ();
	if (count <= libSlot)
		m_itemCache.setCount (libSlot + 1);

	rtl::Array <ModuleItem*>* itemArray = m_itemCache [libSlot];
	if (!itemArray)
	{
		itemArray = AXL_MEM_NEW (rtl::Array <ModuleItem*>);
		m_itemCache [libSlot] = itemArray;
	}

	count = itemArray->getCount ();
	if (count <= itemSlot)
		itemArray->setCount (itemSlot + 1);

	ModuleItem* item = (*itemArray) [itemSlot];
	if (item)
		return item;

	item = m_module->m_namespaceMgr.getGlobalNamespace ()->getItemByName (name);
	(*itemArray) [itemSlot] = item;
	return item;	
}

//.............................................................................

size_t 
ExtensionLibSlotDb::getSlot (const rtl::Guid& guid)
{
	rtl::HashTableMapIterator <rtl::Guid, size_t> it = m_slotMap.visit (guid);
	if (it->m_value)
		return it->m_value;

	size_t slot = ++m_slot;
	it->m_value = slot;
	return slot;
}

//.............................................................................

} // namespace jnc {
