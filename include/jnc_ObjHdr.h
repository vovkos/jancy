// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Type.h"

namespace jnc {

class Runtime;

//.............................................................................

enum ObjHdrFlag
{
	ObjHdrFlag_Dead         = 0x0001,
	ObjHdrFlag_DynamicArray = 0x0002,	
	ObjHdrFlag_Static       = 0x0010,
	ObjHdrFlag_Stack        = 0x0020,
	ObjHdrFlag_GcMark       = 0x0100,
	ObjHdrFlag_GcWeakMark   = 0x0200,
	ObjHdrFlag_GcWeakMark_c = 0x0400,
	ObjHdrFlag_GcRootsAdded = 0x0800,
	ObjHdrFlag_GcMask       = 0x0f00,
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
		m_flags |= ObjHdrFlag_GcWeakMark;
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
		ObjHdrFlag_Static | 
		ObjHdrFlag_GcMark | 
		ObjHdrFlag_GcWeakMark | 
		ObjHdrFlag_GcRootsAdded
	};

	return &objHdr;
}

//.............................................................................

} // namespace jnc {
