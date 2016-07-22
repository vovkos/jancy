#pragma once

#define _JNC_STRUCTTYPE_H

#include "jnc_DerivableType.h"

//.............................................................................

JNC_EXTERN_C
jnc_ModuleItemDecl*
jnc_StructField_getItemDecl (jnc_StructField* field);

JNC_EXTERN_C
jnc_Type*
jnc_StructField_getType (jnc_StructField* field);

JNC_EXTERN_C
size_t
jnc_StructField_getOffset (jnc_StructField* field);

//.............................................................................

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_StructField: jnc_ModuleItem
{
	jnc_ModuleItemDecl*
	getItemDecl ()
	{
		return jnc_StructField_getItemDecl (this);
	}

	jnc_Type*
	getType ()
	{
		return jnc_StructField_getType (this);
	}

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
