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
#include "jnc_StdBuffer.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_ct_Module.h"
#	include "jnc_rt_Runtime.h"
#	include "jnc_std_Buffer.h"
#endif

#include "jnc_Runtime.h"

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_StdBuffer*
jnc_createStdBuffer(jnc_Runtime* runtime) {
	return jnc_g_dynamicExtensionLibHost->m_stdBufferFuncTable->m_createStdBufferFunc(runtime);
}

bool_t
jnc_StdBuffer_reserve(
	jnc_StdBuffer* buffer,
	size_t size
) {
	return jnc_g_dynamicExtensionLibHost->m_stdBufferFuncTable->m_reserveFunc(buffer, size);
}

size_t
jnc_StdBuffer_copy(
	jnc_StdBuffer* buffer,
	const void* p,
	size_t size
) {
	return jnc_g_dynamicExtensionLibHost->m_stdBufferFuncTable->m_copyFunc(buffer, p, size);
}

size_t
jnc_StdBuffer_insert(
	jnc_StdBuffer* buffer,
	size_t offset,
	const void* p,
	size_t size
) {
	return jnc_g_dynamicExtensionLibHost->m_stdBufferFuncTable->m_insertFunc(buffer, offset, p, size);
}

size_t
jnc_StdBuffer_remove(
	jnc_StdBuffer* buffer,
	size_t offset,
	size_t size
) {
	return jnc_g_dynamicExtensionLibHost->m_stdBufferFuncTable->m_removeFunc(buffer, offset, size);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
jnc_StdBuffer*
jnc_createStdBuffer(jnc_Runtime* runtime) {
	return (jnc_StdBuffer*)jnc::createClass<jnc::std::Buffer>(runtime);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_StdBuffer_reserve(
	jnc_StdBuffer* buffer,
	size_t size
) {
	return ((jnc::std::Buffer*)buffer)->reserve(size);
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_StdBuffer_copy(
	jnc_StdBuffer* buffer,
	const void* p,
	size_t size
) {
	return ((jnc::std::Buffer*)buffer)->copy_u(p, size);
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_StdBuffer_insert(
	jnc_StdBuffer* buffer,
	size_t offset,
	const void* p,
	size_t size
) {
	return ((jnc::std::Buffer*)buffer)->insert_u(offset, p, size);
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_StdBuffer_remove(
	jnc_StdBuffer* buffer,
	size_t offset,
	size_t size
) {
	return ((jnc::std::Buffer*)buffer)->remove(offset, size);
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
