// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Type.h"

namespace jnc {

class GcHeap;

//.............................................................................

enum BoxFlag
{
	BoxFlag_Dead         = 0x01,
	BoxFlag_Static       = 0x02,

	BoxFlag_GcMark       = 0x10,
	BoxFlag_GcWeakMark   = 0x20,
	BoxFlag_GcWeakMark_c = 0x40,
	BoxFlag_GcRootsAdded = 0x80,
	BoxFlag_GcMask       = 0xf0,
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
	size_t m_elementCount;
	
	void 
	gcMarkData (GcHeap* gcHeap);

	void 
	gcMarkObject (GcHeap* gcHeap);

	void 
	gcMarkClassMemberFields (GcHeap* gcHeap);

	void 
	gcWeakMarkObject ();

	void 
	gcWeakMarkClosureObject (GcHeap* gcHeap);
};

//.............................................................................

} // namespace jnc {
