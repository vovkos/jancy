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

JNC_DECLARE_TYPE(SslCertNameEntry)
JNC_DECLARE_OPAQUE_CLASS_TYPE(SslCertName)
JNC_DECLARE_OPAQUE_CLASS_TYPE(SslCertificate)

//..............................................................................

enum SslCertFormat
{
	SslCertFormat_Pem,
	SslCertFormat_Der,
};

//..............................................................................

struct SslCertNameEntry
{
	JNC_DECLARE_TYPE_STATIC_METHODS(SslCertNameEntry)

	uint_t m_nid;
	DataPtr m_namePtr;
	DataPtr m_valuePtr;
};

//..............................................................................

class SslCertName: public IfaceHdr
{
	friend class SslCertificate;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(SslCertName)

public:
	size_t m_entryCount;

protected:
	X509_NAME* m_name;

	sl::Array<DataPtr> m_entryArray;
	DataPtr m_oneLinePtr;

public:
	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	static
	DataPtr
	JNC_CDECL
	getEntryTable(
		SslCertName* self,
		size_t i
		);

	static
	DataPtr
	JNC_CDECL
	getOneLine(SslCertName* self);

	static
	DataPtr
	JNC_CDECL
	findEntry(
		SslCertName* self,
		int nid
		);

protected:
	static
	DataPtr
	createEntry(X509_NAME_ENTRY* entry);
};

//..............................................................................

class SslCertificate: public IfaceHdr
{
	friend class SslSocket;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(SslCertificate)

protected:
	X509* m_cert;
	cry::AutoX509 m_autoCert;

	DataPtr m_serialNumberPtr;
	uint64_t m_validFromDate;
	uint64_t m_validToDate;
	SslCertName* m_subject;
	SslCertName* m_issuer;

public:
	static
	SslCertificate*
	create(X509* cert);

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	static
	DataPtr
	JNC_CDECL
	getSerialNumber(SslCertificate* self);

	uint64_t
	JNC_CDECL
	getValidFromDate();

	uint64_t
	JNC_CDECL
	getValidToDate();

	SslCertName*
	JNC_CDECL
	getSubject();

	SslCertName*
	JNC_CDECL
	getIssuer();

	bool
	JNC_CDECL
	encode(
		std::Buffer* buffer,
		uint_t format
		);

	bool
	JNC_CDECL
	decode(
		DataPtr ptr,
		size_t size,
		uint_t format
		)
	{
		return decodeImpl(ptr.m_p, size, format);
	}

	bool
	JNC_CDECL
	load(
		DataPtr fileNamePtr,
		uint_t format
		);

	bool
	JNC_CDECL
	save(
		DataPtr fileNamePtr,
		uint_t format
		);

protected:
	SslCertName*
	createSslCertName(X509_NAME* name);

	uint64_t
	getTimestamp(const ASN1_TIME* time);

	bool
	encodeImpl(
		sl::Array<char>* buffer,
		uint_t format
		);

	bool
	decodeImpl(
		const void* p,
		size_t size,
		uint_t format
		);
};

//..............................................................................

DataPtr
JNC_CDECL
getSslNidShortName(uint_t nid);

DataPtr
JNC_CDECL
getSslNidLongName(uint_t nid);

//..............................................................................

} // namespace io
} // namespace jnc
