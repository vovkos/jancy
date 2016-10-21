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

#define _JNC_BITFIELDTYPE_H

#include "jnc_Type.h"

/// \addtogroup bitfield-type
/// @{

//..............................................................................

JNC_EXTERN_C
jnc_Type*
jnc_BitFieldType_getBaseType (jnc_BitFieldType* type);

JNC_EXTERN_C
size_t
jnc_BitFieldType_getBitOffset (jnc_BitFieldType* type);

JNC_EXTERN_C
size_t
jnc_BitFieldType_getBitCount (jnc_BitFieldType* type);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_BitFieldType: jnc_Type
{
	jnc_Type*
	getBaseType ()
	{
		return jnc_BitFieldType_getBaseType (this);
	}

	size_t
	getBitOffset ()
	{
		return jnc_BitFieldType_getBitOffset (this);
	}

	size_t
	getBitCount ()
	{
		return jnc_BitFieldType_getBitCount (this);
	}
};

#endif // _JNC_CORE

//..............................................................................

/// @}
