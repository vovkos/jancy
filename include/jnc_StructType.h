#pragma once

#define _JNC_STRUCTTYPE_H

#include "jnc_DerivableType.h"

/// \addtogroup struct-type
/// @{

//.............................................................................

JNC_EXTERN_C
size_t
jnc_StructField_getOffset (jnc_StructField* field);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

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

//.............................................................................

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_StructType: jnc_DerivableType
{
};

#endif // _JNC_CORE

//.............................................................................

/// @}
