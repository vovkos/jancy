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

enum StdFunc {
	StdFunc_DynamicSizeOf,
	StdFunc_DynamicCountOf,
	StdFunc_DynamicTypeSizeOf,
	StdFunc_DynamicFieldSizeOf,
	StdFunc_DynamicFieldCountOf,
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
	StdFunc_SetGcShadowStackFrameMap,
	StdFunc_GetTls,

	StdFunc_SetJmp,
	StdFunc_SaveSignalInfo,
	StdFunc_DynamicThrow,
	StdFunc_AsyncRet,
	StdFunc_AsyncThrow,

	StdFunc_VariantUnaryOperator,
	StdFunc_VariantBinaryOperator,
	StdFunc_VariantRelationalOperator,
	StdFunc_VariantMemberOperator,
	StdFunc_VariantIndexOperator,

	StdFunc_VariantMemberProperty_get,
	StdFunc_VariantMemberProperty_set,
	StdFunc_VariantIndexProperty_get,
	StdFunc_VariantIndexProperty_set,

	StdFunc_StringConstruct,
	StdFunc_StringCreate,
	StdFunc_StringSz,
	StdFunc_StringRefSz,
	StdFunc_StringEq,
	StdFunc_StringCmp,

	StdFunc_AppendFmtLiteral_a,
	StdFunc_AppendFmtLiteral_p,
	StdFunc_AppendFmtLiteral_i32,
	StdFunc_AppendFmtLiteral_ui32,
	StdFunc_AppendFmtLiteral_i64,
	StdFunc_AppendFmtLiteral_ui64,
	StdFunc_AppendFmtLiteral_f,
	StdFunc_AppendFmtLiteral_v,
	StdFunc_AppendFmtLiteral_s,
	StdFunc_AppendFmtLiteral_re,

	StdFunc_SimpleMulticastCall,

	StdFunc_AssertionFailure,
	StdFunc_AddStaticDestructor,
	StdFunc_AddStaticClassDestructor,

	StdFunc_TryCheckDataPtrRangeDirect,
	StdFunc_CheckDataPtrRangeDirect,
	StdFunc_TryCheckDataPtrRangeIndirect,
	StdFunc_CheckDataPtrRangeIndirect,

	StdFunc_TryLazyGetDynamicLibFunction,
	StdFunc_LazyGetDynamicLibFunction,

	StdFunc_LlvmMemcpy,
	StdFunc_LlvmMemmove,
	StdFunc_LlvmMemset,

	StdFunc_GetDynamicField,

	StdFunc__Count
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const StdItemSource*
getStdFunctionSource(StdFunc stdFunction);

//..............................................................................

enum StdProp {
	StdProp_VariantMember,
	StdProp_VariantIndex,

	StdProp__Count,
};

//..............................................................................

#define JNC_MAP_STD_FUNCTION(stdFunc, proc) \
	if (module->m_functionMgr.isStdFunctionUsed(stdFunc)) { \
		function = module->m_functionMgr.getStdFunction(stdFunc); \
		ASSERT(function); \
		JNC_MAP_FUNCTION_IMPL(function, proc); \
	}

#define JNC_MAP_STD_PROPERTY(stdProp, getter, setter) \
	if (module->m_functionMgr.isStdPropertyUsed(stdProp)) { \
		prop = module->m_functionMgr.getStdProperty(stdProp); \
		ASSERT(prop); \
		JNC_MAP_PROPERTY_GETTER(prop, getter); \
		JNC_MAP_PROPERTY_SETTER(prop, setter); \
	}

//..............................................................................

} // namespace ct
} // namespace jnc
