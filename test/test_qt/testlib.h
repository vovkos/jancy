#ifndef TESTLIB_H
#define TESTLIB_H

//..............................................................................

// {384498AC-90AF-4634-B083-2A9B02D62680}

JNC_DEFINE_GUID (
	g_testLibGuid,
	0x384498ac, 0x90af, 0x4634, 0xb0, 0x83, 0x2a, 0x9b, 0x2, 0xd6, 0x26, 0x80
	);

enum TestLibCacheSlot
{
	TestLibCacheSlot_Point,
	TestLibCacheSlot_TestClassA,
	TestLibCacheSlot_TestClassB,
	TestLibCacheSlot_TestStruct,
};

//..............................................................................

struct Point
{
	int64_t m_x;
	int64_t m_y;
	int64_t m_z;
	int64_t m_w;
};

//..............................................................................

class TestClassA: public jnc::IfaceHdr
{
public:
	int m_x;

public:
	void
	JNC_CDECL
	destruct ();

	void
	JNC_CDECL
	foo (int x);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class TestClassB: public jnc::IfaceHdr
{
public:
	char m_data [256];

public:
	void
	JNC_CDECL
	markOpaqueGcRoots (jnc::GcHeap* gcHeap);

	bool
	JNC_CDECL
	bar (
		jnc::DataPtr ptr1,
		jnc::DataPtr ptr2,
		jnc::DataPtr ptr3,
		jnc::DataPtr ptr4,
		int a,
		int b
		);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class TestStruct
{
public:
	int m_x;
	double m_y;

public:
	static
	void
	JNC_CDECL
	construct_0 (jnc::DataPtr selfPtr);

	static
	void
	JNC_CDECL
	construct_1 (
		jnc::DataPtr selfPtr,
		int x
		);

	static
	void
	JNC_CDECL
	construct_2 (
		jnc::DataPtr selfPtr,
		double y
		);

	static
	void
	JNC_CDECL
	foo_0 (jnc::DataPtr selfPtr);

	static
	void
	JNC_CDECL
	foo_1 (
		jnc::DataPtr selfPtr,
		int x
		);

	static
	void
	JNC_CDECL
	foo_2 (
		jnc::DataPtr selfPtr,
		double y
		);
};

//..............................................................................

JNC_DECLARE_LIB (TestLib)

#endif
