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

	Use functions from the `Struct Type` to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

enum jnc_StructTypeFlag
{
	jnc_StructTypeFlag_Dynamic = 0x010000,
};

typedef enum jnc_StructTypeFlag jnc_StructTypeFlag;

//..............................................................................

JNC_EXTERN_C
size_t
jnc_StructField_getOffset (jnc_StructField* field);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_StructField: jnc_ModuleItem
{
	size_t
	getOffset ()
	{
		return jnc_StructField_getOffset (this);
	}
};

#endif // _JNC_CORE

//..............................................................................

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_StructType: jnc_DerivableType
{
};

#endif // _JNC_CORE

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_StructTypeFlag StructTypeFlag;

const StructTypeFlag
	StructTypeFlag_Dynamic = jnc_StructTypeFlag_Dynamic;

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
