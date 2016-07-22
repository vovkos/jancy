#pragma once

#include "MyLib.h"

JNC_DECLARE_TYPE (TestStruct)

//.............................................................................

struct TestStruct
{
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
