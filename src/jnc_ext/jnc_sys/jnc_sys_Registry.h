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

#include "jnc_ExtensionLib.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace sys {

JNC_DECLARE_OPAQUE_CLASS_TYPE(RegKey)

//..............................................................................

class RegKey: public IfaceHdr {
protected:
	HKEY m_key;

public:
	~RegKey() {
		close();
	}

	void
	JNC_CDECL
	close();

	bool
	JNC_CDECL
	create0(
		uint_t parent,
		String path
	) {
		return createImpl((HKEY)(uintptr_t)parent, path >> toAxl);
	}

	bool
	JNC_CDECL
	create1(
		RegKey* parent,
		String path
	) {
		return parent && parent->m_key ?
			createImpl(parent->m_key, path >> toAxl) :
			err::fail(err::SystemErrorCode_InvalidParameter);
	}

	bool
	JNC_CDECL
	open0(
		uint_t parent,
		String path,
		uint_t access
	) {
		return openImpl((HKEY)(uintptr_t)parent, path >> toAxl, (REGSAM)access);
	}

	bool
	JNC_CDECL
	open1(
		RegKey* parent,
		String path,
		uint_t access
	) {
		return parent && parent->m_key ?
			openImpl(parent->m_key, path >> toAxl, access) :
			err::fail(err::SystemErrorCode_InvalidParameter);
	}

	static
	DataPtr
	JNC_CDECL
	read(
		RegKey* self,
		String name,
		DataPtr typePtr
	);

	uint32_t
	JNC_CDECL
	readDword(String name);

	static
	String
	JNC_CDECL
	readString(
		RegKey* self,
		String name
	);

	bool
	JNC_CDECL
	write(
		String name,
		uint_t type,
		DataPtr dataPtr,
		size_t size
	) {
		return writeImpl(name >> toAxl, type, dataPtr.m_p, size);
	}

	bool
	JNC_CDECL
	writeString(
		String name,
		String value
	);

protected:
	bool
	createImpl(
		HKEY parent,
		const sl::StringRef& path
	);

	bool
	openImpl(
		HKEY parent,
		const sl::StringRef& path,
		REGSAM access
	);

	size_t
	readImpl(
		sl::Array<byte_t>* buffer,
		const sl::StringRef& name,
		dword_t* type
	);

	bool
	writeImpl(
		const sl::StringRef& name,
		dword_t type,
		const void* p,
		size_t size
	);
};

//..............................................................................

} // namespace sys
} // namespace jnc
