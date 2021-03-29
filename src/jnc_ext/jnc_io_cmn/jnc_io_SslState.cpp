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
#include "jnc_io_SslState.h"

namespace jnc {
namespace io {

//..............................................................................

size_t
JNC_CDECL
SslState::getPeerCertificateChainLength()
{
	STACK_OF(X509)* chain = ::SSL_get_peer_cert_chain(m_ssl);
	return chain ? sk_X509_num(chain) : 0;
}

IfaceHdr*
JNC_CDECL
SslState::getPeerCertificateChainEntry(size_t i)
{
	STACK_OF(X509)* chain = ::SSL_get_peer_cert_chain(m_ssl);
	X509* cert = sk_X509_value(chain, i);
	return cert ? SslCertificate::create(cert) : NULL;
}

IfaceHdr*
JNC_CDECL
SslState::getPeerCertificate()
{
	X509* cert = ::SSL_get_peer_certificate(m_ssl);
	return cert ? SslCertificate::create(cert) : NULL;
}

size_t
JNC_CDECL
SslState::getAvailableCipherCount()
{
	STACK_OF(SSL_CIPHER)* stack = ::SSL_get_ciphers(m_ssl);
	return stack ? sk_SSL_CIPHER_num(stack) : 0;
}

IfaceHdr*
JNC_CDECL
SslState::getAvailableCipherSetEntry(size_t i)
{
	STACK_OF(SSL_CIPHER)* stack = ::SSL_get_ciphers(m_ssl);
	const SSL_CIPHER* cipher = sk_SSL_CIPHER_value(stack, i);
	return cipher ? SslCipher::create(cipher) : NULL;
}

IfaceHdr*
JNC_CDECL
SslState::getCurrentCipher()
{
	const SSL_CIPHER* cipher = ::SSL_get_current_cipher(m_ssl);
	return cipher ? SslCipher::create(cipher) : NULL;
}

bool
JNC_CDECL
SslState::setEphemeralDhParams(
	DataPtr pemPtr,
	size_t length
	)
{
	if (length == -1)
		length = strLen(pemPtr);

	cry::Dh dh;
	return
		dh.readParameters(pemPtr.m_p, length) &&
		m_ssl.setTmpDh(dh);
}

bool
JNC_CDECL
SslState::loadEphemeralDhParams(DataPtr fileNamePtr)
{
	axl::io::SimpleMappedFile file;
	cry::Bio bio;
	cry::Dh dh;

	return
		file.open((char*)fileNamePtr.m_p, axl::io::FileFlag_ReadOnly | axl::io::FileFlag_OpenExisting) &&
		bio.createMemBuf(file.p(), file.getMappingSize()) &&
		dh.readParameters(bio) &&
		m_ssl.setTmpDh(dh);
}

bool
JNC_CDECL
SslState::setEphemeralDhStdParams(uint_t stdDh)
{
	cry::Dh dh;

	bool result;

	switch (stdDh)
	{
	case SslStdDh_Dh1024x160:
		result = dh.create1024x160();
		break;

	case SslStdDh_Dh2048x224:
		result = dh.create2048x224();
		break;

	case SslStdDh_Dh2048x256:
		result = dh.create2048x256();
		break;

	default:
		err::setError(err::SystemErrorCode_InvalidParameter);
		return false;
	}

	return
		result &&
		m_ssl.setTmpDh(dh);
}

bool
JNC_CDECL
SslState::setEphemeralEcdhCurve(DataPtr curveNamePtr)
{
	int curveId = OBJ_sn2nid((char*)curveNamePtr.m_p);
	if (curveId == NID_undef)
	{
		err::setFormatStringError("invalid curve '%s'", curveNamePtr.m_p);
		return false;
	}

	cry::EcKey ec;

	return
		ec.create(curveId) && // no need to generate key now
		m_ssl.setTmpEcdh(ec);
}

bool
SslState::openSsl()
{
	ASSERT(m_socket.isOpen());

	bool result =
		m_sslCtx.create() &&
		m_sslBio.createSocket(m_socket.m_socket) &&
		m_ssl.create(m_sslCtx);

	if (!result)
		return false;

	m_ssl.setBio(m_sslBio.detach());
	m_ssl.setExtraData(g_SslStateSelfIdx, this);
	m_ssl.setInfoCallback(sslInfoCallback);
	return m_ioThread.start();
}

void
SslState::closeSsl()
{
	m_ssl.close();
	m_sslBio.close();
	m_sslCtx.close();
}

void
SslState::sslInfoCallback(
	const SSL* ssl,
	int where,
	int ret
	)
{
	SslState* self = (SslState*)::SSL_get_ex_data(ssl, g_SslStateSelfIdx);
	if (!self)
		return;

	ASSERT(self->m_ssl == ssl);
	callMulticast(self->m_runtime, self->m_onStateChanged, where, ret);
}

//..............................................................................

} // namespace io
} // namespace jnc
