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

const char*
getStdTypeName(StdType stdType) {
	static const char* nameTable[StdType__Count] = {
		NULL,  // StdType_ByteThinPtr,
		NULL,  // StdType_CharConstThinPtr,
		NULL,  // StdType_CharConstPtr,
		NULL,  // StdType_IfaceHdr,
		NULL,  // StdType_IfaceHdrPtr,
		NULL,  // StdType_Box,
		NULL,  // StdType_BoxPtr,
		NULL,  // StdType_DataBox,
		NULL,  // StdType_DataBoxPtr,
		NULL,  // StdType_DetachedDataBox,
		NULL,  // StdType_DetachedDataBoxPtr,
		NULL,  // StdType_AbstractClass,
		NULL,  // StdType_AbstractClassPtr,
		NULL,  // StdType_AbstractData,
		NULL,  // StdType_AbstractDataPtr,
		NULL,  // StdType_SimpleFunction,
		NULL,  // StdType_SimpleMulticast,
		NULL,  // StdType_SimpleEventPtr,
		NULL,  // StdType_Binder,
		NULL,  // StdType_DataPtrValidator,
		NULL,  // StdType_DataPtrValidatorPtr,
		NULL,  // StdType_DataPtrStruct,
		NULL,  // StdType_PropertyPtrStruct = StdType_FunctionPtrStruct,
		NULL,  // StdType_VariantStruct,
		NULL,  // StdType_StringStruct,
		NULL,  // StdType_GcShadowStackFrame,
		NULL,  // StdType_SjljFrame,
		NULL,  // StdType_ReactorBase,
		NULL,  // StdType_ReactorClosure,
		NULL,  // StdType_DynamicLayout,
		NULL,  // StdType_FmtLiteral,
		NULL,  // StdType_Int64Int64,
		NULL,  // StdType_Fp64Fp64,
		NULL,  // StdType_Int64Fp64,
		NULL,  // StdType_Fp64Int64,

		// jnc_rtl_core lazy types

		"GcTriggers",             // StdType_GcTriggers,
		"GcStats",                // StdType_GcStats,
		"Scheduler",              // StdType_Scheduler,
		NULL,                     // StdType_SchedulerPtr,
		"RegexCapture",           // StdType_RegexCapture,
		"RegexMatch",             // StdType_RegexMatch,
		"RegexState",             // StdType_RegexState,
		"RegexFlags",             // StdType_RegexFlags,
		"RegexExecFlags",         // StdType_RegexExecFlags,
		"Regex",                  // StdType_Regex,
		"Promise",                // StdType_Promise,
		NULL,                     // StdType_PromisePtr,
		"Promisifier",            // StdType_Promisifier,
		"DynamicLib",             // StdType_DynamicLib,

		// jnc_rtl_intro lazy types

		"ModuleItem",             // StdType_ModuleItem
		"ModuleItemDecl",         // StdType_ModuleItemDecl
		"ModuleItemInitializer",  // StdType_ModuleItemInitializer
		"Attribute",              // StdType_Attribute
		"AttributeBlock",         // StdType_AttributeBlock
		"Namespace",              // StdType_Namespace
		"GlobalNamespace",        // StdType_GlobalNamespace
		"Type",                   // StdType_Type
		"DataPtrType",            // StdType_DataPtrType
		"NamedType",              // StdType_NamedType
		"MemberBlock",            // StdType_MemberBlock
		"BaseTypeSlot",           // StdType_BaseTypeSlot
		"DerivableType",          // StdType_DerivableType
		"ArrayType",              // StdType_ArrayType
		"FunctionArg",            // StdType_FunctionArg
		"FunctionType",           // StdType_FunctionType
		"FunctionPtrType",        // StdType_FunctionPtrType
		"PropertyType",           // StdType_PropertyType
		"PropertyPtrType",        // StdType_PropertyPtrType
		"EnumConst",              // StdType_EnumConst
		"EnumType",               // StdType_EnumType
		"ClassType",              // StdType_ClassType
		"ClassPtrType",           // StdType_ClassPtrType
		"Field",                  // StdType_Field
		"StructType",             // StdType_StructType
		"UnionType",              // StdType_UnionType
		"Alias",                  // StdType_Alias
		"Const",                  // StdType_Const
		"Variable",               // StdType_Variable
		"Function",               // StdType_Function
		"FunctionOverload",       // StdType_FunctionOverload
		"Property",               // StdType_Property
		"Typedef",                // StdType_Typedef
		"Module",                 // StdType_Module
		"Unit",                   // StdType_Unit
	};

	ASSERT((size_t)stdType < StdType__Count);
	return nameTable[stdType];
}

