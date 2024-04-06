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

#include "pch.h"
#include "jnc_Unit.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_ct_UnitMgr.h"
#endif

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_ExtensionLib*
jnc_Unit_getLib(jnc_Unit* unit) {
	return jnc_g_dynamicExtensionLibHost->m_unitFuncTable->m_getLibFunc(unit);
}

JNC_EXTERN_C
const char*
jnc_Unit_getFilePath(jnc_Unit* unit) {
	return jnc_g_dynamicExtensionLibHost->m_unitFuncTable->m_getFilePathFunc(unit);
}

JNC_EXTERN_C
const char*
jnc_Unit_getFileName(jnc_Unit* unit) {
	return jnc_g_dynamicExtensionLibHost->m_unitFuncTable->m_getFileNameFunc(unit);
}

JNC_EXTERN_C
const char*
jnc_Unit_getDir(jnc_Unit* unit) {
	return jnc_g_dynamicExtensionLibHost->m_unitFuncTable->m_getDirFunc(unit);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
jnc_ExtensionLib*
jnc_Unit_getLib(jnc_Unit* unit) {
	return unit->getLib();
}

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_Unit_getFilePath(jnc_Unit* unit) {
	return unit->getFilePath();
}


JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_Unit_getFileName(jnc_Unit* unit) {
	return unit->getFileName();
}


JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_Unit_getDir(jnc_Unit* unit) {
	return unit->getDir();
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
