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

#define _JNC_ALIAS_H

#include "jnc_Type.h"

/// \addtogroup namespace
/// @{

//..............................................................................

JNC_EXTERN_C
const char*
jnc_Alias_getInitializerString_v(jnc_Alias* alias);

JNC_EXTERN_C
bool_t
jnc_Alias_isResolved(jnc_Alias* alias);

JNC_EXTERN_C
jnc_ModuleItem*
jnc_Alias_getTargetItem(jnc_Alias* alias);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Alias: jnc_ModuleItem
{
	const char*
	getInitializerString_v()
	{
		return jnc_Alias_getInitializerString_v(this);
	}

	bool
	isResolved()
	{
		return jnc_Alias_isResolved(this);
	}

	jnc_ModuleItem*
	getTargetItem()
	{
		return jnc_Alias_getTargetItem(this);
	}
};

#endif // _JNC_CORE

//..............................................................................

/// @}
