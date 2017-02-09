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

#define _JNC_VARIABLE_H

#include "jnc_Type.h"

/**

\defgroup variable Variable
	\ingroup module-subsystem
	\import{jnc_Variable.h}

\addtogroup variable
@{

\struct jnc_Variable
	\verbatim

	Opaque structure used as a handle to Jancy variable.

	Use functions from the :ref:`Variable <cid-variable>` to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

JNC_EXTERN_C
int
jnc_Variable_hasInitializer (jnc_Variable* variable);

JNC_EXTERN_C
const char*
jnc_Variable_getInitializerString_v (jnc_Variable* variable);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Variable: jnc_ModuleItem
{
	int
	hasInitializer ()
	{
		return jnc_Variable_hasInitializer (this);
	}

	const char*
	getInitializerString_v ()
	{
		return jnc_Variable_getInitializerString_v (this);
	}
};

#endif // _JNC_CORE

//..............................................................................

/// @}
