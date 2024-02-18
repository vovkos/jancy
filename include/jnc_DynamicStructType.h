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

#define _JNC_DYNAMICSTRUCTTYPE_H

#include "jnc_Type.h"

/**

\defgroup dynamic-struct-type Dynamic Struct Type
	\ingroup type-subsystem
	\import{jnc_DynamicStructType.h}

	Dynamic struct type fields' presence and size is dynamically determined at runtime.

\addtogroup dynamic-struct-type
@{

\struct jnc_DynamicStructType
	\verbatim

	Opaque structure used as a handle to Jancy struct type.

	Use functions from the `Dynamic Struct Type` group to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_DynamicStructType: jnc_NamedType {
};

#endif // _JNC_CORE

//..............................................................................

/// @}
