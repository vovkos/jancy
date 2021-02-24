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
#include "jnc_ClassType.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_ct_CapabilityMgr.h"
#endif

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

// extensions should only have read-only access to capabilities

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_isEveryCapabilityEnabled()
{
	return jnc_g_dynamicExtensionLibHost->m_capabilityFuncTable->m_isEveryCapabilityEnabledFunc();
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_isCapabilityEnabled(const char* capability)
{
	return jnc_g_dynamicExtensionLibHost->m_capabilityFuncTable->m_isCapabilityEnabledFunc(capability);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_requireCapability(const char* capability)
{
	return jnc_g_dynamicExtensionLibHost->m_capabilityFuncTable->m_requireCapabilityFunc(capability);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_initializeCapabilities(const char* initializer)
{
	jnc::ct::getCapabilityMgr()->initializeCapabilities(initializer);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_enableCapability(
	const char* capability,
	bool_t isEnabled
	)
{
	jnc::ct::getCapabilityMgr()->enableCapability(capability, isEnabled);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_isEveryCapabilityEnabled()
{
	return jnc::ct::getCapabilityMgr()->isEverythingEnabled();
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_isCapabilityEnabled(const char* capability)
{
	return jnc::ct::getCapabilityMgr()->isCapabilityEnabled(capability);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_requireCapability(const char* capability)
{
	if (jnc::ct::getCapabilityMgr()->isCapabilityEnabled(capability))
		return true;

	err::setFormatStringError("capability %s is required but not enabled", capability);
	return false;
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
