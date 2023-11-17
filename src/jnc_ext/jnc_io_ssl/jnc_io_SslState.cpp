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
#include "jnc_io_SslLib.h"

namespace jnc {
namespace io {

int SslState::m_runtimeIdx = -1;
int SslState::m_selfIdx = -1;

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	SslState,
	"io.SslState",
	g_sslLibGuid,
	SslLibCacheSlot_SslState,
	SslState,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(SslState)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<SslState>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<SslState>)

	JNC_MAP_CONST_PROPERTY("m_stateString", &SslState::getStateString)
	JNC_MAP_CONST_PROPERTY("m_stateStringLong", &SslState::getStateStringLong)
	JNC_MAP_CONST_PROPERTY("m_availableCipherCount", &SslState::getAvailableCipherCount)
	JNC_MAP_CONST_PROPERTY("m_availableCipherSet", &SslState::getAvailableCipherSetEntry)
	JNC_MAP_CONST_PROPERTY("m_currentCipher", &SslState::getCurrentCipher)
	JNC_MAP_CONST_PROPERTY("m_peerCertificateChainLength", &SslState::getPeerCertificateChainLength)
	JNC_MAP_CONST_PROPERTY("m_peerCertificateChain", &SslState::getPeerCertificateChainEntry)
	JNC_MAP_CONST_PROPERTY("m_peerCertificate", &SslState::getPeerCertificate)
	JNC_MAP_PROPERTY("m_verifyMode", &SslState::getVerifyMode, &SslState::setVerifyMode)
	JNC_MAP_PROPERTY("m_verifyDepth", &SslState::getVerifyDepth, &SslState::setVerifyDepth)

	JNC_MAP_FUNCTION("createSslState", &SslState::createSslState)
	JNC_MAP_FUNCTION("enableCiphers", &SslState::enableCiphers)
	JNC_MAP_FUNCTION("setEphemeralDhParams", &SslState::setEphemeralDhParams)
	JNC_MAP_FUNCTION("loadEphemeralDhParams", &SslState::loadEphemeralDhParams)
	JNC_MAP_FUNCTION("setEphemeralDhStdParams", &SslState::setEphemeralDhStdParams)
	JNC_MAP_FUNCTION("setEphemeralEcdhCurve", &SslState::setEphemeralEcdhCurve)
	JNC_MAP_FUNCTION("loadVerifyLocations", &SslState::loadVerifyLocations)
	JNC_MAP_FUNCTION("loadCertificate", &SslState::loadCertificate)
	JNC_MAP_FUNCTION("loadPrivateKey", &SslState::loadPrivateKey)
	JNC_MAP_FUNCTION("shutdown", &SslState::shutdown)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
SslState::initAppData() {
	m_runtimeIdx = ::SSL_get_ex_new_index(0, NULL, NULL, NULL, NULL);
	m_selfIdx = ::SSL_get_ex_new_index(0, NULL, NULL, NULL, NULL);
}

bool
SslState::openSsl(
	Runtime* runtime,
	axl::io::Socket* socket
) {
	ASSERT(m_selfIdx != -1 && m_runtimeIdx != -1);
	ASSERT(socket->isOpen());

	bool result =
		m_sslCtx.create() &&
		m_sslBio.createSocket(socket->m_socket) &&
		m_ssl.create(m_sslCtx);

	if (!result)
		return false;

	m_ssl.setBio(m_sslBio.detach());
	m_ssl.setExtraData(m_selfIdx, this);
	m_ssl.setExtraData(m_runtimeIdx, runtime);
	m_ssl.setInfoCallback(sslInfoCallback);
	return true;
}

SslState*
JNC_CDECL
SslState::createSslState(axl::io::Socket* socket) {
	Runtime* runtime = getCurrentThreadRuntime();
	ASSERT(runtime);

	SslState* sslState = createClass<SslState>(runtime);
	bool result = sslState->openSsl(runtime, socket);
	return result ? sslState : NULL;
}

size_t
JNC_CDECL
SslState::getPeerCertificateChainLength() {
	STACK_OF(X509)* chain = ::SSL_get_peer_cert_chain(m_ssl);
	return chain ? sk_X509_num(chain) : 0;
}

SslCertificate*
JNC_CDECL
SslState::getPeerCertificateChainEntry(size_t i) {
	STACK_OF(X509)* chain = ::SSL_get_peer_cert_chain(m_ssl);
	X509* cert = sk_X509_value(chain, i);
	return cert ? SslCertificate::create(cert) : NULL;
}

SslCertificate*
JNC_CDECL
SslState::getPeerCertificate() {
	X509* cert = ::SSL_get_peer_certificate(m_ssl);
	return cert ? SslCertificate::create(cert) : NULL;
}

size_t
JNC_CDECL
SslState::getAvailableCipherCount() {
	STACK_OF(SSL_CIPHER)* stack = ::SSL_get_ciphers(m_ssl);
	return stack ? sk_SSL_CIPHER_num(stack) : 0;
}

SslCipher*
JNC_CDECL
SslState::getAvailableCipherSetEntry(size_t i) {
	STACK_OF(SSL_CIPHER)* stack = ::SSL_get_ciphers(m_ssl);
	const SSL_CIPHER* cipher = sk_SSL_CIPHER_value(stack, i);
	return cipher ? SslCipher::create(cipher) : NULL;
}

SslCipher*
JNC_CDECL
SslState::getCurrentCipher() {
	const SSL_CIPHER* cipher = ::SSL_get_current_cipher(m_ssl);
	return cipher ? SslCipher::create(cipher) : NULL;
}

bool
JNC_CDECL
SslState::setEphemeralDhParams(String pem) {
	cry::Dh dh;
	return
		dh.readParameters(pem.m_ptr.m_p, pem.m_length) &&
		m_ssl.setTmpDh(dh);
}

bool
JNC_CDECL
SslState::loadEphemeralDhParams(String fileName) {
	axl::io::SimpleMappedFile file;
	cry::Bio bio;
	cry::Dh dh;

	return
		file.open(fileName >> toAxl, axl::io::FileFlag_ReadOnly | axl::io::FileFlag_OpenExisting) &&
		bio.createMemBuf(file.p(), file.getMappingSize()) &&
		dh.readParameters(bio) &&
		m_ssl.setTmpDh(dh);
}

bool
JNC_CDECL
SslState::setEphemeralDhStdParams(uint_t stdDh) {
	cry::Dh dh;

	bool result;

	switch (stdDh) {
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
SslState::setEphemeralEcdhCurve(String curveName0) {
	sl::StringRef curveName = curveName0 >> toAxl;
	int curveId = OBJ_sn2nid(curveName.sz());
	if (curveId == NID_undef) {
		err::setFormatStringError("invalid curve '%s'", curveName.sz());
		return false;
	}

	cry::EcKey ec;

	return
		ec.create(curveId) && // no need to generate key now
		m_ssl.setTmpEcdh(ec);
}

void
SslState::sslInfoCallback(
	const SSL* ssl,
	int where,
	int ret
) {
	ASSERT(m_selfIdx != -1 && m_runtimeIdx != -1);
	SslState* self = (SslState*)::SSL_get_ex_data(ssl, m_selfIdx);
	Runtime* runtime = (Runtime*)::SSL_get_ex_data(ssl, m_runtimeIdx);

	if (!self || !runtime)
		return;

	ASSERT(self->m_ssl == ssl);
	callMulticast(runtime, self->m_onStateChanged, where, ret);
}

//..............................................................................

} // namespace io
} // namespace jnc
