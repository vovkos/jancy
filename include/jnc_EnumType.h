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

#define _JNC_ENUMTYPE_H

#include "jnc_Type.h"
#include "jnc_RuntimeStructs.h"

/**

\defgroup enum-type Enum Type
	\ingroup type-subsystem

	\brief Enum type defines a mnemonic identification for a set of integer values.

\addtogroup enum-type
@{

\struct jnc_EnumType
	\verbatim

	Opaque structure used as a handle to Jancy enum type.

	Use functions from the `Enum Type` group to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

enum jnc_EnumTypeFlag {
	jnc_EnumTypeFlag_Exposed = 0x010000,
	jnc_EnumTypeFlag_BitFlag = 0x020000,
	jnc_EnumTypeFlag__All    = 0x030000,
};

typedef enum jnc_EnumTypeFlag jnc_EnumTypeFlag;

//..............................................................................

JNC_EXTERN_C
int64_t
jnc_EnumConst_getValue(jnc_EnumConst* enumConst);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_EnumConst: jnc_ModuleItem {
	int64_t
	getValue() {
		return jnc_EnumConst_getValue(this);
	}
};

#endif // _JNC_CORE

//..............................................................................

JNC_EXTERN_C
jnc_Type*
jnc_EnumType_getBaseType(jnc_EnumType* type);

JNC_EXTERN_C
jnc_Type*
jnc_EnumType_getRootType(jnc_EnumType* type);

JNC_EXTERN_C
size_t
jnc_EnumType_getConstCount(jnc_EnumType* type);

JNC_EXTERN_C
jnc_EnumConst*
jnc_EnumType_getConst(
	jnc_EnumType* type,
	size_t index
);

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_EnumType: jnc_NamedType {
	jnc_Type*
	getBaseType() {
		return jnc_EnumType_getBaseType(this);
	}

	jnc_Type*
	getRootType() {
		return jnc_EnumType_getRootType(this);
	}

	size_t
	getConstCount() {
		return jnc_EnumType_getConstCount(this);
	}

	jnc_EnumConst*
	getConst(size_t index) {
		return jnc_EnumType_getConst(this, index);
	}
};

#endif // _JNC_CORE

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_EnumTypeFlag EnumTypeFlag;

const EnumTypeFlag
	EnumTypeFlag_Exposed = jnc_EnumTypeFlag_Exposed,
	EnumTypeFlag_BitFlag = jnc_EnumTypeFlag_BitFlag,
	EnumTypeFlag__All    = jnc_EnumTypeFlag__All;

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
