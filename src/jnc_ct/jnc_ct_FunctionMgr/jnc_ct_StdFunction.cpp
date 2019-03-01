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
#include "jnc_ct_StdFunction.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

const StdItemSource*
getStdFunctionSource(StdFunc stdFunc)
{
	#include "jnc_StdFunctions.jnc.cpp"

	static StdItemSource sourceTable[StdFunc__Count] =
	{
		{                                        // StdFunc_DynamicSizeOf,
			dynamicSizeOfSrc,
			lengthof(dynamicSizeOfSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_DynamicCountOf,
			dynamicCountOfSrc,
			lengthof(dynamicCountOfSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_DynamicTypeSizeOf,
			dynamicTypeSizeOfSrc,
			lengthof(dynamicTypeSizeOfSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_DynamicFieldSizeOf,
			dynamicFieldSizeOfSrc,
			lengthof(dynamicFieldSizeOfSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_DynamicFieldCountOf,
			dynamicFieldCountOfSrc,
			lengthof(dynamicFieldCountOfSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_DynamicCastDataPtr,
			dynamicCastDataPtrSrc,
			lengthof(dynamicCastDataPtrSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_DynamicCastClassPtr,
			dynamicCastClassPtrSrc,
			lengthof(dynamicCastClassPtrSrc),
			StdNamespace_Internal,
		},
		{                                       // StdFunc_DynamicCastVariant,
			dynamicCastVariantSrc,
			lengthof(dynamicCastVariantSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_StrengthenClassPtr,
			strengthenClassPtrSrc,
			lengthof(strengthenClassPtrSrc),
			StdNamespace_Internal,
		},

		{ NULL },                                // StdFunc_PrimeStaticClass,
		{ NULL },                                // StdFunc_TryAllocateClass,
		{ NULL },                                // StdFunc_AllocateClass,
		{ NULL },                                // StdFunc_TryAllocateData,
		{ NULL },                                // StdFunc_AllocateData,
		{ NULL },                                // StdFunc_TryAllocateArray,
		{ NULL },                                // StdFunc_AllocateArray,
		{ NULL },                                // StdFunc_CreateDataPtrValidator,

		{ NULL },                                // StdFunc_GcSafePoint,
		{ NULL },                                // StdFunc_SetGcShadowStackFrameMap,
		{ NULL },                                // StdFunc_GetTls,

		{ NULL },                                // StdFunc_SetJmp,
		{ NULL },                                // StdFunc_DynamicThrow,
		{ NULL },                                // StdFunc_AsyncRet,
		{ NULL },                                // StdFunc_AsyncThrow,

		{ NULL },                                // StdFunc_VariantUnaryOperator,
		{ NULL },                                // StdFunc_VariantBinaryOperator,
		{ NULL },                                // StdFunc_VariantRelationalOperator,
		{ NULL },                                // StdFunc_VariantMemberOperator,
		{ NULL },                                // StdFunc_VariantIndexOperator,

		{ NULL },                                // StdFunc_VariantMemberProperty_get,
		{ NULL },                                // StdFunc_VariantMemberProperty_set,
		{ NULL },                                // StdFunc_VariantIndexProperty_get,
		{ NULL },                                // StdFunc_VariantIndexProperty_set,

		{                                        // StdFunc_AppendFmtLiteral_a,
			appendFmtLiteralSrc_a,
			lengthof(appendFmtLiteralSrc_a),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_AppendFmtLiteral_p,
			appendFmtLiteralSrc_p,
			lengthof(appendFmtLiteralSrc_p),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_AppendFmtLiteral_i32,
			appendFmtLiteralSrc_i32,
			lengthof(appendFmtLiteralSrc_i32),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_AppendFmtLiteral_ui32,
			appendFmtLiteralSrc_ui32,
			lengthof(appendFmtLiteralSrc_ui32),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_AppendFmtLiteral_i64,
			appendFmtLiteralSrc_i64,
			lengthof(appendFmtLiteralSrc_i64),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_AppendFmtLiteral_ui64,
			appendFmtLiteralSrc_ui64,
			lengthof(appendFmtLiteralSrc_ui64),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_AppendFmtLiteral_f,
			appendFmtLiteralSrc_f,
			lengthof(appendFmtLiteralSrc_f),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_AppendFmtLiteral_v,
			appendFmtLiteralSrc_v,
			lengthof(appendFmtLiteralSrc_v),
			StdNamespace_Internal,
		},
		{ NULL },                                // StdFunc_SimpleMulticastCall,
		{                                        // StdFunc_AssertionFailure,
			assertionFailureSrc,
			lengthof(assertionFailureSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_AddStaticDestructor,
			addStaticDestructorSrc,
			lengthof(addStaticDestructorSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_AddStaticClassDestructor,
			addStaticClassDestructorSrc,
			lengthof(addStaticClassDestructorSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_TryCheckDataPtrRangeDirect,
			tryCheckDataPtrRangeDirectSrc,
			lengthof(tryCheckDataPtrRangeDirectSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_CheckDataPtrRangeDirect,
			checkDataPtrRangeDirectSrc,
			lengthof(checkDataPtrRangeDirectSrc),
			StdNamespace_Internal,
		},
		{ NULL },                                // StdFunc_TryCheckDataPtrRangeIndirect,
		{ NULL },                                // StdFunc_CheckDataPtrRangeIndirect,
		{                                        // StdFunc_TryLazyGetDynamicLibFunctionAddr,
			tryLazyGetDynamicLibFunctionSrc,
			lengthof(tryLazyGetDynamicLibFunctionSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_LazyGetDynamicLibFunctionAddr,
			lazyGetDynamicLibFunctionSrc,
			lengthof(lazyGetDynamicLibFunctionSrc),
			StdNamespace_Internal,
		},
		{ NULL },                                // StdFunc_LlvmMemcpy
		{ NULL },                                // StdFunc_LlvmMemmove
		{ NULL },                                // StdFunc_LlvmMemset
		{                                        // StdFunc_GetDynamicField,
			getDynamicFieldSrc,
			lengthof(getDynamicFieldSrc),
			StdNamespace_Internal,
		},
	};

	ASSERT((size_t)stdFunc < StdFunc__Count);
	return &sourceTable[stdFunc];
}

//..............................................................................

ModuleItem*
LazyStdFunction::getActualItem()
{
	return m_module->m_functionMgr.getStdFunction(m_func);
}

//..............................................................................

} // namespace ct
} // namespace jnc
