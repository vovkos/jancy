//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#pragma once

#include "MyLib.h"

JNC_DECLARE_TYPE (TestStruct)

//..............................................................................

struct TestStruct
{
	int m_x;
	int m_y;
};

typedef struct TestStruct TestStruct;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

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

//..............................................................................
