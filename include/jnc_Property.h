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

#define _JNC_PROPERTY_H

#include "jnc_PropertyType.h"
#include "jnc_Function.h"

/**

\defgroup property Property
	\ingroup module-subsystem
	\import{jnc_Property.h}

\addtogroup property
@{

\struct jnc_Property
	\verbatim

	Opaque structure used as a handle to Jancy property.

	Use functions from the `Property` group to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

enum jnc_PropertyKind {
	jnc_PropertyKind_Undefined = 0,
	jnc_PropertyKind_Normal,
	jnc_PropertyKind_Thunk,
	jnc_PropertyKind_DataThunk,
	jnc_PropertyKind_Internal,
	jnc_PropertyKind__Count
};

typedef enum jnc_PropertyKind jnc_PropertyKind;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum jnc_PropertyFlag {
	jnc_PropertyFlag_Const     = 0x010000,
	jnc_PropertyFlag_Bindable  = 0x020000,
	jnc_PropertyFlag_AutoGet   = 0x100000,
	jnc_PropertyFlag_AutoSet   = 0x200000,
	jnc_PropertyFlag_Finalized = 0x800000,
};

typedef enum jnc_PropertyFlag jnc_PropertyFlag;

//..............................................................................

JNC_INLINE
jnc_PropertyType*
jnc_Property_getType(jnc_Property* prop) {
	return (jnc_PropertyType*)jnc_ModuleItem_getType((jnc_ModuleItem*)prop);
}

JNC_EXTERN_C
jnc_Function*
jnc_Property_getGetter(jnc_Property* prop);

JNC_EXTERN_C
jnc_OverloadableFunction
jnc_Property_getSetter(jnc_Property* prop);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Property: jnc_ModuleItem {
	jnc_PropertyType*
	getType() {
		return jnc_Property_getType(this);
	}

	jnc_Function*
	getGetter() {
		return jnc_Property_getGetter(this);
	}

	jnc::OverloadableFunction
	getSetter() {
		return jnc_Property_getSetter(this);
	}
};

#endif // _JNC_CORE

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_PropertyKind PropertyKind;

const PropertyKind
	PropertyKind_Undefined = jnc_PropertyKind_Undefined,
	PropertyKind_Normal    = jnc_PropertyKind_Normal,
	PropertyKind_Thunk     = jnc_PropertyKind_Thunk,
	PropertyKind_DataThunk = jnc_PropertyKind_DataThunk,
	PropertyKind_Internal  = jnc_PropertyKind_Internal,
	PropertyKind__Count    = jnc_PropertyKind__Count;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef enum jnc_PropertyFlag PropertyFlag;

const PropertyFlag
	PropertyFlag_Const     = jnc_PropertyFlag_Const,
	PropertyFlag_Bindable  = jnc_PropertyFlag_Bindable,
	PropertyFlag_AutoGet   = jnc_PropertyFlag_AutoGet,
	PropertyFlag_AutoSet   = jnc_PropertyFlag_AutoSet,
	PropertyFlag_Finalized = jnc_PropertyFlag_Finalized;

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
