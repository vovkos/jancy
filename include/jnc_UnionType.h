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

#define _JNC_UNIONTYPE_H

#include "jnc_DerivableType.h"

/**

\defgroup union-type Union Type
	\ingroup type-subsystem
	\import{jnc_UnionType.h}

	\brief Union type represents a region of memory which can be interpreted as one type or another depending on some runtime conditions.

	Unions provide a convenient and at the same time efficient of re-using the same block of memory for different purposes.

\addtogroup union-type
@{

\struct jnc_UnionType
	\verbatim

	Opaque structure used as a handle to Jancy union type.

	Use functions from the `Union Type` to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_UnionType: jnc_DerivableType
{
};

#endif // _JNC_CORE

//..............................................................................

/// @}
