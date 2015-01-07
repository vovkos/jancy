#pragma once

#include "ApiSlots.h"

//.............................................................................

struct TestStruct
{
public:
	JNC_BEGIN_TYPE ("TestStruct", ApiSlot_TestStruct)
		JNC_CONSTRUCTOR (&construct_0)
		JNC_OVERLOAD (&construct_1)
		JNC_FUNCTION ("foo", &foo_0)
		JNC_OVERLOAD (&foo_1)
		JNC_OVERLOAD (&foo_2)
	JNC_END_TYPE ()

public:
	int m_x;
	int m_y;

public:
	static
	void
	construct_0 (jnc::DataPtr selfPtr);

	static
	void
	construct_1 (
		jnc::DataPtr selfPtr, 
		int x,
		int y
		);

	static
	void
	foo_0 (jnc::DataPtr selfPtr);

	static
	void
	foo_1 (
		jnc::DataPtr selfPtr, 
		int x,
		int y
		);

	static
	void
	foo_2 (
		jnc::DataPtr selfPtr, 
		jnc::DataPtr srcPtr
		);
};

//.............................................................................
