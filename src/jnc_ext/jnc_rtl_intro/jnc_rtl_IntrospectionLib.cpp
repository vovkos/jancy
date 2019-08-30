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
#include "jnc_rtl_Type.h"
#include "jnc_rtl_ArrayType.h"
#include "jnc_rtl_BitFieldType.h"
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
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(NamedTypeBlock)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(BaseTypeSlot)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(DerivableType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(ArrayType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(BitFieldType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(FunctionType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(FunctionPtrType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(PropertyType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(PropertyPtrType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(EnumConst)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(EnumType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(ClassType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(ClassPtrType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(StructField)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(StructType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(UnionType)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Alias)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Variable)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Function)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Property)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Module)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Unit)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE()

JNC_BEGIN_LIB_FUNCTION_MAP(jnc_IntrospectionLib)
	JNC_MAP_STD_TYPE(StdType_ModuleItem, ModuleItem)
	JNC_MAP_STD_TYPE(StdType_ModuleItemDecl, ModuleItemDecl)
	JNC_MAP_STD_TYPE(StdType_ModuleItemInitializer, ModuleItemInitializer)
	JNC_MAP_STD_TYPE(StdType_Attribute, Attribute)
	JNC_MAP_STD_TYPE(StdType_AttributeBlock, AttributeBlock)
	JNC_MAP_STD_TYPE(StdType_Namespace, Namespace)
	JNC_MAP_STD_TYPE(StdType_GlobalNamespace, GlobalNamespace)
	JNC_MAP_STD_TYPE(StdType_Type, Type)
	JNC_MAP_STD_TYPE(StdType_DataPtrType, DataPtrType)
	JNC_MAP_STD_TYPE(StdType_NamedType, NamedType)
	JNC_MAP_STD_TYPE(StdType_NamedTypeBlock, NamedTypeBlock)
	JNC_MAP_STD_TYPE(StdType_BaseTypeSlot, BaseTypeSlot)
	JNC_MAP_STD_TYPE(StdType_DerivableType, DerivableType)
	JNC_MAP_STD_TYPE(StdType_ArrayType, ArrayType)
	JNC_MAP_STD_TYPE(StdType_BitFieldType, BitFieldType)
	JNC_MAP_STD_TYPE(StdType_FunctionType, FunctionType)
	JNC_MAP_STD_TYPE(StdType_FunctionPtrType, FunctionPtrType)
	JNC_MAP_STD_TYPE(StdType_PropertyType, PropertyType)
	JNC_MAP_STD_TYPE(StdType_PropertyPtrType, PropertyPtrType)
	JNC_MAP_STD_TYPE(StdType_EnumConst, EnumConst)
	JNC_MAP_STD_TYPE(StdType_EnumType, EnumType)
	JNC_MAP_STD_TYPE(StdType_ClassType, ClassType)
	JNC_MAP_STD_TYPE(StdType_ClassPtrType, ClassPtrType)
	JNC_MAP_STD_TYPE(StdType_StructField, StructField)
	JNC_MAP_STD_TYPE(StdType_StructType, StructType)
	JNC_MAP_STD_TYPE(StdType_UnionType, UnionType)
	JNC_MAP_STD_TYPE(StdType_Alias, Alias)
	JNC_MAP_STD_TYPE(StdType_Variable, Variable)
	JNC_MAP_STD_TYPE(StdType_Function, Function)
	JNC_MAP_STD_TYPE(StdType_Property, Property)
	JNC_MAP_STD_TYPE(StdType_Module, Module)
	JNC_MAP_STD_TYPE(StdType_Unit, Unit)
JNC_END_LIB_FUNCTION_MAP()

//..............................................................................

IfaceHdr*
JNC_CDECL
getIntrospectionClass(
	void* item,
	StdType stdType
	)
{
	if (!item)
		return NULL;

	GcHeap* gcHeap = getCurrentThreadRuntime()->getGcHeap();
	return gcHeap->getIntrospectionClass(item, stdType);
}

//..............................................................................

} // namespace rtl
} // namespace jnc
