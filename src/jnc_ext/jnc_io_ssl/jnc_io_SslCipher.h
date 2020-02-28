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

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE(SslCipher)

//..............................................................................

class SslCipher: public IfaceHdr
{
	friend class SslSocket;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(SslCipher)

protected:
	const SSL_CIPHER* m_cipher;
	DataPtr m_namePtr;
	DataPtr m_descriptionPtr;

public:
	static
	SslCipher*
	create(const SSL_CIPHER* cert);

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	static
	DataPtr
	JNC_CDECL
	getName(SslCipher* self);

	static
	DataPtr
	JNC_CDECL
	getDescription(SslCipher* self);

	uint_t
	JNC_CDECL
	getBitCount();
};

//..............................................................................

} // namespace io
} // namespace jnc
