#include "pch.h"
#include "testlib.h"
#include "mainwindow.h"

//.............................................................................

void
AXL_CDECL
TestClassA::foo (int x)
{
	printf ("TestClassA::foo (%d)\n", x);
	m_x = x;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
AXL_CDECL
TestClassB::markOpaqueGcRoots (jnc::rt::GcHeap* gcHeap)
{
//	if (self->m_hiddenIface)
//		self->m_hiddenIface->m_box->gcMarkObject (gcHeap);
}

bool
AXL_CDECL
TestClassB::bar (
	jnc::rt::DataPtr ptr1,
	jnc::rt::DataPtr ptr2,
	jnc::rt::DataPtr ptr3,
	jnc::rt::DataPtr ptr4,
	int a,
	int b
	)
{
	const char* p1 = (const char*) ptr1.m_p;
	const char* p2 = (const char*) ptr2.m_p;
	const char* p3 = (const char*) ptr3.m_p;
	const char* p4 = (const char*) ptr4.m_p;

	printf ("TestClassB::bar ()\n");

	return true;
}

//.............................................................................

void
AXL_CDECL
TestStruct::construct_0 (jnc::rt::DataPtr selfPtr)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;
	printf ("TestStruct::construct () { m_x = %d, m_y = %f }\n", self->m_x, self->m_y);
}

void
AXL_CDECL
TestStruct::construct_1 (jnc::rt::DataPtr selfPtr, int x)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;
	printf ("TestStruct::construct (int x = %d) { m_x = %d, m_y = %f }\n", x, self->m_x, self->m_y);
	self->m_x = x;
}

void
AXL_CDECL
TestStruct::construct_2 (jnc::rt::DataPtr selfPtr, double y)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;
	printf ("TestStruct::construct (double y = %f) { m_x = %d, m_y = %f }\n", y, self->m_x, self->m_y);
	self->m_y = y;
}

void
AXL_CDECL
TestStruct::foo_0 (jnc::rt::DataPtr selfPtr)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;
	printf ("TestStruct::foo () { m_x = %d, m_y = %f }\n", self->m_x, self->m_y);
}

void
AXL_CDECL
TestStruct::foo_1 (jnc::rt::DataPtr selfPtr, int x)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;
	printf ("TestStruct::foo (int x = %d) { m_x = %d, m_y = %f }\n", x, self->m_x, self->m_y);
}

void
AXL_CDECL
TestStruct::foo_2 (jnc::rt::DataPtr selfPtr, double y)
{
	TestStruct* self = (TestStruct*) selfPtr.m_p;
	printf ("TestStruct::foo (double y = %f) { m_x = %d, m_y = %f }\n", y, self->m_x, self->m_y);
}

//.............................................................................

int
TestLib::printf (
	const char* format,
	...
	)
{
	AXL_VA_DECL (va, format);
	return (int) getMainWindow ()->writeOutput_va (format, va.m_va);
}

void
TestLib::testPtr (
	jnc::rt::DataPtr ptr,
	jnc::rt::DataPtr ptr2
	)
{
	printf ("TestLib::testPtr\n");

	((axl::io::SockAddr*) ptr.m_p)->parse ((const char*) ptr2.m_p);
}

void
TestLib::testVariant (jnc::rt::Variant variant)
{
	printf ("TestLib::testVariant\n");
}

void
TestLib::qtWait (uint_t msTime)
{
	uint64_t start = sys::getTimestamp ();
	uint64_t interval = msTime * 10000;
	
	QEventLoop eventLoop;

	for (;;)
	{
		uint_t now = sys::getTimestamp ();
		if (now - start > interval)
			break;

		eventLoop.processEvents (QEventLoop::AllEvents, 100);
	}
}

//.............................................................................
