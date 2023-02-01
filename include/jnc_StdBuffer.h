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

#define _JNC_STDBUFFER_H

#include "jnc_RuntimeStructs.h"

typedef struct jnc_StdBuffer jnc_StdBuffer;

//..............................................................................

JNC_EXTERN_C
jnc_StdBuffer*
jnc_createStdBuffer(jnc_Runtime* runtime);

JNC_INLINE
size_t
jnc_StdBuffer_getSize(jnc_StdBuffer* buffer);

JNC_INLINE
bool_t
jnc_StdBuffer_isEmpty(jnc_StdBuffer* buffer);

JNC_INLINE
bool_t
jnc_StdBuffer_setSize(
	jnc_StdBuffer* buffer,
	size_t size
);

JNC_INLINE
void
jnc_StdBuffer_clear(jnc_StdBuffer* buffer);

JNC_EXTERN_C
bool_t
jnc_StdBuffer_reserve(
	jnc_StdBuffer* buffer,
	size_t size
);

JNC_EXTERN_C
size_t
jnc_StdBuffer_copy(
	jnc_StdBuffer* buffer,
	const void* p,
	size_t size
);

JNC_INLINE
size_t
jnc_StdBuffer_append(
	jnc_StdBuffer* buffer,
	const void* p,
	size_t size
);

JNC_EXTERN_C
size_t
jnc_StdBuffer_insert(
	jnc_StdBuffer* buffer,
	size_t offset,
	const void* p,
	size_t size
);

JNC_EXTERN_C
size_t
jnc_StdBuffer_remove(
	jnc_StdBuffer* buffer,
	size_t offset,
	size_t size
);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_StdBuffer {
	jnc_IfaceHdr m_ifaceHdr;
	jnc_DataPtr m_ptr;
	size_t m_size;
	size_t m_maxSize;

#ifdef __cplusplus
	size_t
	getSize() {
		return jnc_StdBuffer_getSize(this);
	}

	bool
	isEmpty() {
		return jnc_StdBuffer_isEmpty(this) != 0;
	}

	void
	clear() {
		jnc_StdBuffer_clear(this);
	}

	bool
	setSize(size_t size) {
		return jnc_StdBuffer_setSize(this, size) != 0;
	}

	bool
	reserve(size_t size) {
		return jnc_StdBuffer_reserve(this, size) != 0;
	}

	size_t
	copy(
		const void* p,
		size_t size
	) {
		return jnc_StdBuffer_copy(this, p, size);
	}

	size_t
	append(
		const void* p,
		size_t size
	) {
		return jnc_StdBuffer_append(this, p, size);
	}

	size_t
	insert(
		size_t offset,
		const void* p,
		size_t size
	) {
		return jnc_StdBuffer_insert(this, offset, p, size);
	}

	size_t
	remove(
		jnc_StdBuffer* buffer,
		size_t offset,
		size_t size
	) {
		return jnc_StdBuffer_remove(this, offset, size);
	}
#endif
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_INLINE
size_t
jnc_StdBuffer_getSize(jnc_StdBuffer* buffer) {
	return buffer->m_size;
}

JNC_INLINE
bool_t
jnc_StdBuffer_isEmpty(jnc_StdBuffer* buffer) {
	return buffer->m_size == 0;
}

JNC_INLINE
bool_t
jnc_StdBuffer_setSize(
	jnc_StdBuffer* buffer,
	size_t size
) {
	bool_t result = jnc_StdBuffer_reserve(buffer, size);
	if (result)
		buffer->m_size = size;
	return result;
}

JNC_INLINE
void
jnc_StdBuffer_clear(jnc_StdBuffer* buffer) {
	buffer->m_size = 0;
}

JNC_INLINE
size_t
jnc_StdBuffer_append(
	jnc_StdBuffer* buffer,
	const void* p,
	size_t size
) {
	return jnc_StdBuffer_insert(buffer, -1, p, size);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

typedef jnc_StdBuffer      StdBuffer;

//..............................................................................

} // namespace jnc

#endif // __cplusplus
