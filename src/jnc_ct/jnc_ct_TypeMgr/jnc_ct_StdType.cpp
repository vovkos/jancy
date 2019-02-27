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
		{ NULL },                            // StdType_DynamicArrayBox,
		{ NULL },                            // StdType_DynamicArrayBoxPtr,
		{ NULL },                            // StdType_StaticDataBox,
		{ NULL },                            // StdType_StaticDataBoxPtr,
		{ NULL },                            // StdType_AbstractClass,
		{ NULL },                            // StdType_AbstractClassPtr,
		{ NULL },                            // StdType_AbstractData,
		{ NULL },                            // StdType_AbstractDataPtr,
		{ NULL },                            // StdType_SimpleFunction,
		{ NULL },                            // StdType_SimpleMulticast,
		{ NULL },                            // StdType_SimpleEventPtr,
		{ NULL },                            // StdType_Binder,
		{                                    // StdType_Scheduler,
			schedulerTypeSrc,
			lengthof(schedulerTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_RegexMatch,
			regexMatchTypeSrc,
			lengthof(regexMatchTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_RegexState,
			regexStateTypeSrc,
			lengthof(regexStateTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_RegexDfa,
			regexDfaTypeSrc,
			lengthof(regexDfaTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_Promise,
			promiseTypeSrc,
			lengthof(promiseTypeSrc),
			StdNamespace_Jnc,
		},
		{ NULL},                             // StdType_PromisePtr,
		{                                    // StdType_Promisifier,
			promisifierTypeSrc,
			lengthof(promisifierTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_DynamicLib,
			dynamicLibTypeSrc,
			lengthof(dynamicLibTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_DynamicLayout,
			dynamicLayoutTypeSrc,
			lengthof(dynamicLayoutTypeSrc),
			StdNamespace_Internal,
		},
		{                                    // StdType_FmtLiteral,
			fmtLiteralTypeSrc,
			lengthof(fmtLiteralTypeSrc),
			StdNamespace_Internal,
		},
		{                                    // StdType_Int64Int64,
			int64Int64TypeSrc,
			lengthof(int64Int64TypeSrc),
			StdNamespace_Internal,
		},
		{                                    // StdType_Fp64Fp64,
			fp64Fp64TypeSrc,
			lengthof(fp64Fp64TypeSrc),
			StdNamespace_Internal,
		},
		{                                    // StdType_Int64Fp64,
			int64Fp64TypeSrc,
			lengthof(int64Fp64TypeSrc),
			StdNamespace_Internal,
		},
		{                                    // StdType_Fp64Int64,
			fp64Int64TypeSrc,
			lengthof(fp64Int64TypeSrc),
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
