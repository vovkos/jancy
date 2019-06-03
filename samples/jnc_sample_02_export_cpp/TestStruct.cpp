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

#include "pch.h"
#include "TestStruct.h"

//..............................................................................

JNC_DEFINE_TYPE(
	TestStruct,
	"TestStruct",
	g_myLibGuid,
	MyLibCacheSlot_TestStruct
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(TestStruct)
	JNC_MAP_CONSTRUCTOR(&TestStruct::construct_0)
	JNC_MAP_OVERLOAD(&TestStruct::construct_1)
	JNC_MAP_FUNCTION("foo", &TestStruct::foo_0)
	JNC_MAP_OVERLOAD(&TestStruct::foo_1)
	JNC_MAP_OVERLOAD(&TestStruct::foo_2)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
TestStruct::construct_0(jnc::DataPtr selfPtr)
{
	TestStruct* self = (TestStruct*)selfPtr.m_p;

	printf("  TestStruct::construct_0 ()\n");
	self->m_x = 1;
	self->m_y = 2;
}

void
TestStruct::construct_1(
	jnc::DataPtr selfPtr,
	int x,
	int y
	)
{
	TestStruct* self = (TestStruct*)selfPtr.m_p;

	printf("  TestStruct::construct_1 (%d, %d)\n", x, y);
	self->m_x = x;
	self->m_y = y;
}

void
TestStruct::foo_0(jnc::DataPtr selfPtr)
{
	TestStruct* self = (TestStruct*)selfPtr.m_p;

	printf("  TestStruct::foo_0 ()\n");
	int t = self->m_x;
	self->m_x = self->m_y;
	self->m_y = t;
}

void
TestStruct::foo_1(
	jnc::DataPtr selfPtr,
	int x,
	int y
	)
{
	TestStruct* self = (TestStruct*)selfPtr.m_p;

	printf("  TestStruct::foo_1 (%d, %d)\n", x, y);
	self->m_x = x;
	self->m_y = y;
}

void
TestStruct::foo_2(
	jnc::DataPtr selfPtr,
	jnc::DataPtr srcPtr
	)
{
	TestStruct* self = (TestStruct*)selfPtr.m_p;
	TestStruct* src = (TestStruct*)srcPtr.m_p;

	printf("  TestStruct::foo_2 ( { %d, %d } )\n", src->m_x, src->m_y);
	self->m_x = src->m_x;
	self->m_y = src->m_y;
}

//..............................................................................
