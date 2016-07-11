// This file is part of AXL (R) Library

// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_RuntimeStructs.h"

typedef struct jnc_OpaqueClassTypeInfo jnc_OpaqueClassTypeInfo;

//.............................................................................

typedef
void
jnc_MarkOpaqueGcRootsFunc (
	jnc_IfaceHdr* iface,
	jnc_Runtime* runtime
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_OpaqueClassTypeInfo
{
	size_t m_size;
	jnc_MarkOpaqueGcRootsFunc* m_markOpaqueGcRootsFunc;
	bool m_isNonCreatable;
};

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

typedef jnc_MarkOpaqueGcRootsFunc MarkOpaqueGcRootsFunc;
typedef jnc_OpaqueClassTypeInfo OpaqueClassTypeInfo;

//.............................................................................

} // namespace jnc

#endif // __cplusplus
