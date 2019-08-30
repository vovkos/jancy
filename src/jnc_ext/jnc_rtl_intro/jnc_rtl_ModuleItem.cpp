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
#include "jnc_rtl_ModuleItem.h"
#include "jnc_Construct.h"
#include "jnc_Runtime.h"
#include "jnc_rt_Runtime.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	ModuleItemDecl,
	"jnc.ModuleItemDecl",
	sl::g_nullGuid,
	-1,
	ModuleItemDecl,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(ModuleItemDecl)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<ModuleItemDecl, ct::ModuleItemDecl*>))
	JNC_MAP_DESTRUCTOR(&jnc::destruct<ModuleItemDecl>)
	JNC_MAP_CONST_PROPERTY("m_name", &ModuleItemDecl::getName)
	JNC_MAP_CONST_PROPERTY("m_qualifiedName", &ModuleItemDecl::getQualifiedName)
	JNC_MAP_CONST_PROPERTY("m_storageKind", &ModuleItemDecl::getStorageKind)
	JNC_MAP_CONST_PROPERTY("m_accessKind", &ModuleItemDecl::getAccessKind)
	JNC_MAP_CONST_PROPERTY("m_attributeBlock", &ModuleItemDecl::getAttributeBlock)
	JNC_MAP_CONST_PROPERTY("m_parentNamespace", &ModuleItemDecl::getParentNamespace)
	JNC_MAP_CONST_PROPERTY("m_parentUnit", &ModuleItemDecl::getParentUnit)
	JNC_MAP_CONST_PROPERTY("m_line", &ModuleItemDecl::getLine)
	JNC_MAP_CONST_PROPERTY("m_col", &ModuleItemDecl::getCol)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	ModuleItemInitializer,
	"jnc.ModuleItemInitializer",
	sl::g_nullGuid,
	-1,
	ModuleItemDecl,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(ModuleItemInitializer)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<ModuleItemInitializer, ct::ModuleItemInitializer*>))
	JNC_MAP_CONST_PROPERTY("m_initializer", &ModuleItemInitializer::getInitializer)
JNC_END_TYPE_FUNCTION_MAP()
// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	ModuleItem,
	"jnc.ModuleItem",
	sl::g_nullGuid,
	-1,
	ModuleItem,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(ModuleItem)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<ModuleItem, ct::ModuleItem*>))
	JNC_MAP_CONST_PROPERTY("m_module", &ModuleItem::getModule)
	JNC_MAP_CONST_PROPERTY("m_itemKind", &ModuleItem::getItemKind)
	JNC_MAP_CONST_PROPERTY("m_flags", &ModuleItem::getFlags)
	JNC_MAP_CONST_PROPERTY("m_type", &ModuleItem::getType)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
JNC_CDECL
ModuleItemDecl::markOpaqueGcRoots(jnc::GcHeap* gcHeap)
{
	if (!m_cache)
		return;

	gcHeap->markDataPtr(m_cache->m_namePtr);
	gcHeap->markDataPtr(m_cache->m_qualifiedNamePtr);
}

DataPtr
JNC_CDECL
ModuleItemDecl::getName(ModuleItemDecl* self)
{
	Cache* cache = self->getCache();
	if (!cache->m_namePtr.m_p)
		cache->m_namePtr = createForeignStringPtr(self->m_decl->getName(), false);

	return cache->m_namePtr;
}

DataPtr
JNC_CDECL
ModuleItemDecl::getQualifiedName(ModuleItemDecl* self)
{
	Cache* cache = self->getCache();
	if (!cache->m_namePtr.m_p)
		cache->m_namePtr = createForeignStringPtr(self->m_decl->getQualifiedName(), false);

	return cache->m_namePtr;
}

//..............................................................................

void
JNC_CDECL
ModuleItemInitializer::markOpaqueGcRoots(jnc::GcHeap* gcHeap)
{
	gcHeap->markDataPtr(m_initializerPtr);
}

DataPtr
JNC_CDECL
ModuleItemInitializer::getInitializer(ModuleItemInitializer* self)
{
	if (!self->m_initializerPtr.m_p)
		self->m_initializerPtr = createForeignStringPtr(self->m_initializer->getInitializerString(), false);

	return self->m_initializerPtr;
}

//..............................................................................

ModuleItem*
JNC_CDECL
getModuleItem(ct::ModuleItem* item)
{
	if (!item)
		return NULL;

	static StdType stdTypeTable[ModuleItemKind__Count] =
	{
		StdType_ModuleItem,       // ModuleItemKind_Undefined
		StdType_GlobalNamespace,  // ModuleItemKind_Namespace
		StdType_ModuleItem,       // ModuleItemKind_Scope
		StdType_Type,             // ModuleItemKind_Type
		StdType_Typedef,          // ModuleItemKind_Typedef
		StdType_Alias,            // ModuleItemKind_Alias
		StdType_Const,            // ModuleItemKind_Const
		StdType_Variable,         // ModuleItemKind_Variable
		StdType_FunctionArg,      // ModuleItemKind_FunctionArg
		StdType_Function,         // ModuleItemKind_Function
		StdType_Property,         // ModuleItemKind_Property
		StdType_ModuleItem,       // ModuleItemKind_PropertyTemplate
		StdType_EnumConst,        // ModuleItemKind_EnumConst
		StdType_StructField,      // ModuleItemKind_StructField
		StdType_BaseTypeSlot,     // ModuleItemKind_BaseTypeSlot
		StdType_ModuleItem,       // ModuleItemKind_Orphan
		StdType_ModuleItem,       // ModuleItemKind_Lazy
	};

	ModuleItemKind itemKind = item->getItemKind();
	ASSERT((size_t)itemKind < countof(stdTypeTable));

	return (ModuleItem*)getIntrospectionClass(item, stdTypeTable[itemKind]);
}

//..............................................................................

} // namespace rtl
} // namespace jnc
