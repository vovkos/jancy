// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Type.h"

namespace jnc {

class Runtime;

//.............................................................................

enum BoxFlag
{
	BoxFlag_Dead         = 0x0001,
	BoxFlag_DynamicArray = 0x0002,
	BoxFlag_Variable     = 0x0004,
	BoxFlag_Static       = 0x0010,
	BoxFlag_Stack        = 0x0020,

	BoxFlag_GcMark       = 0x0100,
	BoxFlag_GcWeakMark   = 0x0200,
	BoxFlag_GcWeakMark_c = 0x0400,
	BoxFlag_GcRootsAdded = 0x0800,
	BoxFlag_GcMask       = 0x0f00,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct Box
{
	Box* m_root;

	union
	{
		Type* m_type;
		ClassType* m_classType;
	};

	uintptr_t m_flags;
	void* m_dataPtrValidatorCache;
	
	void 
	gcMarkData (Runtime* runtime);

	void 
	gcMarkObject (Runtime* runtime);

	void 
	gcMarkClassMemberFields (Runtime* runtime);

	void 
	gcWeakMarkObject ();

	void 
	gcWeakMarkClosureObject (Runtime* runtime);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct VariableBox: Box
{
	void* m_p;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Box*
getStaticBox ()
{
	static Box objHdr = 
	{ 
		&objHdr, 
		NULL, 
		BoxFlag_Static | 
		BoxFlag_GcMark | 
		BoxFlag_GcWeakMark | 
		BoxFlag_GcRootsAdded
	};

	return &objHdr;
}

//.............................................................................

} // namespace jnc {
