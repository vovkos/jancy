o//..............................................................................
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
#include "jnc_Promise.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_ct_Module.h"
#	include "jnc_rt_Runtime.h"
#	include "jnc_rtl_Promise.h"
#endif

#include "jnc_Runtime.h"

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

void
JNC_CDECL
jnc_Promise_complete(
	jnc_Promise* promise,
	jnc_Variant result,
	jnc_DataPtr errorPtr
	)
{
	return jnc_g_dynamicExtensionLibHost->m_promiseFuncTable->m_completeFunc(promise, result, errorPtr);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

void
JNC_CDECL
jnc_Promise_complete(
	jnc_Promise* promise,
	jnc_Variant result,
	jnc_DataPtr errorPtr
	)
{
	((jnc::rtl::PromiseImpl*)promise)->complete_2(result, errorPtr);
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
