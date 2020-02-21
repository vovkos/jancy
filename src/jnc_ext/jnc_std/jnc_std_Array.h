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
#include "jnc_Variant.h"

namespace jnc {
namespace std {

class Array;

JNC_DECLARE_CLASS_TYPE(Array)

//..............................................................................

class Array: public IfaceHdr
{
public:
	DataPtr m_ptr;
	size_t m_count;
	size_t m_maxCount;

public:
	void
	JNC_CDECL
	clear();

	bool
	JNC_CDECL
	setCount(size_t count);

	bool
	JNC_CDECL
	reserve(size_t count);

	size_t
	JNC_CDECL
	copy(
		DataPtr ptr,
		size_t count
		);

	size_t
	JNC_CDECL
	insert(
		size_t index,
		DataPtr ptr,
		size_t count
		);

	size_t
	JNC_CDECL
	remove(
		size_t index,
		size_t count
		);
};

//..............................................................................

} // namespace std
} // namespace jnc
