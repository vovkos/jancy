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

#define _JNC_ATTRIBUTEBLOCK_H

#include "jnc_ModuleItem.h"
#include "jnc_Variant.h"

/**

\defgroup attribute-block Attribute Block
	\ingroup module-subsystem

\addtogroup attribute-block
@{

*/

//..............................................................................

JNC_EXTERN_C
jnc_Variant
jnc_Attribute_getValue(jnc_Attribute* attr);

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Attribute: jnc_ModuleItem {
	jnc_Variant
	getValue() {
		return jnc_Attribute_getValue(this);
	}
};

#endif // _JNC_CORE

//..............................................................................

JNC_EXTERN_C
size_t
jnc_AttributeBlock_getAttributeCount(jnc_AttributeBlock* block);

JNC_EXTERN_C
jnc_Attribute*
jnc_AttributeBlock_getAttribute(
	jnc_AttributeBlock* block,
	size_t index
);

JNC_EXTERN_C
jnc_Attribute*
jnc_AttributeBlock_findAttribute(
	jnc_AttributeBlock* block,
	const char* name
);

JNC_EXTERN_C
bool_t
jnc_AttributeBlock_ensureAttributeValuesReady(jnc_AttributeBlock* block);

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_AttributeBlock: jnc_ModuleItem {
	size_t
	getAttributeCount() {
		return jnc_AttributeBlock_getAttributeCount(this);
	}

	jnc_Attribute*
	getAttribute(size_t index) {
		return jnc_AttributeBlock_getAttribute(this, index);
	}

	jnc_Attribute*
	findAttribute(const char* name) {
		return jnc_AttributeBlock_findAttribute(this, name);
	}

	bool
	ensureAttributeValuesReady() {
		return jnc_AttributeBlock_ensureAttributeValuesReady(this) != 0;
	}

};

#endif // _JNC_CORE

//..............................................................................

/// @}
