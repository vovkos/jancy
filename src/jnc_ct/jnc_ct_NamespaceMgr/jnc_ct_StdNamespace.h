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
namespace ct {

//..............................................................................

enum StdNamespace
{
	StdNamespace_Global,
	StdNamespace_Jnc,
	StdNamespace_Internal,
	StdNamespace__Count,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct StdItemSource
{
	const char* m_source;
	size_t m_length;
	StdNamespace m_stdNamespace;
};

//..............................................................................

} // namespace ct
} // namespace jnc
