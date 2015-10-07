#pragma once

#include "MyLibGlobals.h"

//.............................................................................

struct TestStruct
{
public:
	JNC_BEGIN_TYPE_MAP ("TestStruct", g_myLibCacheSlot, MyLibTypeCacheSlot_TestStruct)
		JNC_MAP_CONSTRUCTOR (&construct_0)
		JNC_MAP_OVERLOAD (&construct_1)
		JNC_MAP_FUNCTION ("foo", &foo_0)
		JNC_MAP_OVERLOAD (&foo_1)
		JNC_MAP_OVERLOAD (&foo_2)
	JNC_END_TYPE_MAP ()

public:
	int m_x;
	int m_y;

public:
	static
	void
	construct_0 (jnc::rt::DataPtr selfPtr);

	static
	void
	construct_1 (
		jnc::rt::DataPtr selfPtr, 
		int x,
		int y
		);

	static
	void
	foo_0 (jnc::rt::DataPtr selfPtr);

	static
	void
	foo_1 (
		jnc::rt::DataPtr selfPtr, 
		int x,
		int y
		);

	static
	void
	foo_2 (
		jnc::rt::DataPtr selfPtr, 
		jnc::rt::DataPtr srcPtr
		);
};

//.............................................................................
