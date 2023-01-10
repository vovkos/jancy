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

class Buffer;

JNC_DECLARE_CLASS_TYPE(Buffer)

//..............................................................................

class Buffer: public IfaceHdr {
public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(Buffer)

public:
	DataPtr m_ptr;
	size_t m_size;
	size_t m_maxSize;

public:
	bool
	JNC_CDECL
	reserve(size_t size);

	bool
	setSize(size_t size);

	size_t
	JNC_CDECL
	copy(
		DataPtr ptr,
		size_t size
	) {
		// TODO: check range
		return copy_u(ptr.m_p, size);
	}

	size_t
	copy_u(
		const void* p,
		size_t size
	);

	size_t
	JNC_CDECL
	insert(
		size_t offset,
		DataPtr ptr,
		size_t size
	) {
		// TODO: check range
		return insert_u(offset, ptr.m_p, size);
	}

	size_t
	insert_u(
		size_t offset,
		const void* p,
		size_t size
	);

	size_t
	JNC_CDECL
	remove(
		size_t offset,
		size_t size
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
bool
Buffer::setSize(size_t size) {
	bool result = reserve(size);
	if (result)
		m_size = size;

	return result;
}

//..............................................................................

} // namespace std
} // namespace jnc
