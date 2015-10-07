#include "pch.h"
#include "TestStruct.h"

//.............................................................................

void
TestStruct::construct_0 (jnc::rt::DataPtr selfPtr)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;

	printf ("  TestStruct::construct_0 ()\n");
	self->m_x = 1;
	self->m_y = 2;
}

void
TestStruct::construct_1 (
	jnc::rt::DataPtr selfPtr, 
	int x,
	int y
	)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;

	printf ("  TestStruct::construct_1 (%d, %d)\n", x, y);
	self->m_x = x;
	self->m_y = y;
}

void
TestStruct::foo_0 (jnc::rt::DataPtr selfPtr)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;

	printf ("  TestStruct::foo_0 ()\n");
	int t = self->m_x;
	self->m_x = self->m_y;
	self->m_y = t;
}

void
TestStruct::foo_1 (
	jnc::rt::DataPtr selfPtr, 
	int x,
	int y
	)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;

	printf ("  TestStruct::foo_1 (%d, %d)\n", x, y);
	self->m_x = x;
	self->m_y = y;
}

void
TestStruct::foo_2 (
	jnc::rt::DataPtr selfPtr, 
	jnc::rt::DataPtr srcPtr
	)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;
	TestStruct* src = (TestStruct*) srcPtr.m_p;

	printf ("  TestStruct::foo_2 ( { %d, %d } )\n", src->m_x, src->m_y);
	self->m_x = src->m_x;
	self->m_y = src->m_y;
}

//.............................................................................
