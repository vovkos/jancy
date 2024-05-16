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
#include "jnc_rtl_AttributeBlock.h"
#include "jnc_rtl_Namespace.h"
#include "jnc_rtl_Field.h"
#include "jnc_rtl_Type.h"
#include "jnc_rtl_ArrayType.h"
#include "jnc_rtl_FunctionType.h"
#include "jnc_rtl_PropertyType.h"
#include "jnc_rtl_DerivableType.h"
#include "jnc_rtl_EnumType.h"
#include "jnc_rtl_ClassType.h"
#include "jnc_rtl_StructType.h"
#include "jnc_rtl_UnionType.h"
#include "jnc_rtl_Alias.h"
#include "jnc_rtl_Variable.h"
#include "jnc_rtl_Function.h"
#include "jnc_rtl_Property.h"
#include "jnc_rtl_Module.h"
#include "jnc_ct_Module.h"
#include "jnc_rt_Runtime.h"
#include "jnc_Runtime.h"

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// jancy sources

;static char g_jnc_introSrc[] =
#include "jnc_intro.jnc.cpp"
;static char g_jnc_aliasSrc[] =
#include "jnc_Alias.jnc.cpp"
;static char g_jnc_arrayTypeSrc[] =
#include "jnc_ArrayType.jnc.cpp"
;static char g_jnc_attributeBlockSrc[] =
#include "jnc_AttributeBlock.jnc.cpp"
;static char g_jnc_classTypeSrc[] =
#include "jnc_ClassType.jnc.cpp"
;static char g_jnc_derivableTypeSrc[] =
#include "jnc_DerivableType.jnc.cpp"
;static char g_jnc_enumTypeSrc[] =
#include "jnc_EnumType.jnc.cpp"
;static char g_jnc_fieldSrc[] =
#include "jnc_Field.jnc.cpp"
;static char g_jnc_functionSrc[] =
#include "jnc_Function.jnc.cpp"
;static char g_jnc_functionTypeSrc[] =
#include "jnc_FunctionType.jnc.cpp"
;static char g_jnc_memberBlockSrc[] =
#include "jnc_MemberBlock.jnc.cpp"
;static char g_jnc_moduleSrc[] =
#include "jnc_Module.jnc.cpp"
;static char g_jnc_moduleItemSrc[] =
#include "jnc_ModuleItem.jnc.cpp"
;static char g_jnc_namespaceSrc[] =
#include "jnc_Namespace.jnc.cpp"
;static char g_jnc_opKindSrc[] =
#include "jnc_OpKind.jnc.cpp"
;static char g_jnc_propertySrc[] =
#include "jnc_Property.jnc.cpp"
;static char g_jnc_propertyTypeSrc[] =
#include "jnc_PropertyType.jnc.cpp"
;static char g_jnc_structTypeSrc[] =
#include "jnc_StructType.jnc.cpp"
;static char g_jnc_typeSrc[] =
#include "jnc_Type.jnc.cpp"
;static char g_jnc_unionTypeSrc[] =
#include "jnc_UnionType.jnc.cpp"
;static char g_jnc_variableSrc[] =
#include "jnc_Variable.jnc.cpp"
;

