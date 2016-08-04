#pragma once

#define _JNC_ATTRIBUTEBLOCK_H

#include "jnc_ModuleItem.h"

//.............................................................................

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Attribute: jnc_ModuleItem
{
};

#endif // _JNC_CORE

//.............................................................................

JNC_EXTERN_C
size_t
jnc_AttributeBlock_getAttributeCount (jnc_AttributeBlock* block);

JNC_EXTERN_C
jnc_Attribute*
jnc_AttributeBlock_getAttribute (
	jnc_AttributeBlock* block,
	size_t index
	);

JNC_EXTERN_C
jnc_Attribute*
jnc_AttributeBlock_findAttribute (
	jnc_AttributeBlock* block,
	const char* name
	);

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_AttributeBlock: jnc_ModuleItem
{
	size_t
	getAttributeCount ()
	{
		return jnc_AttributeBlock_getAttributeCount (this);
	}

	jnc_Attribute*
	getAttribute (size_t index)
	{
		return jnc_AttributeBlock_getAttribute (this, index);
	}

	jnc_Attribute*
	findAttribute (const char* name)
	{
		return jnc_AttributeBlock_findAttribute (this, name);
	}
};

#endif // _JNC_CORE

//.............................................................................
