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
#include "jnc_rtl_Module.h"
#include "jnc_rtl_Namespace.h"
#include "jnc_Construct.h"
#include "jnc_rt_Runtime.h"
#include "jnc_Runtime.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	ModuleItemDecl,
	"jnc.ModuleItemDecl",
	sl::g_nullGuid,
	-1,
	ModuleItemDecl,
	&ModuleItemDecl::markOpaqueGcRoots
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
	ModuleItemInitializer,
	&ModuleItemInitializer::markOpaqueGcRoots
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
ModuleItemDecl::markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
	Cache* cache = m_cache.get();
	if (!cache)
		return;

	gcHeap->markString(cache->m_name);
	gcHeap->markString(cache->m_qualifiedName);
	gcHeap->markClassPtr(cache->m_parentUnit);
	gcHeap->markClassPtr(cache->m_parentNamespace);
	gcHeap->markClassPtr((ModuleItemBase<ct::AttributeBlock>*)cache->m_attributeBlock);
}

void
JNC_CDECL
ModuleItemDecl::initializeDynamicDecl(AttributeBlock* attributeBlock) {
	ASSERT(m_decl->getAttributeBlock() && (m_decl->getAttributeBlock()->getFlags() & ct::AttributeBlockFlag_Dynamic));
	m_dynamicDecl = m_decl;
	m_cache.getOrCreate()->m_attributeBlock = attributeBlock;
}

String
JNC_CDECL
ModuleItemDecl::getName(ModuleItemDecl* self) {
	Cache* cache = self->m_cache.getOrCreate();
	if (!cache->m_name.m_length)
		cache->m_name = createForeignString(self->m_decl->getName(), false);

	return cache->m_name;
}

String
JNC_CDECL
ModuleItemDecl::getQualifiedName(ModuleItemDecl* self) {
	Cache* cache = self->m_cache.getOrCreate();
	if (!cache->m_name.m_length)
		cache->m_name = createForeignString(self->m_decl->getQualifiedName(), false);

	return cache->m_name;
}

AttributeBlock*
JNC_CDECL
ModuleItemDecl::getAttributeBlock() {
	Cache* cache = m_cache.getOrCreate();
	if (!cache->m_attributeBlock)
		cache->m_attributeBlock = rtl::getAttributeBlock(m_decl->getAttributeBlock());

	return cache->m_attributeBlock;
}

Namespace*
JNC_CDECL
ModuleItemDecl::getParentNamespace() {
	Cache* cache = m_cache.getOrCreate();
	if (!cache->m_parentNamespace)
		cache->m_parentNamespace = rtl::getNamespace(m_decl->getParentNamespace());

	return cache->m_parentNamespace;
}

Unit*
JNC_CDECL
ModuleItemDecl::getParentUnit() {
	Cache* cache = m_cache.getOrCreate();
	if (!cache->m_parentUnit)
		cache->m_parentUnit = rtl::getUnit(m_decl->getParentUnit());;

	return cache->m_parentUnit;
}

//..............................................................................

void
JNC_CDECL
ModuleItemInitializer::markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
	gcHeap->markString(m_initializerString);
}

String
JNC_CDECL
ModuleItemInitializer::getInitializer(ModuleItemInitializer* self) {
	if (!self->m_initializerString.m_length)
		self->m_initializerString = createForeignString(self->m_initializer->getInitializerString(), false);

	return self->m_initializerString;
}

//..............................................................................

ModuleItem*
JNC_CDECL
getModuleItem(ct::ModuleItem* item) {
	if (!item)
		return NULL;

	static StdType stdTypeTable[ModuleItemKind__Count] = {
		StdType_ModuleItem,       // ModuleItemKind_Undefined
		StdType_GlobalNamespace,  // ModuleItemKind_Namespace
		StdType_Attribute,        // ModuleItemKind_Attribute
		StdType_AttributeBlock,   // ModuleItemKind_AttributeBlock
		StdType_ModuleItem,       // ModuleItemKind_Scope
		StdType_Type,             // ModuleItemKind_Type
		StdType_Typedef,          // ModuleItemKind_Typedef
		StdType_Alias,            // ModuleItemKind_Alias
		StdType_Const,            // ModuleItemKind_Const
		StdType_Variable,         // ModuleItemKind_Variable
		StdType_FunctionArg,      // ModuleItemKind_FunctionArg
		StdType_Function,         // ModuleItemKind_Function
		StdType_FunctionOverload, // ModuleItemKind_FunctionOverload
		StdType_Property,         // ModuleItemKind_Property
		StdType_ModuleItem,       // ModuleItemKind_PropertyTemplate
		StdType_EnumConst,        // ModuleItemKind_EnumConst
		StdType_Field,            // ModuleItemKind_Field
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
