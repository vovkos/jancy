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

#define _JNC_UNIT_H

#include "jnc_Def.h"

/**

\defgroup unit Unit
	\ingroup module-subsystem
	\import{jnc_Unit.h}

\addtogroup unit
@{
*/

//..............................................................................

JNC_EXTERN_C
jnc_ExtensionLib*
jnc_Unit_getLib(jnc_Unit* unit);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Unit
{
	jnc_ExtensionLib*
	getLib()
	{
		return jnc_Unit_getLib(this);
	}
};

#endif // _JNC_CORE

//..............................................................................

/// @}
