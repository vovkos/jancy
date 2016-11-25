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
#include "TestClass.h"

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	TestClass,
	"TestClass",
	g_myLibGuid,
	MyLibCacheSlot_TestClass,
	TestClass,
	&TestClass::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (TestClass)
	JNC_MAP_CONSTRUCTOR (&(jnc::construct <TestClass, int>))
	JNC_MAP_DESTRUCTOR (&jnc::destruct <TestClass>)
	JNC_MAP_BINARY_OPERATOR (jnc::BinOpKind_AddAssign, &TestClass::addAssign)
	JNC_MAP_BINARY_OPERATOR (jnc::BinOpKind_SubAssign, &TestClass::subAssign)
	JNC_MAP_FUNCTION ("foo", &TestClass::foo_0)
	JNC_MAP_OVERLOAD (&TestClass::foo_1)
	JNC_MAP_OVERLOAD (&TestClass::foo_2)
	JNC_MAP_PROPERTY ("m_prop", &TestClass::setProp, &TestClass::setProp)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

TestClass::TestClass (int value)
{
	printf ("  TestClass::TestClass (%d)\n", value);

	m_internalValue = value;
	sprintf (m_internalData, "TestClass (%p)", this);
}

TestClass::~TestClass ()
{
	printf ("  TestClass::~TestClass ()\n");
}

void
JNC_CDECL
TestClass::markOpaqueGcRoots (jnc::GcHeap* gcHeap)
{
	// mark opaque roots (no need to mark roots visible to jancy)

	if (m_internalObject)
		gcHeap->markClass (m_internalObject->m_box);
}

int
JNC_CDECL
TestClass::addAssign (int delta)
{
	printf ("  TestClass::addAssign (%d)\n", delta);
	return setInternalValue (m_internalValue + delta);
}

int
JNC_CDECL
TestClass::subAssign (int delta)
{
	printf ("  TestClass::subAssign (%d)\n", delta);
	return setInternalValue (m_internalValue - delta);
}

int
JNC_CDECL
TestClass::foo_0 ()
{
	printf ("  TestClass::foo_0 ()\n");
	return m_internalValue;
}

int
JNC_CDECL
TestClass::foo_1 (int value)
{
	printf ("  TestClass::foo_1 (%d)\n", value);
	return setInternalValue (value);
}

int
JNC_CDECL
TestClass::foo_2 (TestClass* src)
{
	printf ("  TestClass::foo_2 ()\n");
	m_propValue = src->m_propValue;
	return setInternalValue (src->m_internalValue);
}

void
JNC_CDECL
TestClass::setProp (jnc::DataPtr ptr)
{
	printf ("  TestClass::setProp (%s)\n", ptr.m_p);
	m_propValue = ptr;
}

int
TestClass::setInternalValue (int value)
{
	m_internalValue = value;
	if (m_internalValue < 0)
		callMulticast (jnc::getCurrentThreadRuntime (), m_onNegative);

	return value;
}

//..............................................................................
