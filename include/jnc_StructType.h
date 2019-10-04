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

#define _JNC_STRUCTTYPE_H

#include "jnc_DerivableType.h"

/**

\defgroup struct-type Struct Type
	\ingroup type-subsystem
	\import{jnc_StructType.h}

	Struct type represents a sequence of zero or more fields.

\addtogroup struct-type
@{

\struct jnc_StructType
	\verbatim

	Opaque structure used as a handle to Jancy struct type.

	Use functions from the `Struct Type` group to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_StructType: jnc_DerivableType
{
};

#endif // _JNC_CORE

//..............................................................................

/// @}
