#pragma once

#define _JNC_ARRAYTYPE_H

#include "jnc_Type.h"

//.............................................................................

JNC_EXTERN_C
jnc_Type*
jnc_ArrayType_getElementType (jnc_ArrayType* type);

JNC_EXTERN_C
size_t
jnc_ArrayType_getElementCount (jnc_ArrayType* type);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_ArrayType: jnc_Type
{
	jnc_Type*
	getElementType ()
	{
		return jnc_ArrayType_getElementType (this);
	}

	size_t
	getElementCount ()
	{
		return jnc_ArrayType_getElementCount (this);
	}
};

#endif // _JNC_CORE

//.............................................................................
