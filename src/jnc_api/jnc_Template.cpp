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
#include "jnc_Template.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_ct_TemplateMgr.h"
#endif

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
size_t
jnc_Template_getArgCount(jnc_Template* templ) {
	return templ->getArgArray().getCount();
}

JNC_EXTERN_C
jnc_Type*
jnc_Template_getArg(
	jnc_Template* templ,
	size_t index
) {
	return templ->getArgArray()[index];
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
