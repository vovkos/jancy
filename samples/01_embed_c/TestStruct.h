#pragma once

#include "MyLib.h"

JNC_DECLARE_TYPE (TestStruct)

//.............................................................................

struct TestStruct
{
	int m_x;
	int m_y;
};

typedef struct TestStruct TestStruct;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
TestStruct_construct_0 (jnc_DataPtr selfPtr);

void
TestStruct_construct_1 (
	jnc_DataPtr selfPtr, 
	int x,
	int y
	);

void
TestStruct_foo_0 (jnc_DataPtr selfPtr);

void
TestStruct_foo_1 (
	jnc_DataPtr selfPtr, 
	int x,
	int y
	);

void
TestStruct_foo_2 (
	jnc_DataPtr selfPtr, 
	jnc_DataPtr srcPtr
	);

//.............................................................................
