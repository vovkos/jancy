// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Type.h"

namespace jnc {

class CRuntime;

//.............................................................................

enum EObjHdrFlag
{
	EObjHdrFlag_Dead         = 0x0001,
	EObjHdrFlag_DynamicArray = 0x0002,	
	EObjHdrFlag_Static       = 0x0010,
	EObjHdrFlag_Stack        = 0x0020,
	EObjHdrFlag_UHeap        = 0x0040,
	EObjHdrFlag_GcMark       = 0x0100,
	EObjHdrFlag_GcWeakMark   = 0x0200,
	EObjHdrFlag_GcWeakMark_c = 0x0400,
	EObjHdrFlag_GcRootsAdded = 0x0800,
	EObjHdrFlag_GcMask       = 0x0f00,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct TObjHdr
{
	size_t m_ScopeLevel; // if scope level != 0 and the object is not of class-type, then the rest can be omitted
	TObjHdr* m_pRoot;

	union
	{
		CType* m_pType;
		CClassType* m_pClassType;
	};

	uintptr_t m_Flags;
	
	void 
	GcMarkData (CRuntime* pRuntime);

	void 
	GcMarkObject (CRuntime* pRuntime);

	void 
	GcWeakMarkObject ()
	{
		m_Flags |= EObjHdrFlag_GcWeakMark;
	}

	void 
	GcWeakMarkClosureObject (CRuntime* pRuntime);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
TObjHdr*
GetStaticObjHdr ()
{
	static TObjHdr ObjHdr = 
	{ 
		0, 
		&ObjHdr, 
		NULL, 
		EObjHdrFlag_Static | 
		EObjHdrFlag_GcMark | 
		EObjHdrFlag_GcWeakMark | 
		EObjHdrFlag_GcRootsAdded
	};

	return &ObjHdr;
}

//.............................................................................

} // namespace jnc {
