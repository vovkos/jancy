// This file is part of AXL (R) Library

// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_rt_RuntimeStructs.h"

namespace jnc {
namespace rt {

class GcHeap;

} // namespace rt {

namespace ext {

//.............................................................................

typedef
void
MarkOpaqueGcRootsFunc (
	rt::IfaceHdr* iface,
	rt::GcHeap* gcHeap
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct OpaqueClassTypeInfo
{
	size_t m_size;
	MarkOpaqueGcRootsFunc* m_markOpaqueGcRootsFunc;
	bool m_isNonCreatable;
};

//.............................................................................

} // namespace ext
} // namespace jnc
