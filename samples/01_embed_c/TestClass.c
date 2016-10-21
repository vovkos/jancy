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
	&TestClass_markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (TestClass)
	JNC_MAP_CONSTRUCTOR (TestClass_construct)
	JNC_MAP_DESTRUCTOR (TestClass_destruct)
	JNC_MAP_BINARY_OPERATOR (jnc_BinOpKind_AddAssign, TestClass_addAssign)
	JNC_MAP_BINARY_OPERATOR (jnc_BinOpKind_SubAssign, TestClass_subAssign)
	JNC_MAP_FUNCTION ("foo", TestClass_foo_0)
	JNC_MAP_OVERLOAD (TestClass_foo_1)
	JNC_MAP_OVERLOAD (TestClass_foo_2)
	JNC_MAP_PROPERTY ("m_prop", TestClass_setProp, TestClass_setProp)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

void
TestClass_construct (
	TestClass* self,
	int value
	)
{
	printf ("  TestClass_construct (%d)\n", value);

	self->m_internalValue = value;
	sprintf (self->m_internalData, "TestClass (%p)", self);
}

void
TestClass_destruct (TestClass* self)
{
	printf ("  TestClass_destruct ()\n");
}

void
TestClass_markOpaqueGcRoots (
	TestClass* self,
	jnc_GcHeap* gcHeap
	)
{
	// mark opaque roots (no need to mark roots visible to jancy)

	if (self->m_internalObject)
		jnc_GcHeap_markClass (gcHeap, self->m_internalObject->m_box);
}

int
TestClass_setInternalValue (
	TestClass* self,
	int value
	)
{
	self->m_internalValue = value;
	if (self->m_internalValue < 0)
	{
		OnNegativeEventFunc* mc = jnc_getMulticastCallMethodMachineCode (&self->m_onNegative.m_multicast);
		mc (&self->m_onNegative.m_multicast);
	}

	return value;
}

int
TestClass_addAssign (
	TestClass* self,
	int delta
	)
{
	printf ("  TestClass_addAssign (%d)\n", delta);
	return TestClass_setInternalValue (self, self->m_internalValue + delta);
}

int
TestClass_subAssign (
	TestClass* self,
	int delta
	)
{
	printf ("  TestClass_subAssign (%d)\n", delta);
	return TestClass_setInternalValue (self, self->m_internalValue - delta);
}

int
TestClass_foo_0 (TestClass* self)
{
	printf ("  TestClass_foo_0 ()\n");
	return self->m_internalValue;
}

int
TestClass_foo_1 (
	TestClass* self,
	int value
	)
{
	printf ("  TestClass_foo_1 (%d)\n", value);
	return TestClass_setInternalValue (self, value);
}

int
TestClass_foo_2 (
	TestClass* self,
	TestClass* src
	)
{
	printf ("  TestClass_foo_2 ()\n");
	self->m_propValue = src->m_propValue;
	return TestClass_setInternalValue (self, src->m_internalValue);
}

void
TestClass_setProp (
	TestClass* self,
	jnc_DataPtr ptr
	)
{
	printf ("  TestClass_setProp (%s)\n", ptr.m_p);
	self->m_propValue = ptr;
}

//..............................................................................
