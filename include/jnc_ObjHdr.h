// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Type.h"

namespace jnc {

class Runtime;

//.............................................................................

enum ObjHdrFlagKind
{
	ObjHdrFlagKind_Dead         = 0x0001,
	ObjHdrFlagKind_DynamicArray = 0x0002,	
	ObjHdrFlagKind_Static       = 0x0010,
	ObjHdrFlagKind_Stack        = 0x0020,
	ObjHdrFlagKind_UHeap        = 0x0040,
	ObjHdrFlagKind_GcMark       = 0x0100,
	ObjHdrFlagKind_GcWeakMark   = 0x0200,
	ObjHdrFlagKind_GcWeakMark_c = 0x0400,
	ObjHdrFlagKind_GcRootsAdded = 0x0800,
	ObjHdrFlagKind_GcMask       = 0x0f00,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct ObjHdr
{
	size_t m_scopeLevel; // if scope level != 0 and the object is not of class-type, then the rest can be omitted
	ObjHdr* m_root;

	union
	{
		Type* m_type;
		ClassType* m_classType;
	};

	uintptr_t m_flags;
	
	void 
	gcMarkData (Runtime* runtime);

	void 
	gcMarkObject (Runtime* runtime);

	void 
	gcWeakMarkObject ()
	{
		m_flags |= ObjHdrFlagKind_GcWeakMark;
	}

	void 
	gcWeakMarkClosureObject (Runtime* runtime);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
ObjHdr*
getStaticObjHdr ()
{
	static ObjHdr objHdr = 
	{ 
		0, 
		&objHdr, 
		NULL, 
		ObjHdrFlagKind_Static | 
		ObjHdrFlagKind_GcMark | 
		ObjHdrFlagKind_GcWeakMark | 
		ObjHdrFlagKind_GcRootsAdded
	};

	return &objHdr;
}

//.............................................................................

} // namespace jnc {
