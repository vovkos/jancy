#ifndef TESTLIB_H
#define TESTLIB_H


//.............................................................................

struct Point
{
	int64_t m_x;
	int64_t m_y;
	int64_t m_z;
	int64_t m_w;
};

//.............................................................................

class TestClassA: public jnc::rt::IfaceHdr
{
public:
	JNC_BEGIN_CLASS_TYPE_MAP ("TestClassA", -1, -1)
		JNC_MAP_FUNCTION ("foo", &TestClassA::foo)
	JNC_END_CLASS_TYPE_MAP ()

public:
	int m_x;

public:
	void
	AXL_CDECL
	destruct ();

	void
	AXL_CDECL
	foo (int x);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class TestClassB: public jnc::rt::IfaceHdr
{
public:
	JNC_OPAQUE_CLASS_TYPE_INFO (TestClassB, &TestClassB::markOpaqueGcRoots)

	JNC_BEGIN_CLASS_TYPE_MAP ("TestClassB", -1, -1)
		JNC_MAP_FUNCTION ("bar", &TestClassB::bar)
	JNC_END_CLASS_TYPE_MAP ()

public:
	char m_data [256];

public:
	void
	AXL_CDECL
	markOpaqueGcRoots (jnc::rt::GcHeap* gcHeap);

	bool
	AXL_CDECL
	bar (
		jnc::rt::DataPtr ptr1,
		jnc::rt::DataPtr ptr2,
		jnc::rt::DataPtr ptr3,
		jnc::rt::DataPtr ptr4,
		int a,
		int b
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class TestStruct
{
public:
	JNC_BEGIN_TYPE_MAP ("TestStruct", -1, -1)
		JNC_MAP_CONSTRUCTOR (&TestStruct::construct_0)
		JNC_MAP_OVERLOAD (&TestStruct::construct_1)
		JNC_MAP_OVERLOAD (&TestStruct::construct_2)

		JNC_MAP_FUNCTION ("foo", &TestStruct::foo_0)
		JNC_MAP_OVERLOAD (&TestStruct::foo_1)
		JNC_MAP_OVERLOAD (&TestStruct::foo_2)
	JNC_END_TYPE_MAP ()

public:
	int m_x;
	double m_y;

public:
	static
	void
	AXL_CDECL
	construct_0 (jnc::rt::DataPtr selfPtr);

	static
	void
	AXL_CDECL
	construct_1 (
		jnc::rt::DataPtr selfPtr, 
		int x
		);

	static
	void
	AXL_CDECL
	construct_2 (
		jnc::rt::DataPtr selfPtr, 
		double y
		);

	static
	void
	AXL_CDECL
	foo_0 (jnc::rt::DataPtr selfPtr);

	static
	void
	AXL_CDECL
	foo_1 (
		jnc::rt::DataPtr selfPtr, 
		int x
		);

	static
	void
	AXL_CDECL
	foo_2 (
		jnc::rt::DataPtr selfPtr, 
		double y
		);
};

//.............................................................................

class TestLib: public jnc::ext::ExtensionLib
{
public:
	JNC_BEGIN_LIB_MAP ()
		JNC_MAP_FUNCTION ("printf", &TestLib::printf)

//		JNC_MAP_TYPE (TestClassA)
//		JNC_MAP_TYPE (TestClassB)
//		JNC_MAP_TYPE (TestStruct)
//		JNC_MAP_FUNCTION ("testPtr",     &testPtr)
//		JNC_MAP_FUNCTION ("testVariant", &testVariant)
//		JNC_MAP_FUNCTION ("qtWait", &qtWait)
	JNC_END_LIB_MAP ()
	
	JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE ()
		JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (TestClassB)
	JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE ()
	
	static
	int
	printf (
		const char* format,
		...
		);

	static
	void
	testPtr (
		jnc::rt::DataPtr ptr,
		jnc::rt::DataPtr ptr2
		);

	static
	void
	testVariant (jnc::rt::Variant variant);

	static
	void
	qtWait (uint_t msTime);
};

#endif