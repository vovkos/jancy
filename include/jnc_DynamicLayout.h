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

#define _JNC_DYNAMICLAYOUT_H

#include "jnc_RuntimeStructs.h"
#include "jnc_ModuleItem.h"

typedef struct jnc_DynamicLayout     jnc_DynamicLayout;
typedef struct jnc_DynamicLayoutSink jnc_DynamicLayoutSink;

//..............................................................................

#ifdef __cplusplus
struct jnc_DynamicLayoutSink: jnc_IfaceHdr {
};
#endif

//..............................................................................

enum jnc_DynamicLayoutMode {
	jnc_DynamicLayoutMode_Block = 0,
	jnc_DynamicLayoutMode_Stream,
};

typedef enum jnc_DynamicLayoutMode jnc_DynamicLayoutMode;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum jnc_DynamicLayoutAwaitKind {
	jnc_DynamicLayoutAwaitKind_None = 0,
	jnc_DynamicLayoutAwaitKind_Size, // awaiting (size - bufferSize) more bytes
	jnc_DynamicLayoutAwaitKind_Char, // awaiting awaitChar (scanning since size)
};

typedef enum jnc_DynamicLayoutAwaitKind jnc_DynamicLayoutAwaitKind;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
jnc_DynamicLayout*
jnc_createDynamicLayout(
	jnc_Runtime* runtime,
	uint_t mode,
	jnc_DynamicLayoutSink* sink,
	jnc_DataPtr ptr,
	size_t size
);

JNC_EXTERN_C
size_t
jnc_DynamicLayout_getSize(jnc_DynamicLayout* layout);

JNC_EXTERN_C
size_t
jnc_DynamicLayout_getBufferSize(jnc_DynamicLayout* layout);

JNC_EXTERN_C
size_t
jnc_DynamicLayout_getSizeLimit(jnc_DynamicLayout* layout);

JNC_EXTERN_C
void
jnc_DynamicLayout_setSizeLimit(
	jnc_DynamicLayout* layout,
	size_t size
);

JNC_EXTERN_C
uint_t
jnc_DynamicLayout_getMode(jnc_DynamicLayout* layout);

JNC_EXTERN_C
jnc_DynamicLayoutSink*
jnc_DynamicLayout_getSink(jnc_DynamicLayout* layout);

JNC_EXTERN_C
jnc_DynamicLayoutAwaitKind
jnc_DynamicLayout_getAwaitKind(jnc_DynamicLayout* layout);

JNC_EXTERN_C
char
jnc_DynamicLayout_getAwaitChar(jnc_DynamicLayout* layout);

JNC_EXTERN_C
void
jnc_DynamicLayout_reset(
	jnc_DynamicLayout* layout,
	uint_t mode,
	jnc_DynamicLayoutSink* sink,
	jnc_DataPtr ptr,
	size_t size
);

JNC_INLINE
void
jnc_DynamicLayout_clear(jnc_DynamicLayout* layout) {
	jnc_DynamicLayout_reset(layout, jnc_DynamicLayoutMode_Block, NULL, jnc_g_nullDataPtr, 0);
}

JNC_EXTERN_C
void
jnc_DynamicLayout_closeAllGroups(jnc_DynamicLayout* layout);

JNC_EXTERN_C
size_t
jnc_DynamicLayout_resume(
	jnc_DynamicLayout* layout,
	jnc_DataPtr ptr,
	size_t size
);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#ifdef __cplusplus
struct jnc_DynamicLayout: jnc_IfaceHdr {
	size_t
	getSize() {
		return jnc_DynamicLayout_getSize(this);
	}

	size_t
	getBufferSize() {
		return jnc_DynamicLayout_getBufferSize(this);
	}

	size_t
	getSizeLimit() {
		return jnc_DynamicLayout_getSizeLimit(this);
	}

	void
	setSizeLimit(size_t limit) {
		return jnc_DynamicLayout_setSizeLimit(this, limit);
	}

	uint_t
	getMode() {
		return jnc_DynamicLayout_getMode(this);
	}

	jnc_DynamicLayoutSink*
	getSink() {
		return jnc_DynamicLayout_getSink(this);
	}

	jnc_DynamicLayoutAwaitKind
	getAwaitKind() {
		return jnc_DynamicLayout_getAwaitKind(this);
	}

	char
	getAwaitChar() {
		return jnc_DynamicLayout_getAwaitChar(this);
	}

	void
	clear() {
		jnc_DynamicLayout_clear(this);
	}

	void
	reset(
		uint_t mode,
		jnc_DynamicLayoutSink* sink,
		jnc_DataPtr ptr,
		size_t size
	) {
		jnc_DynamicLayout_reset(this, mode, sink, ptr, size);
	}

	void
	closeAllGroups() {
		jnc_DynamicLayout_closeAllGroups(this);
	}

	size_t
	resume(
		jnc_DataPtr ptr,
		size_t size
	) {
		return jnc_DynamicLayout_resume(this, ptr, size);
	}
};
#endif

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

typedef jnc_DynamicLayout     DynamicLayout;
typedef jnc_DynamicLayoutSink DynamicLayoutSink;
typedef jnc_DynamicLayoutMode DynamicLayoutMode;
typedef jnc_DynamicLayoutAwaitKind DynamicLayoutAwaitKind;

const DynamicLayoutMode
	DynamicLayoutMode_Block  = jnc_DynamicLayoutMode_Block,
	DynamicLayoutMode_Stream = jnc_DynamicLayoutMode_Stream;

const DynamicLayoutAwaitKind
	DynamicLayoutAwaitKind_None = jnc_DynamicLayoutAwaitKind_None,
	DynamicLayoutAwaitKind_Size = jnc_DynamicLayoutAwaitKind_Size,
	DynamicLayoutAwaitKind_Char = jnc_DynamicLayoutAwaitKind_Char;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
DynamicLayout*
createDynamicLayout(Runtime* runtime) {
	return jnc_createDynamicLayout(runtime, DynamicLayoutMode_Block, NULL, g_nullDataPtr, 0);
}

inline
DynamicLayout*
createDynamicLayout(
	Runtime* runtime,
	DynamicLayoutSink* sink,
	DataPtr ptr,
	size_t size
) {
	return jnc_createDynamicLayout(runtime, DynamicLayoutMode_Block, sink, ptr, size);
}

inline
DynamicLayout*
createDynamicLayout(
	Runtime* runtime,
	uint_t mode,
	DynamicLayoutSink* sink,
	DataPtr ptr,
	size_t size
) {
	return jnc_createDynamicLayout(runtime, mode, sink, ptr, size);
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus
