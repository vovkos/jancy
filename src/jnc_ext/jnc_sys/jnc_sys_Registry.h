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

class RegKey: public IfaceHdr
{
protected:
	HKEY m_key;

public:
	~RegKey()
	{
		close();
	}

	void
	JNC_CDECL
	close();

	bool
	JNC_CDECL
	create0(
		uint_t parent,
		DataPtr pathPtr
		)
	{
		return createImpl((HKEY)(uintptr_t)parent, (char*)pathPtr.m_p);
	}

	bool
	JNC_CDECL
	create1(
		RegKey* parent,
		DataPtr pathPtr
		)
	{
		return parent && parent->m_key ?
			createImpl(parent->m_key, (char*)pathPtr.m_p) :
			err::fail(err::SystemErrorCode_InvalidParameter);
	}

	bool
	JNC_CDECL
	open0(
		uint_t parent,
		DataPtr pathPtr,
		uint_t access
		)
	{
		return openImpl((HKEY)(uintptr_t)parent, (char*)pathPtr.m_p, (REGSAM)access);
	}

	bool
	JNC_CDECL
	open1(
		RegKey* parent,
		DataPtr pathPtr,
		uint_t access
		)
	{
		return parent && parent->m_key ?
			openImpl(parent->m_key, (char*)pathPtr.m_p, access) :
			err::fail(err::SystemErrorCode_InvalidParameter);
	}

	static
	DataPtr
	JNC_CDECL
	read(
		RegKey* self,
		DataPtr namePtr,
		DataPtr typePtr
		);

	uint32_t
	JNC_CDECL
	readDword(DataPtr namePtr);

	static
	DataPtr
	JNC_CDECL
	readString(
		RegKey* self,
		DataPtr namePtr
		);

	bool
	JNC_CDECL
	write(
		DataPtr namePtr,
		uint_t type,
		DataPtr dataPtr,
		size_t size
		)
	{
		return writeImpl((char*)namePtr.m_p, type, dataPtr.m_p, size);
	}

	bool
	JNC_CDECL
	writeString(
		DataPtr namePtr,
		DataPtr valuePtr
		);

protected:
	bool
	createImpl(
		HKEY parent,
		const char* path
		);

	bool
	openImpl(
		HKEY parent,
		const char* path,
		REGSAM access
		);

	size_t
	readImpl(
		sl::Array<byte_t>* buffer,
		const char* name,
		dword_t* type
		);

	bool
	writeImpl(
		const char* name,
		dword_t type,
		const void* p,
		size_t size
		);
};

//..............................................................................

} // namespace sys
} // namespace jnc
