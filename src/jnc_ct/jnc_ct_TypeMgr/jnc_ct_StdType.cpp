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
#include "jnc_ct_StdType.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

const StdItemSource*
getStdTypeSource(StdType stdType)
{
	#include "jnc_StdTypes.jnc.cpp"
	#include "jnc_GcTriggers.jnc.cpp"
	#include "jnc_GcStats.jnc.cpp"
	#include "jnc_Scheduler.jnc.cpp"
	#include "jnc_Regex.jnc.cpp"
	#include "jnc_Promise.jnc.cpp"
	#include "jnc_DynamicLib.jnc.cpp"
	#include "jnc_ModuleItem.jnc.cpp"
	#include "jnc_AttributeBlock.jnc.cpp"
	#include "jnc_Field.jnc.cpp"
	#include "jnc_Namespace.jnc.cpp"
	#include "jnc_OpKind.jnc.cpp"
	#include "jnc_Type.jnc.cpp"
	#include "jnc_MemberBlock.jnc.cpp"
	#include "jnc_DerivableType.jnc.cpp"
	#include "jnc_ArrayType.jnc.cpp"
	#include "jnc_BitFieldType.jnc.cpp"
	#include "jnc_FunctionType.jnc.cpp"
	#include "jnc_PropertyType.jnc.cpp"
	#include "jnc_EnumType.jnc.cpp"
	#include "jnc_ClassType.jnc.cpp"
	#include "jnc_StructType.jnc.cpp"
	#include "jnc_UnionType.jnc.cpp"
	#include "jnc_Alias.jnc.cpp"
	#include "jnc_Variable.jnc.cpp"
	#include "jnc_Function.jnc.cpp"
	#include "jnc_Property.jnc.cpp"
	#include "jnc_Module.jnc.cpp"

	static StdItemSource sourceTable[StdType__Count] =
	{
		{ NULL },                            // StdType_BytePtr,
		{ NULL },                            // StdType_CharConstPtr,
		{ NULL },                            // StdType_IfaceHdr,
		{ NULL },                            // StdType_IfaceHdrPtr,
		{ NULL },                            // StdType_Box,
		{ NULL },                            // StdType_BoxPtr,
		{ NULL },                            // StdType_DataBox,
		{ NULL },                            // StdType_DataBoxPtr,
		{ NULL },                            // StdType_DetachedDataBox,
		{ NULL },                            // StdType_DetachedDataBoxPtr,
		{ NULL },                            // StdType_AbstractClass,
		{ NULL },                            // StdType_AbstractClassPtr,
		{ NULL },                            // StdType_AbstractData,
		{ NULL },                            // StdType_AbstractDataPtr,
		{ NULL },                            // StdType_SimpleFunction,
		{ NULL },                            // StdType_SimpleMulticast,
		{ NULL },                            // StdType_SimpleEventPtr,
		{ NULL },                            // StdType_Binder,
		{ NULL },                            // StdType_DataPtrValidator,
		{ NULL },                            // StdType_DataPtrValidatorPtr,
		{ NULL },                            // StdType_DataPtrStruct,
		{ NULL },                            // StdType_PropertyPtrStruct = StdType_FunctionPtrStruct,
		{ NULL },                            // StdType_VariantStruct,
		{ NULL },                            // StdType_GcShadowStackFrame,
		{ NULL },                            // StdType_SjljFrame,
		{ NULL },                            // StdType_ReactorBase,
		{ NULL },                            // StdType_ReactorClosure,

		// these types should be redefined without source

		{                                    // StdType_DynamicLayout,
			dynamicLayoutSrc,
			lengthof(dynamicLayoutSrc),
			StdNamespace_Internal,
		},
		{                                    // StdType_FmtLiteral,
			fmtLiteralSrc,
			lengthof(fmtLiteralSrc),
			StdNamespace_Internal,
		},
		{                                    // StdType_Int64Int64,
			int64Int64Src,
			lengthof(int64Int64Src),
			StdNamespace_Internal,
		},
		{                                    // StdType_Fp64Fp64,
			fp64Fp64Src,
			lengthof(fp64Fp64Src),
			StdNamespace_Internal,
		},
		{                                    // StdType_Int64Fp64,
			int64Fp64Src,
			lengthof(int64Fp64Src),
			StdNamespace_Internal,
		},
		{                                    // StdType_Fp64Int64,
			fp64Int64Src,
			lengthof(fp64Int64Src),
			StdNamespace_Internal,
		},

		// jnc_rtl_core lazy types

		{                                    // StdType_GcTriggers,
			gcTriggersSrc,
			lengthof(gcTriggersSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_GcStats,
			gcStatsSrc,
			lengthof(gcStatsSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_Scheduler,
			schedulerSrc,
			lengthof(schedulerSrc),
			StdNamespace_Jnc,
		},
		{ NULL},                             // StdType_SchedulerPtr,
		{                                    // StdType_RegexMatch,
			regexMatchSrc,
			lengthof(regexMatchSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_RegexState,
			regexStateSrc,
			lengthof(regexStateSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_RegexDfa,
			regexDfaSrc,
			lengthof(regexDfaSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_Promise,
			promiseSrc,
			lengthof(promiseSrc),
			StdNamespace_Jnc,
		},
		{ NULL},                             // StdType_PromisePtr,
		{                                    // StdType_Promisifier,
			promisifierSrc,
			lengthof(promisifierSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_DynamicLib,
			dynamicLibSrc,
			lengthof(dynamicLibSrc),
			StdNamespace_Jnc,
		},

		// jnc_rtl_intro lazy types

		{                                    // StdType_ModuleItemKind,
			moduleItemKindSrc,
			lengthof(moduleItemKindSrc),
			StdNamespace_Jnc,
		},

		{                                    // StdType_ModuleItemFlags,
			moduleItemFlagsSrc,
			lengthof(moduleItemFlagsSrc),
			StdNamespace_Jnc,
		},

		{                                    // StdType_StorageKind,
			storageKindSrc,
			lengthof(storageKindSrc),
			StdNamespace_Jnc,
		},

		{                                    // StdType_AccessKind,
			accessKindSrc,
			lengthof(accessKindSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_ModuleItem
			moduleItemSrc,
			lengthof(moduleItemSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_ModuleItemDecl
			moduleItemDeclSrc,
			lengthof(moduleItemDeclSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_ModuleItemInitializer
			moduleItemInitializerSrc,
			lengthof(moduleItemInitializerSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_Attribute
			attributeSrc,
			lengthof(attributeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_AttributeBlock
			attributeBlockSrc,
			lengthof(attributeBlockSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_NamespaceKind
			namespaceKindSrc,
			lengthof(namespaceKindSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_Namespace
			namespaceSrc,
			lengthof(namespaceSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_GlobalNamespace
			globalNamespaceSrc,
			lengthof(globalNamespaceSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_UnOpKind
			unOpKindSrc,
			lengthof(unOpKindSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_BinOpKind
			binOpKindSrc,
			lengthof(binOpKindSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_TypeKind
			typeKindSrc,
			lengthof(typeKindSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_TypeKindFlags
			typeKindFlagsSrc,
			lengthof(typeKindFlagsSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_TypeFlags
			typeFlagsSrc,
			lengthof(typeFlagsSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_PtrTypeFlags
			ptrTypeFlagsSrc,
			lengthof(ptrTypeFlagsSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_DataPtrTypeKind
			dataPtrTypeKindSrc,
			lengthof(dataPtrTypeKindSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_Type
			typeSrc,
			lengthof(typeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_DataPtrType
			dataPtrTypeSrc,
			lengthof(dataPtrTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_NamedType
			namedTypeSrc,
			lengthof(namedTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_MemberBlock
			memberBlockSrc,
			lengthof(memberBlockSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_BaseTypeSlot
			baseTypeSlotSrc,
			lengthof(baseTypeSlotSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_DerivableType
			derivableTypeSrc,
			lengthof(derivableTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_ArrayType
			arrayTypeSrc,
			lengthof(arrayTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_BitFieldType
			bitFieldTypeSrc,
			lengthof(bitFieldTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_FunctionTypeFlags
			functionTypeFlagsSrc,
			lengthof(functionTypeFlagsSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_FunctionPtrTypeKind
			functionPtrTypeKindSrc,
			lengthof(functionPtrTypeKindSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_FunctionArg
			functionArgSrc,
			lengthof(functionArgSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_FunctionType
			functionTypeSrc,
			lengthof(functionTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_FunctionPtrType
			functionPtrTypeSrc,
			lengthof(functionPtrTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_PropertyTypeFlags
			propertyTypeFlagsSrc,
			lengthof(propertyTypeFlagsSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_PropertyPtrTypeKind
			propertyPtrTypeKindSrc,
			lengthof(propertyPtrTypeKindSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_PropertyType
			propertyTypeSrc,
			lengthof(propertyTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_PropertyPtrType
			propertyPtrTypeSrc,
			lengthof(propertyPtrTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_EnumTypeFlags
			enumTypeFlagsSrc,
			lengthof(enumTypeFlagsSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_EnumConst
			enumConstSrc,
			lengthof(enumConstSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_EnumType
			enumTypeSrc,
			lengthof(enumTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_ClassTypeKind
			classTypeKindSrc,
			lengthof(classTypeKindSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_ClassTypeFlags
			classTypeFlagsSrc,
			lengthof(classTypeFlagsSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_ClassPtrTypeKind
			classPtrTypeKindSrc,
			lengthof(classPtrTypeKindSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_ClassType
			classTypeSrc,
			lengthof(classTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_ClassPtrType
			classPtrTypeSrc,
			lengthof(classPtrTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_StructTypeKind
			structTypeKindSrc,
			lengthof(structTypeKindSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_Field
			fieldSrc,
			lengthof(fieldSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_StructType
			structTypeSrc,
			lengthof(structTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_UnionType
			unionTypeSrc,
			lengthof(unionTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_Alias
			aliasSrc,
			lengthof(aliasSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_Const
			constSrc,
			lengthof(constSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_Variable
			variableSrc,
			lengthof(variableSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_FunctionKind
			functionKindSrc,
			lengthof(functionKindSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_FunctionKindFlags
			functionKindFlagsSrc,
			lengthof(functionKindFlagsSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_Function
			functionSrc,
			lengthof(functionSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_PropertyKind
			propertyKindSrc,
			lengthof(propertyKindSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_PropertyFlag
			propertyFlagSrc,
			lengthof(propertyFlagSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_Property
			propertySrc,
			lengthof(propertySrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_Typedef
			typedefSrc,
			lengthof(typedefSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_ModuleCompileFlags
			moduleCompileFlagsSrc,
			lengthof(moduleCompileFlagsSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_ModuleCompileState
			moduleCompileStateSrc,
			lengthof(moduleCompileStateSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_Module
			moduleSrc,
			lengthof(moduleSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_Unit
			unitSrc,
			lengthof(unitSrc),
			StdNamespace_Jnc,
		},
	};

	ASSERT((size_t)stdType < StdType__Count);
	return &sourceTable[stdType];
}

//..............................................................................

ModuleItem*
LazyStdType::getActualItem()
{
	return m_module->m_typeMgr.getStdType(m_stdType);
}

//..............................................................................

} // namespace ct
} // namespace jnc
