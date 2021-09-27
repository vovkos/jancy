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

#define _JNC_PROMISE_H

#include "jnc_Variant.h"

/**

\defgroup promise Promise
	\ingroup runtime-structs
	\import{jnc_Promise.h}

\addtogroup promise
@{
*/

typedef struct jnc_Promise jnc_Promise;

//..............................................................................

JNC_EXTERN_C
jnc_Promise*
jnc_createPromise(jnc_Runtime* runtime);

JNC_EXTERN_C
void
jnc_Promise_complete(
	jnc_Promise* promise,
	jnc_Variant result,
	jnc_DataPtr errorPtr
);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_Promise {
	jnc_IfaceHdr m_ifaceHdr;
	size_t m_state;
	jnc_IfaceHdr* m_scheduler;
	jnc_Promise* m_pendingPromise;
	jnc_GcShadowStackFrame* m_gcShadowStackFrame;
	jnc_Variant m_result;
	jnc_DataPtr m_errorPtr;

	// implementation is opaque (see jnc::rtl::PromiseImpl)

#ifdef __cplusplus
	void
	complete(
		jnc_Variant result,
		jnc_DataPtr errorPtr
	) {
		jnc_Promise_complete(this, result, errorPtr);
	}
#endif
};

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_Promise Promise;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Promise*
createPromise(jnc_Runtime* runtime) {
	return jnc_createPromise(runtime);
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
