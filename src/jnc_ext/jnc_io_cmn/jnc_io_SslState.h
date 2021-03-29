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

//..............................................................................

enum SslStdDh
{
	SslStdDh_Dh1024x160,
	SslStdDh_Dh2048x224,
	SslStdDh_Dh2048x256,
};

//..............................................................................

class SslState: IfaceHdr
{
protected:
	ClassBox<Multicast> m_onStateChanged;

	axl::io::SslCtx m_sslCtx;
	axl::cry::Bio m_sslBio;
	axl::io::Ssl m_ssl;

public:
	size_t
	JNC_CDECL
	getAvailableCipherCount();

	IfaceHdr*
	JNC_CDECL
	getAvailableCipherSetEntry(size_t i);

	IfaceHdr*
	JNC_CDECL
	getCurrentCipher();

	size_t
	JNC_CDECL
	getPeerCertificateChainLength();

	IfaceHdr*
	JNC_CDECL
	getPeerCertificateChainEntry(size_t i);

	IfaceHdr*
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

protected:
	bool
	openSsl();

	void
	closeSsl();

	bool
	sslSuspendLoop();

	bool
	sslHandshakeLoop(bool isClient);

	static
	void
	sslInfoCallback(
		const SSL* ssl,
		int where,
		int ret
		);

#if (_JNC_OS_WIN)
	void
	processFdClose(int error);
#endif
};

//..............................................................................

} // namespace io
} // namespace jnc