namespace jnc {
namespace rtl {

//..............................................................................

// {A6A0CD47-577D-4BF0-8095-D000F7CD38A1}
JNC_DEFINE_GUID(
	jnc_g_introspectionLibGuid,
	0xa6a0cd47, 0x577d, 0x4bf0, 0x80, 0x95, 0xd0, 0x0, 0xf7, 0xcd, 0x38, 0xa1
);

JNC_DEFINE_LIB(
	jnc_IntrospectionLib,
	jnc_g_introspectionLibGuid,
	"IntrospectionLib",
	"Jancy introspection RTL extension library"
)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE(jnc_IntrospectionLib)
	JNC_LIB_SOURCE_FILE("jnc_intro.jnc",          g_jnc_introSrc)
	JNC_LIB_SOURCE_FILE("jnc_Alias.jnc",          g_jnc_aliasSrc)
	JNC_LIB_SOURCE_FILE("jnc_ArrayType.jnc",      g_jnc_arrayTypeSrc)
	JNC_LIB_SOURCE_FILE("jnc_AttributeBlock.jnc", g_jnc_attributeBlockSrc)
	JNC_LIB_SOURCE_FILE("jnc_ClassType.jnc",      g_jnc_classTypeSrc)
	JNC_LIB_SOURCE_FILE("jnc_DerivableType.jnc",  g_jnc_derivableTypeSrc)
	JNC_LIB_SOURCE_FILE("jnc_EnumType.jnc",       g_jnc_enumTypeSrc)
	JNC_LIB_SOURCE_FILE("jnc_Field.jnc",          g_jnc_fieldSrc)
	JNC_LIB_SOURCE_FILE("jnc_Function.jnc",       g_jnc_functionSrc)
	JNC_LIB_SOURCE_FILE("jnc_FunctionType.jnc",   g_jnc_functionTypeSrc)
	JNC_LIB_SOURCE_FILE("jnc_MemberBlock.jnc",    g_jnc_memberBlockSrc)
	JNC_LIB_SOURCE_FILE("jnc_Module.jnc",         g_jnc_moduleSrc)
	JNC_LIB_SOURCE_FILE("jnc_ModuleItem.jnc",     g_jnc_moduleItemSrc)
	JNC_LIB_SOURCE_FILE("jnc_Namespace.jnc",      g_jnc_namespaceSrc)
	JNC_LIB_SOURCE_FILE("jnc_OpKind.jnc",         g_jnc_opKindSrc)
	JNC_LIB_SOURCE_FILE("jnc_Property.jnc",       g_jnc_propertySrc)
	JNC_LIB_SOURCE_FILE("jnc_PropertyType.jnc",   g_jnc_propertyTypeSrc)
	JNC_LIB_SOURCE_FILE("jnc_StructType.jnc",     g_jnc_structTypeSrc)
	JNC_LIB_SOURCE_FILE("jnc_Type.jnc",           g_jnc_typeSrc)
	JNC_LIB_SOURCE_FILE("jnc_UnionType.jnc",      g_jnc_unionTypeSrc)
	JNC_LIB_SOURCE_FILE("jnc_Variable.jnc",       g_jnc_variableSrc)
JNC_END_LIB_SOURCE_FILE_TABLE()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE(jnc_IntrospectionLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(ModuleItem)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(ModuleItemDecl)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(ModuleItemInitializer)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Attribute)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(AttributeBlock)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Namespace)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(GlobalNamespace)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Type)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(DataPtrType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(NamedType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(MemberBlock)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(BaseTypeSlot)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(DerivableType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(ArrayType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(FunctionArg)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(FunctionType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(FunctionPtrType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(PropertyType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(PropertyPtrType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(EnumConst)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(EnumType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(ClassType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(ClassPtrType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Field)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(StructType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(UnionType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Alias)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Const)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Variable)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Function)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(FunctionOverload)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Property)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Typedef)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Module)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Unit)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE()

JNC_BEGIN_LIB_FUNCTION_MAP(jnc_IntrospectionLib)
	JNC_ASSERT_NO_REUSABLE_TAIL_PADDING(ModuleItem);
	JNC_ASSERT_NO_REUSABLE_TAIL_PADDING(ModuleItemDecl);
	JNC_ASSERT_NO_REUSABLE_TAIL_PADDING(ModuleItemInitializer);
	JNC_ASSERT_NO_REUSABLE_TAIL_PADDING(Namespace);
	JNC_ASSERT_NO_REUSABLE_TAIL_PADDING(Type);
	JNC_ASSERT_NO_REUSABLE_TAIL_PADDING(NamedType);
	JNC_ASSERT_NO_REUSABLE_TAIL_PADDING(DerivableType);
	JNC_ASSERT_NO_REUSABLE_TAIL_PADDING(MemberBlock);

	JNC_MAP_TYPE(ModuleItem)
	JNC_MAP_TYPE(ModuleItemDecl)
	JNC_MAP_TYPE(ModuleItemInitializer)
	JNC_MAP_TYPE(Attribute)
	JNC_MAP_TYPE(AttributeBlock)
	JNC_MAP_TYPE(Namespace)
	JNC_MAP_TYPE(GlobalNamespace)
	JNC_MAP_TYPE(Type)
	JNC_MAP_TYPE(DataPtrType)
	JNC_MAP_TYPE(NamedType)
	JNC_MAP_TYPE(MemberBlock)
	JNC_MAP_TYPE(BaseTypeSlot)
	JNC_MAP_TYPE(DerivableType)
	JNC_MAP_TYPE(ArrayType)
	JNC_MAP_TYPE(FunctionArg)
	JNC_MAP_TYPE(FunctionType)
	JNC_MAP_TYPE(FunctionPtrType)
	JNC_MAP_TYPE(PropertyType)
	JNC_MAP_TYPE(PropertyPtrType)
	JNC_MAP_TYPE(EnumConst)
	JNC_MAP_TYPE(EnumType)
	JNC_MAP_TYPE(ClassType)
	JNC_MAP_TYPE(ClassPtrType)
	JNC_MAP_TYPE(Field)
	JNC_MAP_TYPE(StructType)
	JNC_MAP_TYPE(UnionType)
	JNC_MAP_TYPE(Alias)
	JNC_MAP_TYPE(Const)
	JNC_MAP_TYPE(Variable)
	JNC_MAP_TYPE(Function)
	JNC_MAP_TYPE(FunctionOverload)
	JNC_MAP_TYPE(Property)
	JNC_MAP_TYPE(Typedef)
	JNC_MAP_TYPE(Module)
	JNC_MAP_TYPE(Unit)
JNC_END_LIB_FUNCTION_MAP()

//..............................................................................

IfaceHdr*
JNC_CDECL
getIntrospectionClass(
	void* item,
	StdType stdType
) {
	if (!item)
		return NULL;

	GcHeap* gcHeap = getCurrentThreadRuntime()->getGcHeap();
	return gcHeap->getIntrospectionClass(item, stdType);
}

IfaceHdr*
JNC_CDECL
createIntrospectionClass(
	void* item,
	StdType stdType
) {
	if (!item)
		return NULL;

	GcHeap* gcHeap = getCurrentThreadRuntime()->getGcHeap();
	return gcHeap->createIntrospectionClass(item, stdType);
}

Function*
getFunction(OverloadableFunction function) {
	return function ?
		function->getItemKind() == ModuleItemKind_Function ?
			getFunction(function.getFunction()) :
		function->getItemKind() == ModuleItemKind_FunctionOverload ?
			getFunction(function.getFunctionOverload()->getOverload(0)) :
			NULL :
		NULL;
}

//..............................................................................

} // namespace rtl
} // namespace jnc
