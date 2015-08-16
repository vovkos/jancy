#include "pch.h"
#include "TestClass.h"
#include "axl_g_WarningSuppression.h"

//.............................................................................

TestClass::~TestClass ()
{
	printf ("  TestClass::~TestClass ()\n");
}

void
TestClass::enumGcRoots (
	jnc::GcHeap* gcHeap,
	TestClass* self
	)
{
	// mark opaque roots (no need to mark roots visible to jancy)

	if (self->m_internalObject)
		gcHeap->markClass (self->m_internalObject->m_box);
}

TestClass*
TestClass::operatorNew (
	jnc::ClassType* type,
	int value
	)
{
	printf ("  TestClass::operatorNew (%d)\n", value);

	jnc::ApiClassBox <TestClass>* object = (jnc::ApiClassBox <TestClass>*) jnc::StdLib::allocateClass (type);
	object->prime (type);
	object->m_internalValue = value;
	sprintf (object->m_internalData, "TestClass (%p)", (TestClass*) object);
	return object;
}

int 
AXL_CDECL
TestClass::addAssign (int delta)
{
	printf ("  TestClass::addAssign (%d)\n", delta);
	return setInternalValue (m_internalValue + delta);
}

int 
AXL_CDECL
TestClass::subAssign (int delta)
{
	printf ("  TestClass::subAssign (%d)\n", delta);
	return setInternalValue (m_internalValue - delta);
}

int
AXL_CDECL
TestClass::foo_0 ()
{
	printf ("  TestClass::foo_0 ()\n");
	return m_internalValue;
}

int
AXL_CDECL
TestClass::foo_1 (int value)
{
	printf ("  TestClass::foo_1 (%d)\n", value);
	return setInternalValue (value);
}

int
AXL_CDECL
TestClass::foo_2 (TestClass* src)
{
	printf ("  TestClass::foo_2 ()\n");
	m_propValue = src->m_propValue;
	return setInternalValue (src->m_internalValue);
}

void
AXL_CDECL
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
		callMulticast (jnc::getCurrentThreadRuntime (), &m_onNegative);

	return value;
}

//.............................................................................
