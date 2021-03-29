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

#include "jnc_io_SslStateBase.h"
#include "jnc_io_SslCipher.h"
#include "jnc_io_SslCertificate.h"

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE(SslState)

//..............................................................................

enum SslStdDh
{
	SslStdDh_Dh1024x160,
	SslStdDh_Dh2048x224,
	SslStdDh_Dh2048x256,
};

//..............................................................................

class SslState: public SslStateBase
{
public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(SslState)

protected:
	static int m_selfIdx;
	static int m_runtimeIdx;

public:
	static
	void
	initAppData();

	bool
	JNC_CDECL
	openSsl(axl::io::Socket* socket);

	size_t
	JNC_CDECL
	getAvailableCipherCount();

	SslCipher*
	JNC_CDECL
	getAvailableCipherSetEntry(size_t i);

	SslCipher*
	JNC_CDECL
	getCurrentCipher();

	size_t
	JNC_CDECL
	getPeerCertificateChainLength();

	SslCertificate*
	JNC_CDECL
	getPeerCertificateChainEntry(size_t i);

	SslCertificate*
	JNC_CDECL
	getPeerCertificate();

	int
	JNC_CDECL
	getVerifyMode()
	{
		return m_ssl.getVerifyMode();
	}

	void
	JNC_CDECL
	setVerifyMode(int mode)
	{
		m_ssl.setVerifyMode(mode);
	}

	size_t
	JNC_CDECL
	getVerifyDepth()
	{
		return m_ssl.getVerifyDepth();
	}

	void
	JNC_CDECL
	setVerifyDepth(size_t depth)
	{
		m_ssl.setVerifyDepth((int)depth);
	}

	static
	DataPtr
	JNC_CDECL
	getStateString(SslState* self)
	{
		return strDup(self->m_ssl.getStateString());
	}

	static
	DataPtr
	JNC_CDECL
	getStateStringLong(SslState* self)
	{
		return strDup(self->m_ssl.getStateStringLong());
	}

	bool
	JNC_CDECL
	enableCiphers(DataPtr ciphersPtr)
	{
		return
			m_sslCtx.setCipherList((char*)ciphersPtr.m_p) &&
			m_ssl.setCipherList((char*)ciphersPtr.m_p);
	}

	bool
	JNC_CDECL
	setEphemeralDhParams(
		DataPtr pemPtr,
		size_t length
		);

	bool
	JNC_CDECL
	loadEphemeralDhParams(DataPtr fileNamePtr);

	bool
	JNC_CDECL
	setEphemeralDhStdParams(uint_t dh);

	bool
	JNC_CDECL
	setEphemeralEcdhCurve(DataPtr curveNamePtr);

	bool
	JNC_CDECL
	loadVerifyLocations(
		DataPtr caFileNamePtr,
		DataPtr caDirPtr
		)
	{
		return m_sslCtx.loadVerifyLocations((char*)caFileNamePtr.m_p, (char*)caDirPtr.m_p);
	}

	bool
	JNC_CDECL
	loadCertificate(DataPtr fileNamePtr)
	{
		return m_ssl.useCertificateFile((char*)fileNamePtr.m_p, SSL_FILETYPE_PEM);
	}

	bool
	JNC_CDECL
	loadPrivateKey(DataPtr fileNamePtr)
	{
		return m_ssl.usePrivateKeyFile((char*)fileNamePtr.m_p, SSL_FILETYPE_PEM);
	}

	bool
	JNC_CDECL
	shutdown()
	{
		return m_ssl.shutdown();
	}

protected:
	static
	void
	sslInfoCallback(
		const SSL* ssl,
		int where,
		int ret
		);
};

//..............................................................................

} // namespace io
} // namespace jnc
