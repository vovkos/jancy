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

#define _JNC_CAPABILITY_H

//..............................................................................

JNC_EXTERN_C
void
jnc_initializeCapabilities(const char* initializer);

JNC_EXTERN_C
void
jnc_enableCapability(
	const char* capability,
	bool_t isEnabled
	);

JNC_EXTERN_C
bool_t
jnc_isEveryCapabilityEnabled();

JNC_EXTERN_C
bool_t
jnc_isCapabilityEnabled(const char* capability);

JNC_EXTERN_C
bool_t
jnc_requireCapability(const char* capability);

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

inline
void
initializeCapabilities(const char* initializer)
{
	jnc_initializeCapabilities(initializer);
}


inline
void
enableCapability(
	const char* capability,
	bool isEnabled = true
	)
{
	jnc_enableCapability(capability, isEnabled);
}

inline
bool
isEveryCapabilityEnabled()
{
	return jnc_isEveryCapabilityEnabled() != 0;
}

inline
bool
isCapabilityEnabled(const char* capability)
{
	return jnc_isCapabilityEnabled(capability) != 0;
}

inline
bool_t
requireCapability(const char* capability)
{
	return jnc_requireCapability(capability) != 0;
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
