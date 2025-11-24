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

#define _JNC_TEMPLATE_H

#include "jnc_Type.h"

/// \addtogroup namespace
/// @{

//..............................................................................

JNC_EXTERN_C
jnc_TypeKind
jnc_Template_getDerivableTypeKind(jnc_Template* templ);

JNC_EXTERN_C
jnc_Type*
jnc_Template_getDeclType(jnc_Template* templ);

JNC_EXTERN_C
size_t
jnc_Template_getArgCount(jnc_Template* templ);

JNC_EXTERN_C
jnc_Type*
jnc_Template_getArg(
	jnc_Template* templ,
	size_t index
);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Template: jnc_ModuleItem {
	jnc_TypeKind
	getDerivableTypeKind() {
		return jnc_Template_getDerivableTypeKind(this);
	}

	jnc_Type*
	getDeclType() {
		return jnc_Template_getDeclType(this);
	}

	size_t
	getArgCount() {
		return jnc_Template_getArgCount(this);
	}

	jnc_Type*
	getArg(size_t index) {
		return jnc_Template_getArg(this, index);
	}
};

#endif // _JNC_CORE

//..............................................................................

/// @}
