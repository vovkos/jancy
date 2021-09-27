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
bool_t
jnc_isEveryCapabilityEnabled();

JNC_EXTERN_C
bool_t
jnc_isCapabilityEnabled(const char* capability);

JNC_EXTERN_C
size_t
jnc_readCapabilityParam(
	const char* param,
	void* value,
	size_t size
);

JNC_INLINE
size_t
jnc_getCapabilityParamSize(const char* param) {
	return jnc_readCapabilityParam(param, NULL, 0);
}

JNC_EXTERN_C
size_t
jnc_writeCapabilityParam(
	const char* param,
	const void* value,
	size_t size
);

JNC_EXTERN_C
bool_t
jnc_failWithCapabilityError(const char* capability);

JNC_INLINE
bool_t
jnc_requireCapability(const char* capability) {
	return jnc_isCapabilityEnabled(capability) ? true : jnc_failWithCapabilityError(capability);
}

JNC_EXTERN_C
void
jnc_enableCapability(
	const char* capability,
	bool_t isEnabled
);

JNC_EXTERN_C
void
jnc_initializeCapabilities(const char* initializer);

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

inline
bool
isEveryCapabilityEnabled() {
	return jnc_isEveryCapabilityEnabled() != 0;
}

inline
bool
isCapabilityEnabled(const char* capability) {
	return jnc_isCapabilityEnabled(capability) != 0;
}

inline
size_t
getCapabilityParamSize(const char* param) {
	return jnc_getCapabilityParamSize(param);
}

inline
size_t
readCapabilityParam(
	const char* param,
	void* value,
	size_t size
) {
	return jnc_readCapabilityParam(param, value, size);
}

inline
size_t
writeCapabilityParam(
	const char* param,
	const void* value,
	size_t size
) {
	return jnc_writeCapabilityParam(param, value, size);
}

inline
bool
requireCapability(const char* capability) {
	return jnc_requireCapability(capability) != 0;
}

inline
bool
failWithCapabilityError(const char* capability) {
	return jnc_failWithCapabilityError(capability) != 0;
}

inline
void
enableCapability(
	const char* capability,
	bool isEnabled = true
) {
	jnc_enableCapability(capability, isEnabled);
}

inline
void
initializeCapabilities(const char* initializer) {
	jnc_initializeCapabilities(initializer);
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
