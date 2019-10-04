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

#pragma once

#include "jnc_ExtensionLib.h"
#include "jnc_rtl_IntrospectionLib.h"
#include "jnc_ct_ModuleItem.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(ModuleItemDecl)
JNC_DECLARE_OPAQUE_CLASS_TYPE(ModuleItemInitializer)
JNC_DECLARE_OPAQUE_CLASS_TYPE(ModuleItem)

//..............................................................................

class ModuleItemDecl: public IfaceHdr
{
protected:
	struct Cache
	{
		DataPtr m_namePtr;
		DataPtr m_qualifiedNamePtr;
	};

protected:
	Cache* m_cache;

protected:
	ct::ModuleItemDecl* m_decl;

public:
	ModuleItemDecl(ct::ModuleItemDecl* decl)
	{
		m_decl = decl;
	}

	~ModuleItemDecl()
	{
		if (m_cache)
			AXL_MEM_DELETE(m_cache);
	}

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	static
	DataPtr
	JNC_CDECL
	getName(ModuleItemDecl* self);

	static
	DataPtr
	JNC_CDECL
	getQualifiedName(ModuleItemDecl* self);

	StorageKind
	JNC_CDECL
	getStorageKind()
	{
		return m_decl->getStorageKind();
	}

	AccessKind
	JNC_CDECL
	getAccessKind()
	{
		return m_decl->getAccessKind();
	}

	AttributeBlock*
	JNC_CDECL
	getAttributeBlock()
	{
		return rtl::getAttributeBlock(m_decl->getAttributeBlock());
	}

	Namespace*
	JNC_CDECL
	getParentNamespace()
	{
		return rtl::getNamespace(m_decl->getParentNamespace());
	}

	Unit*
	JNC_CDECL
	getParentUnit()
	{
		return rtl::getUnit(m_decl->getParentUnit());
	}

	int
	JNC_CDECL
	getLine()
	{
		return m_decl->getPos().m_line;
	}

	int
	JNC_CDECL
	getCol()
	{
		return m_decl->getPos().m_col;
	}

protected:
	Cache*
	getCache()
	{
		return m_cache ? m_cache : m_cache = AXL_MEM_ZERO_NEW(Cache);
	}
};

//..............................................................................

class ModuleItemInitializer: public IfaceHdr
{
protected:
	DataPtr m_initializerPtr;

protected:
	ct::ModuleItemInitializer* m_initializer;

public:
	ModuleItemInitializer(ct::ModuleItemInitializer* initializer)
	{
		m_initializer = initializer;
	}

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	static
	DataPtr
	JNC_CDECL
	getInitializer(ModuleItemInitializer* self);
};

//..............................................................................

template <typename T>
class ModuleItemBase: public IfaceHdr
{
protected:
	T* m_item;

public:
	ModuleItemBase(T* moduleItem)
	{
		m_item = moduleItem;
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class ModuleItem: public ModuleItemBase<ct::ModuleItem>
{
public:
	ModuleItem(ct::ModuleItem* item):
		ModuleItemBase(item)
	{
	}

	Module*
	JNC_CDECL
	getModule()
	{
		return rtl::getModule(m_item->getModule());
	}

	ModuleItemKind
	JNC_CDECL
	getItemKind()
	{
		return m_item->getItemKind();
	}

	uint_t
	JNC_CDECL
	getFlags()
	{
		return m_item->getFlags();
	}

	Type*
	JNC_CDECL
	getType()
	{
		return rtl::getType(m_item->getType());
	}
};

//..............................................................................

ModuleItem*
JNC_CDECL
getModuleItem(ct::ModuleItem* item);

//..............................................................................

} // namespace rtl
} // namespace jnc
