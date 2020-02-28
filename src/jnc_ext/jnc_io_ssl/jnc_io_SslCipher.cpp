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
#include "jnc_io_SslCipher.h"
#include "jnc_io_SslLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	SslCipher,
	"io.SslCipher",
	g_sslLibGuid,
	SslLibCacheSlot_SslCipher,
	SslCipher,
	&SslCipher::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(SslCipher)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<SslCipher>)

	JNC_MAP_CONST_PROPERTY("m_name", &SslCipher::getName)
	JNC_MAP_CONST_PROPERTY("m_description", &SslCipher::getDescription)
	JNC_MAP_CONST_PROPERTY("m_bitCount", &SslCipher::getBitCount)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

SslCipher*
SslCipher::create(const SSL_CIPHER* cipher)
{
	Runtime* runtime = getCurrentThreadRuntime();
	SslCipher* self = createClass<SslCipher>(runtime);
	self->m_cipher = cipher;
	return self;
}

void
JNC_CDECL
SslCipher::markOpaqueGcRoots(jnc::GcHeap* gcHeap)
{
	gcHeap->markDataPtr(m_namePtr);
	gcHeap->markDataPtr(m_descriptionPtr);
}

DataPtr
JNC_CDECL
SslCipher::getName(SslCipher* self)
{
	if (!self->m_namePtr.m_p)
		self->m_namePtr = strDup(::SSL_CIPHER_get_name(self->m_cipher));

	return self->m_namePtr;
}

DataPtr
JNC_CDECL
SslCipher::getDescription(SslCipher* self)
{
	if (self->m_descriptionPtr.m_p)
		return self->m_descriptionPtr;

	char buffer[256];
	::SSL_CIPHER_description(self->m_cipher, buffer, sizeof(buffer));
	self->m_descriptionPtr = strDup(buffer);
	return self->m_descriptionPtr;
}

uint_t
JNC_CDECL
SslCipher::getBitCount()
{
	int bitCount = 0;
	::SSL_CIPHER_get_bits(m_cipher, &bitCount);
	return bitCount;
}

//..............................................................................

} // namespace io
} // namespace jnc
