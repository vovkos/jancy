#pragma once

#include "MyLib.h"

JNC_DECLARE_OPAQUE_CLASS_TYPE (TestClass)

//.............................................................................

typedef 
void
OnNegativeEventFunc (jnc_Multicast* multicast);

struct TestClass
{
	jnc_IfaceHdr m_ifaceHdr;

	// these fields are accessible from Jancy
	
	struct
	{
		jnc_Box m_box;
		jnc_Multicast m_multicast;
	} m_onNegative;

	jnc_DataPtr m_propValue;

	// opaque section

	int m_internalValue;
	jnc_IfaceHdr* m_internalObject;
	char m_internalData [256];
};

typedef struct TestClass TestClass;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
TestClass_construct (
	TestClass* self,
	int value
	);

void
TestClass_destruct (TestClass* self);
	
void
TestClass_markOpaqueGcRoots (
	TestClass* self,
	jnc_GcHeap* gcHeap
	);

int 
TestClass_addAssign (
	TestClass* self,
	int delta
	);

int 
TestClass_subAssign (
	TestClass* self,
	int delta
	);

int
TestClass_foo_0 (TestClass* self);

int
TestClass_foo_1 (
	TestClass* self,
	int value
	);

int
TestClass_foo_2 (
	TestClass* self,
	TestClass* src
	);

void
TestClass_setProp (
	TestClass* self,
	jnc_DataPtr ptr
	);

//.............................................................................
