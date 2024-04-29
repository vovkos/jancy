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

#define _JNC_UNIT_H

#include "jnc_Def.h"

/**

\defgroup unit Unit
	\ingroup module-subsystem
	\import{jnc_Unit.h}

\addtogroup unit
@{
*/

//..............................................................................

JNC_EXTERN_C
jnc_ExtensionLib*
jnc_Unit_getLib(jnc_Unit* unit);

JNC_EXTERN_C
const char*
jnc_Unit_getFilePath(jnc_Unit* unit);

JNC_EXTERN_C
const char*
jnc_Unit_getFileName(jnc_Unit* unit);

JNC_EXTERN_C
const char*
jnc_Unit_getDir(jnc_Unit* unit);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Unit {
	jnc_ExtensionLib*
	getLib() {
		return jnc_Unit_getLib(this);
	}

	const char*
	getFilePath() {
		return jnc_Unit_getFilePath(this);
	}

	const char*
	getFileName(jnc_Unit* unit) {
		return jnc_Unit_getFileName(this);
	}

	const char*
	getDir(jnc_Unit* unit) {
		return jnc_Unit_getDir(this);
	}
};

#endif // _JNC_CORE

//..............................................................................

/// @}
