#include "pch.h"
#include "jnc_ct_StdFunction.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

const StdItemSource*
getStdFunctionSource (StdFunc stdFunc)
{
	#include "jnc_ct_StdFunctions.jnc.cpp"

	static StdItemSource sourceTable [StdFunc__Count] =
	{
		{                                        // StdFunc_DynamicSizeOf,
			dynamicSizeOfSrc,
			lengthof (dynamicSizeOfSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_DynamicCountOf,
			dynamicCountOfSrc,
			lengthof (dynamicCountOfSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_DynamicCastDataPtr,
			dynamicCastDataPtrSrc,
			lengthof (dynamicCastDataPtrSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_DynamicCastClassPtr,
			dynamicCastClassPtrSrc,
			lengthof (dynamicCastClassPtrSrc),
			StdNamespace_Internal,
		},
		{                                       // StdFunc_DynamicCastVariant,
			dynamicCastVariantSrc,
			lengthof (dynamicCastVariantSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_StrengthenClassPtr,
			strengthenClassPtrSrc,
			lengthof (strengthenClassPtrSrc),
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
		{ NULL },                                // StdFunc_CollectGarbage,
		{ NULL },                                // StdFunc_SetGcShadowStackFrameMap,		
		{ NULL },                                // StdFunc_GetTls,
		
		{ NULL },                                // StdFunc_SetJmp,
		{ NULL },                                // StdFunc_DynamicThrow,

		{ NULL },                                // StdFunc_VariantUnaryOperator,
		{ NULL },                                // StdFunc_VariantBinaryOperator,
		{ NULL },                                // StdFunc_VariantRelationalOperator,

		{                                        // StdFunc_AppendFmtLiteral_a,
			appendFmtLiteralSrc_a,
			lengthof (appendFmtLiteralSrc_a),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_AppendFmtLiteral_p,
			appendFmtLiteralSrc_p,
			lengthof (appendFmtLiteralSrc_p),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_AppendFmtLiteral_i32,
			appendFmtLiteralSrc_i32,
			lengthof (appendFmtLiteralSrc_i32),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_AppendFmtLiteral_ui32,
			appendFmtLiteralSrc_ui32,
			lengthof (appendFmtLiteralSrc_ui32),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_AppendFmtLiteral_i64,
			appendFmtLiteralSrc_i64,
			lengthof (appendFmtLiteralSrc_i64),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_AppendFmtLiteral_ui64,
			appendFmtLiteralSrc_ui64,
			lengthof (appendFmtLiteralSrc_ui64),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_AppendFmtLiteral_f,
			appendFmtLiteralSrc_f,
			lengthof (appendFmtLiteralSrc_f),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_AppendFmtLiteral_v,
			appendFmtLiteralSrc_v,
			lengthof (appendFmtLiteralSrc_v),
			StdNamespace_Internal,
		},
		{ NULL },                                // StdFunc_SimpleMulticastCall,
		{                                        // StdFunc_AssertionFailure,
			assertionFailureSrc,
			lengthof (assertionFailureSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_AddStaticDestructor,
			addStaticDestructorSrc,
			lengthof (addStaticDestructorSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_AddStaticClassDestructor,
			addStaticClassDestructorSrc,
			lengthof (addStaticClassDestructorSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_TryCheckDataPtrRangeDirect,
			tryCheckDataPtrRangeDirectSrc,
			lengthof (tryCheckDataPtrRangeDirectSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_CheckDataPtrRangeDirect,
			checkDataPtrRangeDirectSrc,
			lengthof (checkDataPtrRangeDirectSrc),
			StdNamespace_Internal,
		},
		{ NULL },                                // StdFunc_TryCheckDataPtrRangeIndirect,
		{ NULL },                                // StdFunc_CheckDataPtrRangeIndirect,
		{                                        // StdFunc_TryCheckNullPtr,
			tryCheckNullPtrSrc,
			lengthof (tryCheckNullPtrSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_CheckNullPtr,
			checkNullPtrSrc,
			lengthof (checkNullPtrSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_CheckStackOverflow,
			checkStackOverflowSrc,
			lengthof (checkStackOverflowSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_CheckDivByZero_i32,
			checkDivByZeroSrc_i32,
			lengthof (checkDivByZeroSrc_i32),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_CheckDivByZero_i64,
			checkDivByZeroSrc_i64,
			lengthof (checkDivByZeroSrc_i64),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_CheckDivByZero_f32,
			checkDivByZeroSrc_f32,
			lengthof (checkDivByZeroSrc_f32),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_CheckDivByZero_f64,
			checkDivByZeroSrc_f64,
			lengthof (checkDivByZeroSrc_f64),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_TryLazyGetDynamicLibFunctionAddr,
			tryLazyGetDynamicLibFunctionSrc,
			lengthof (tryLazyGetDynamicLibFunctionSrc),
			StdNamespace_Internal,
		},
		{                                        // StdFunc_LazyGetDynamicLibFunctionAddr,
			lazyGetDynamicLibFunctionSrc,
			lengthof (lazyGetDynamicLibFunctionSrc),
			StdNamespace_Internal,
		},
		{ NULL },                               // StdFunc_LlvmMemcpy
		{ NULL },                               // StdFunc_LlvmMemmove
		{ NULL },                               // StdFunc_LlvmMemset
	};

	ASSERT ((size_t) stdFunc < StdFunc__Count);
	return &sourceTable [stdFunc];
}

//.............................................................................
	
ModuleItem*
LazyStdFunction::getActualItem ()
{
	return m_module->m_functionMgr.getStdFunction (m_func);
}

//.............................................................................

} // namespace ct
} // namespace jnc
