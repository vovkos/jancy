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
	#include "jnc_DynamicLib.jnc.cpp"
	#include "jnc_Regex.jnc.cpp"
	#include "jnc_Promise.jnc.cpp"

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
		{                                    // jnc_StdType_GcTriggers,
			gcTriggersSrc,
			lengthof(gcTriggersSrc),
			StdNamespace_Jnc,
		},
		{                                    // jnc_StdType_GcStats,
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
