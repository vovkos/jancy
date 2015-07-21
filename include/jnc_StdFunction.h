// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ModuleItem.h"
#include "jnc_StdNamespace.h"

namespace jnc {

//.............................................................................

enum StdFunction
{
	StdFunction_DynamicSizeOf,
	StdFunction_DynamicCountOf,
	StdFunction_DynamicCastDataPtr,
	StdFunction_DynamicCastClassPtr,
	StdFunction_DynamicCastVariant,
	StdFunction_StrengthenClassPtr,
	
	StdFunction_PrimeStaticClass,	
	StdFunction_TryAllocateClass,
	StdFunction_AllocateClass,
	StdFunction_TryAllocateData,
	StdFunction_AllocateData,
	StdFunction_TryAllocateArray,
	StdFunction_AllocateArray,

	StdFunction_CollectGarbage,
	StdFunction_GetCurrentThreadId,
	StdFunction_CreateThread,
	StdFunction_Sleep,
	StdFunction_GetTimestamp,
	StdFunction_Format,
	StdFunction_StrLen,
	StdFunction_StrCmp,
	StdFunction_StriCmp,
	StdFunction_StrChr,
	StdFunction_StrCat,
	StdFunction_StrDup,
	StdFunction_MemCmp,
	StdFunction_MemChr,
	StdFunction_MemCpy,
	StdFunction_MemSet,
	StdFunction_MemCat,
	StdFunction_MemDup,
	StdFunction_Rand,
	StdFunction_Printf,
	StdFunction_Atoi,
	StdFunction_GetTls,

	StdFunction_AppendFmtLiteral_a,
	StdFunction_AppendFmtLiteral_p,
	StdFunction_AppendFmtLiteral_i32,
	StdFunction_AppendFmtLiteral_ui32,
	StdFunction_AppendFmtLiteral_i64,
	StdFunction_AppendFmtLiteral_ui64,
	StdFunction_AppendFmtLiteral_f,
	StdFunction_AppendFmtLiteral_v,
	StdFunction_AppendFmtLiteral_s,
	StdFunction_AppendFmtLiteral_sr,
	StdFunction_AppendFmtLiteral_cb,
	StdFunction_AppendFmtLiteral_cbr,
	StdFunction_AppendFmtLiteral_br,
	
	StdFunction_SimpleMulticastCall,
	StdFunction_Throw,
	StdFunction_GetLastError,
	StdFunction_SetPosixError,
	StdFunction_SetStringError,
	StdFunction_AssertionFailure,
	StdFunction_AddStaticDestructor,
	StdFunction_AddStaticClassDestructor,
	StdFunction_TryCheckDataPtrRangeDirect,
	StdFunction_CheckDataPtrRangeDirect,
	StdFunction_TryCheckDataPtrRangeIndirect,
	StdFunction_CheckDataPtrRangeIndirect,
	StdFunction_TryCheckNullPtr,
	StdFunction_CheckNullPtr,
	StdFunction_TryLazyGetLibraryFunction,
	StdFunction_LazyGetLibraryFunction,
	StdFunction_LlvmMemcpy,
	StdFunction_LlvmMemmove,
	StdFunction_LlvmMemset,
	StdFunction__Count
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getStdFunctionName (StdFunction stdFunction);

const StdItemSource*
getStdFunctionSource (StdFunction stdFunction);

//.............................................................................

class LazyStdFunction: public LazyModuleItem
{
	friend class FunctionMgr;

protected:
	StdFunction m_func;

public:
	LazyStdFunction ()
	{
		m_func = (StdFunction) -1;
	}

	virtual
	ModuleItem*
	getActualItem ();
};

//.............................................................................

} // namespace jnc {
