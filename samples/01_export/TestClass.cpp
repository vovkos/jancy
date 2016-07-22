#include "pch.h"
#include "TestClass.h"
#include "axl_g_WarningSuppression.h"

//.............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	TestClass,
	"TestClass", 
	g_myLibGuid, 
	MyLibCacheSlot_TestClass,
	TestClass, 
	&TestClass::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (TestClass)
	JNC_MAP_CONSTRUCTOR (&(sl::construct <TestClass, int>))
	JNC_MAP_DESTRUCTOR (&sl::destruct <TestClass>)
	JNC_MAP_BINARY_OPERATOR (jnc::BinOpKind_AddAssign, &TestClass::addAssign)
	JNC_MAP_BINARY_OPERATOR (jnc::BinOpKind_SubAssign, &TestClass::subAssign)
	JNC_MAP_FUNCTION ("foo", &TestClass::foo_0)
	JNC_MAP_OVERLOAD (&TestClass::foo_1)
	JNC_MAP_OVERLOAD (&TestClass::foo_2)
	JNC_MAP_PROPERTY ("m_prop", &TestClass::setProp, &TestClass::setProp)
JNC_END_TYPE_FUNCTION_MAP ()

//.............................................................................

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
TestClass::markOpaqueGcRoots (jnc::GcHeap* gcHeap)
{
	// mark opaque roots (no need to mark roots visible to jancy)

	if (m_internalObject)
		gcHeap->markClass (m_internalObject->m_box);
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
		callMulticast (jnc::getCurrentThreadRuntime (), m_onNegative);

	return value;
}

//.............................................................................
