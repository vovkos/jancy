#include "pch.h"
#include "TestStruct.h"

//..............................................................................

JNC_DEFINE_TYPE (
	TestStruct,
	"TestStruct",
	g_myLibGuid,
	MyLibCacheSlot_TestStruct
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (TestStruct)
	JNC_MAP_CONSTRUCTOR (TestStruct_construct_0)
	JNC_MAP_OVERLOAD (TestStruct_construct_1)
	JNC_MAP_FUNCTION ("foo", TestStruct_foo_0)
	JNC_MAP_OVERLOAD (TestStruct_foo_1)
	JNC_MAP_OVERLOAD (TestStruct_foo_2)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

void
TestStruct_construct_0 (jnc_DataPtr selfPtr)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;

	printf ("  TestStruct_construct_0 ()\n");
	self->m_x = 1;
	self->m_y = 2;
}

void
TestStruct_construct_1 (
	jnc_DataPtr selfPtr,
	int x,
	int y
	)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;

	printf ("  TestStruct_construct_1 (%d, %d)\n", x, y);
	self->m_x = x;
	self->m_y = y;
}

void
TestStruct_foo_0 (jnc_DataPtr selfPtr)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;
	int t;

	printf ("  TestStruct_foo_0 ()\n");

	t = self->m_x;
	self->m_x = self->m_y;
	self->m_y = t;
}

void
TestStruct_foo_1 (
	jnc_DataPtr selfPtr,
	int x,
	int y
	)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;

	printf ("  TestStruct_foo_1 (%d, %d)\n", x, y);
	self->m_x = x;
	self->m_y = y;
}

void
TestStruct_foo_2 (
	jnc_DataPtr selfPtr,
	jnc_DataPtr srcPtr
	)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;
	TestStruct* src = (TestStruct*) srcPtr.m_p;

	printf ("  TestStruct_foo_2 ( { %d, %d } )\n", src->m_x, src->m_y);
	self->m_x = src->m_x;
	self->m_y = src->m_y;
}

//..............................................................................
