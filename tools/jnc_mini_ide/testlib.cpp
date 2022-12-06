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
#include "testlib.h"
#include "mainwindow.h"

//..............................................................................

JNC_DEFINE_TYPE(
	Point,
	"Point",
	g_testLibGuid,
	TestLibCacheSlot_Point
)

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE(
	TestClassA,
	"TestClassA",
	g_testLibGuid,
	TestLibCacheSlot_TestClassA
)

JNC_BEGIN_TYPE_FUNCTION_MAP(TestClassA)
	JNC_MAP_FUNCTION("foo", &TestClassA::foo)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	TestClassB,
	"TestClassB",
	g_testLibGuid,
	TestLibCacheSlot_TestClassB,
	TestClassB,
	&TestClassB::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(TestClassB)
	JNC_MAP_FUNCTION("bar", &TestClassB::bar)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE(TestStruct, "TestStruct", g_testLibGuid, TestLibCacheSlot_TestStruct)

JNC_BEGIN_TYPE_FUNCTION_MAP(TestStruct)
	JNC_MAP_CONSTRUCTOR(&TestStruct::construct_0)
	JNC_MAP_OVERLOAD(&TestStruct::construct_1)
	JNC_MAP_OVERLOAD(&TestStruct::construct_2)

	JNC_MAP_FUNCTION("foo", &TestStruct::foo_0)
	JNC_MAP_OVERLOAD(&TestStruct::foo_1)
	JNC_MAP_OVERLOAD(&TestStruct::foo_2)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
JNC_CDECL
TestClassA::foo(int x) {
	printf("TestClassA::foo (%d)\n", x);
	m_x = x;
}

//..............................................................................

void
JNC_CDECL
TestClassB::markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
//	if (self->m_hiddenIface)
//		self->m_hiddenIface->m_box->gcMarkObject (gcHeap);
}

bool
JNC_CDECL
TestClassB::bar(
	jnc::DataPtr ptr1,
	jnc::DataPtr ptr2,
	jnc::DataPtr ptr3,
	jnc::DataPtr ptr4,
	int a,
	int b
) {
	const char* p1 = (const char*) ptr1.m_p;
	const char* p2 = (const char*) ptr2.m_p;
	const char* p3 = (const char*) ptr3.m_p;
	const char* p4 = (const char*) ptr4.m_p;

	printf("TestClassB::bar ()\n");

	return true;
}

//..............................................................................

void
JNC_CDECL
TestStruct::construct_0(jnc::DataPtr selfPtr) {
	TestStruct* self = (TestStruct*)selfPtr.m_p;
	printf("TestStruct::construct () { m_x = %d, m_y = %f }\n", self->m_x, self->m_y);
}

void
JNC_CDECL
TestStruct::construct_1(jnc::DataPtr selfPtr, int x) {
	TestStruct* self = (TestStruct*)selfPtr.m_p;
	printf("TestStruct::construct (int x = %d) { m_x = %d, m_y = %f }\n", x, self->m_x, self->m_y);
	self->m_x = x;
}

void
JNC_CDECL
TestStruct::construct_2(jnc::DataPtr selfPtr, double y) {
	TestStruct* self = (TestStruct*)selfPtr.m_p;
	printf("TestStruct::construct (double y = %f) { m_x = %d, m_y = %f }\n", y, self->m_x, self->m_y);
	self->m_y = y;
}

void
JNC_CDECL
TestStruct::foo_0(jnc::DataPtr selfPtr) {
	TestStruct* self = (TestStruct*)selfPtr.m_p;
	printf("TestStruct::foo () { m_x = %d, m_y = %f }\n", self->m_x, self->m_y);
}

void
JNC_CDECL
TestStruct::foo_1(jnc::DataPtr selfPtr, int x) {
	TestStruct* self = (TestStruct*)selfPtr.m_p;
	printf("TestStruct::foo (int x = %d) { m_x = %d, m_y = %f }\n", x, self->m_x, self->m_y);
}

void
JNC_CDECL
TestStruct::foo_2(jnc::DataPtr selfPtr, double y) {
	TestStruct* self = (TestStruct*)selfPtr.m_p;
	printf("TestStruct::foo (double y = %f) { m_x = %d, m_y = %f }\n", y, self->m_x, self->m_y);
}

//..............................................................................

void
testPtr(
	jnc::DataPtr ptr,
	jnc::DataPtr ptr2
) {
	printf("TestLib::testPtr\n");
}

void
testVariant(jnc::Variant variant) {
	printf("TestLib::testVariant\n");
}

void
testAlloc() {
	jnc::Runtime* runtime = jnc::getCurrentThreadRuntime();
	ASSERT(runtime);

	jnc::GcHeap* gcHeap = runtime->getGcHeap();

	jnc::DataPtr ptr1;
	jnc::DataPtr ptr2;
	jnc::DataPtr ptr3;

	JNC_BEGIN_CALL_SITE(runtime)
		ptr1 = gcHeap->allocateBuffer(100);
		memset(ptr1.m_p, 0xaa, 100);

		ptr2 = gcHeap->allocateBuffer(100);
		memset(ptr2.m_p, 0xbb, 100);

		ptr3 = gcHeap->allocateBuffer(100);
		memset(ptr3.m_p, 0xcc, 100);
	JNC_END_CALL_SITE()

	gcHeap->collect();

	printf("done\n");
}

void
testCallSite() {
	jnc::Runtime* runtime = jnc::getCurrentThreadRuntime();
	ASSERT(runtime);

	printf("before JNC_BEGIN_CALL_SITE\n");

	JNC_BEGIN_CALL_SITE(runtime)
		printf("inside JNC_CALL_SITE\n");
	JNC_END_CALL_SITE()

	printf("after JNC_END_CALL_SITE\n");
}

//..............................................................................

JNC_DEFINE_LIB(
	TestLib,
	g_testLibGuid,
	"TestLib",
	"Jancy QT-based test utility extension library"
)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE(TestLib)
JNC_END_LIB_SOURCE_FILE_TABLE()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE(TestLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(TestClassB)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE()

JNC_BEGIN_LIB_FUNCTION_MAP(TestLib)
//	JNC_MAP_TYPE(TestClassA)
//	JNC_MAP_TYPE(TestClassB)
//	JNC_MAP_TYPE(TestStruct)
//	JNC_MAP_FUNCTION("testPtr",     &testPtr)
//	JNC_MAP_FUNCTION("testVariant", &testVariant)
//	JNC_MAP_FUNCTION("testAlloc", testAlloc)
//	JNC_MAP_FUNCTION("testCallSite", testCallSite)
JNC_END_LIB_FUNCTION_MAP()

//..............................................................................