const StdItemSource*
getStdTypeSource(StdType stdType) {
	#include "jnc_StdTypes.jnc.cpp"

	static StdItemSource sourceTable[StdType__Count] = {
		{ NULL },  // StdType_ByteThinPtr,
		{ NULL },  // StdType_CharConstThinPtr,
		{ NULL },  // StdType_CharConstPtr,
		{ NULL },  // StdType_IfaceHdr,
		{ NULL },  // StdType_IfaceHdrPtr,
		{ NULL },  // StdType_Box,
		{ NULL },  // StdType_BoxPtr,
		{ NULL },  // StdType_DataBox,
		{ NULL },  // StdType_DataBoxPtr,
		{ NULL },  // StdType_DetachedDataBox,
		{ NULL },  // StdType_DetachedDataBoxPtr,
		{ NULL },  // StdType_AbstractClass,
		{ NULL },  // StdType_AbstractClassPtr,
		{ NULL },  // StdType_AbstractData,
		{ NULL },  // StdType_AbstractDataPtr,
		{ NULL },  // StdType_SimpleFunction,
		{ NULL },  // StdType_SimpleMulticast,
		{ NULL },  // StdType_SimpleEventPtr,
		{ NULL },  // StdType_Binder,
		{ NULL },  // StdType_DataPtrValidator,
		{ NULL },  // StdType_DataPtrValidatorPtr,
		{ NULL },  // StdType_DataPtrStruct,
		{ NULL },  // StdType_PropertyPtrStruct = StdType_FunctionPtrStruct,
		{ NULL },  // StdType_VariantStruct,
		{ NULL },  // StdType_StringStruct,
		{ NULL },  // StdType_GcShadowStackFrame,
		{ NULL },  // StdType_SjljFrame,
		{ NULL },  // StdType_ReactorBase,
		{ NULL },  // StdType_ReactorClosure,

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

		{ NULL },  // StdType_GcTriggers,
		{ NULL },  // StdType_GcStats,
		{ NULL },  // StdType_Scheduler,
		{ NULL },  // StdType_SchedulerPtr,
		{ NULL },  // StdType_RegexCapture,
		{ NULL },  // StdType_RegexMatch,
		{ NULL },  // StdType_RegexState,
		{ NULL },  // StdType_RegexFlags,
		{ NULL },  // StdType_RegexExecFlags,
		{ NULL },  // StdType_Regex,
		{ NULL },  // StdType_Promise,
		{ NULL },  // StdType_PromisePtr,
		{ NULL },  // StdType_Promisifier,
		{ NULL },  // StdType_DynamicLib,

		// jnc_rtl_intro lazy types

		{ NULL },  // StdType_ModuleItem
		{ NULL },  // StdType_ModuleItemDecl
		{ NULL },  // StdType_ModuleItemInitializer
		{ NULL },  // StdType_Attribute
		{ NULL },  // StdType_AttributeBlock
		{ NULL },  // StdType_Namespace
		{ NULL },  // StdType_GlobalNamespace
		{ NULL },  // StdType_Type
		{ NULL },  // StdType_DataPtrType
		{ NULL },  // StdType_NamedType
		{ NULL },  // StdType_MemberBlock
		{ NULL },  // StdType_BaseTypeSlot
		{ NULL },  // StdType_DerivableType
		{ NULL },  // StdType_ArrayType
		{ NULL },  // StdType_FunctionArg
		{ NULL },  // StdType_FunctionType
		{ NULL },  // StdType_FunctionPtrType
		{ NULL },  // StdType_PropertyType
		{ NULL },  // StdType_PropertyPtrType
		{ NULL },  // StdType_EnumConst
		{ NULL },  // StdType_EnumType
		{ NULL },  // StdType_ClassType
		{ NULL },  // StdType_ClassPtrType
		{ NULL },  // StdType_Field
		{ NULL },  // StdType_StructType
		{ NULL },  // StdType_UnionType
		{ NULL },  // StdType_Alias
		{ NULL },  // StdType_Const
		{ NULL },  // StdType_Variable
		{ NULL },  // StdType_Function
		{ NULL },  // StdType_FunctionOverload
		{ NULL },  // StdType_Property
		{ NULL },  // StdType_Typedef
		{ NULL },  // StdType_Module
		{ NULL },  // StdType_Unit
	};

	ASSERT((size_t)stdType < StdType__Count);
	return &sourceTable[stdType];
}

//..............................................................................

} // namespace ct
} // namespace jnc
