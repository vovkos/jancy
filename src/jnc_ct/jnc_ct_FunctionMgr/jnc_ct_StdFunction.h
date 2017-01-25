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

#include "jnc_ct_ModuleItem.h"
#include "jnc_ct_StdNamespace.h"

namespace jnc {
namespace ct {

//..............................................................................

enum StdFunc
{
	StdFunc_DynamicSizeOf,
	StdFunc_DynamicCountOf,
	StdFunc_DynamicCastDataPtr,
	StdFunc_DynamicCastClassPtr,
	StdFunc_DynamicCastVariant,
	StdFunc_StrengthenClassPtr,

	StdFunc_PrimeStaticClass,
	StdFunc_TryAllocateClass,
	StdFunc_AllocateClass,
	StdFunc_TryAllocateData,
	StdFunc_AllocateData,
	StdFunc_TryAllocateArray,
	StdFunc_AllocateArray,
	StdFunc_CreateDataPtrValidator,
	StdFunc_GcSafePoint,
	StdFunc_SetGcShadowStackFrameMap, // sets frame-map and zeroes gc-roots
	StdFunc_GetTls,

	StdFunc_SetJmp,
	StdFunc_DynamicThrow,

	StdFunc_VariantUnaryOperator,
	StdFunc_VariantBinaryOperator,
	StdFunc_VariantRelationalOperator,

	StdFunc_AppendFmtLiteral_a,
	StdFunc_AppendFmtLiteral_p,
	StdFunc_AppendFmtLiteral_i32,
	StdFunc_AppendFmtLiteral_ui32,
	StdFunc_AppendFmtLiteral_i64,
	StdFunc_AppendFmtLiteral_ui64,
	StdFunc_AppendFmtLiteral_f,
	StdFunc_AppendFmtLiteral_v,

	StdFunc_SimpleMulticastCall,

	StdFunc_AssertionFailure,
	StdFunc_AddStaticDestructor,
	StdFunc_AddStaticClassDestructor,

	StdFunc_TryCheckDataPtrRangeDirect,
	StdFunc_CheckDataPtrRangeDirect,
	StdFunc_TryCheckDataPtrRangeIndirect,
	StdFunc_CheckDataPtrRangeIndirect,
	StdFunc_TryCheckNullPtr,
	StdFunc_CheckNullPtr,
	StdFunc_CheckStackOverflow,
	StdFunc_CheckDivByZero_i32,
	StdFunc_CheckDivByZero_i64,
	StdFunc_CheckDivByZero_f32,
	StdFunc_CheckDivByZero_f64,

	StdFunc_TryLazyGetDynamicLibFunction,
	StdFunc_LazyGetDynamicLibFunction,

	StdFunc_LlvmMemcpy,
	StdFunc_LlvmMemmove,
	StdFunc_LlvmMemset,
	StdFunc__Count
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const StdItemSource*
getStdFunctionSource (StdFunc stdFunction);

//..............................................................................

class LazyStdFunction: public LazyModuleItem
{
	friend class FunctionMgr;

protected:
	StdFunc m_func;

public:
	LazyStdFunction ()
	{
		m_func = (StdFunc) -1;
	}

	virtual
	ModuleItem*
	getActualItem ();
};

//..............................................................................

} // namespace ct
} // namespace jnc
