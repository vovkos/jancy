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
jnc_ModuleItem*
jnc_Alias_getTargetItem (jnc_Alias* alias);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Alias: jnc_ModuleItem
{
	jnc_ModuleItem*
	getTargetItem ()
	{	
		return jnc_Alias_getTargetItem (this);
	}
};

#endif // _JNC_CORE

//..............................................................................

/// @}
