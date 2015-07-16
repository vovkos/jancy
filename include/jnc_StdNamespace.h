// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

namespace jnc {

//.............................................................................

enum StdNamespace
{
	StdNamespace_Global,
	StdNamespace_Jnc,
	StdNamespace_Internal,
	StdNamespace__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct StdItemSource
{
	const char* m_p;
	size_t m_length;
	StdNamespace m_stdNamespace;
};

//.............................................................................

} // namespace jnc {
