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

namespace jnc {
namespace std {

class StringBuilder;

JNC_DECLARE_CLASS_TYPE(StringBuilder)

//..............................................................................

class StringBuilder: public IfaceHdr
{
public:
	DataPtr m_ptr;
	size_t m_length;
	size_t m_bufferSize;

public:
	void
	JNC_CDECL
	clear()
	{
		m_length = 0;
	}

	bool
	JNC_CDECL
	reserve(size_t size);

	size_t
	JNC_CDECL
	copy_char(
		utf32_t c,
		size_t count
		);

	size_t
	JNC_CDECL
	copy_utf8(
		DataPtr ptr,
		size_t length
		);

	size_t
	JNC_CDECL
	copy_utf16(
		DataPtr ptr,
		size_t length
		);

	size_t
	JNC_CDECL
	copy_utf32(
		DataPtr ptr,
		size_t length
		);

	size_t
	JNC_CDECL
	insert_char(
		size_t offset,
		utf32_t c,
		size_t count
		);

	size_t
	JNC_CDECL
	insert_utf8(
		size_t offset,
		DataPtr ptr,
		size_t length
		);

	size_t
	JNC_CDECL
	insert_utf16(
		size_t offset,
		DataPtr ptr,
		size_t length
		);

	size_t
	JNC_CDECL
	insert_utf32(
		size_t offset,
		DataPtr ptr,
		size_t length
		);

	size_t
	JNC_CDECL
	remove(
		size_t offset,
		size_t length
		);

	static
	DataPtr
	detachString(StringBuilder* self);

	static
	DataPtr
	cloneString(StringBuilder* self);

protected:
	size_t
	copyImpl(
		const char* p,
		size_t length
		);

	size_t
	insertImpl(
		size_t offset,
		const char* p,
		size_t length
		);
};

//..............................................................................

} // namespace std
} // namespace jnc
