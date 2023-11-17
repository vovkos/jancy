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

enum SslStdDh {
	SslStdDh_Dh1024x160,
	SslStdDh_Dh2048x224,
	SslStdDh_Dh2048x256,
};

//..............................................................................

class SslState: public SslStateBase {
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
	openSsl(
		Runtime* runtime,
		axl::io::Socket* socket
	);

	static
	SslState*
	createSslState(axl::io::Socket* socket);

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
	getVerifyMode() {
		return m_ssl.getVerifyMode();
	}

	void
	JNC_CDECL
	setVerifyMode(int mode) {
		m_ssl.setVerifyMode(mode);
	}

	size_t
	JNC_CDECL
	getVerifyDepth() {
		return m_ssl.getVerifyDepth();
	}

	void
	JNC_CDECL
	setVerifyDepth(size_t depth) {
		m_ssl.setVerifyDepth((int)depth);
	}

	static
	String
	JNC_CDECL
	getStateString(SslState* self) {
		return allocateString(self->m_ssl.getStateString());
	}

	static
	String
	JNC_CDECL
	getStateStringLong(SslState* self) {
		return allocateString(self->m_ssl.getStateStringLong());
	}

	bool
	JNC_CDECL
	enableCiphers(String ciphers) {
		return
			m_sslCtx.setCipherList(ciphers >> toAxl) &&
			m_ssl.setCipherList(ciphers >> toAxl);
	}

	bool
	JNC_CDECL
	setEphemeralDhParams(String pem);

	bool
	JNC_CDECL
	loadEphemeralDhParams(String fileName);

	bool
	JNC_CDECL
	setEphemeralDhStdParams(uint_t dh);

	bool
	JNC_CDECL
	setEphemeralEcdhCurve(String curveName);

	bool
	JNC_CDECL
	loadVerifyLocations(
		String caFileName,
		String caDir
	) {
		return m_sslCtx.loadVerifyLocations(caFileName >> toAxl, caDir >> toAxl);
	}

	bool
	JNC_CDECL
	loadCertificate(String fileName) {
		return m_ssl.useCertificateFile(fileName >> toAxl, SSL_FILETYPE_PEM);
	}

	bool
	JNC_CDECL
	loadPrivateKey(String fileName) {
		return m_ssl.usePrivateKeyFile(fileName >> toAxl, SSL_FILETYPE_PEM);
	}

	bool
	JNC_CDECL
	shutdown() {
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
