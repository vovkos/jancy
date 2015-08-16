#pragma once

#include "ApiSlots.h"

//.............................................................................

class TestClass: public jnc::IfaceHdr
{
public:
	JNC_BEGIN_CLASS ("TestClass", ApiSlot_TestClass)
		JNC_OPAQUE_CLASS (TestClass, &enumGcRoots)
		JNC_OPERATOR_NEW (&operatorNew)
		JNC_DESTRUCTOR (TestClass)
		JNC_BINARY_OPERATOR (jnc::BinOpKind_AddAssign, &TestClass::addAssign)
		JNC_BINARY_OPERATOR (jnc::BinOpKind_SubAssign, &TestClass::subAssign)
		JNC_FUNCTION ("foo", &TestClass::foo_0)
		JNC_OVERLOAD (&TestClass::foo_1)
		JNC_OVERLOAD (&TestClass::foo_2)
		JNC_PROPERTY ("m_prop", &TestClass::setProp, &TestClass::setProp)
	JNC_END_CLASS ()

public: // these fields are accessible from Jancy
	jnc::ClassBox <jnc::Multicast> m_onNegative;
	jnc::DataPtr m_propValue;

protected: // opaque section
	int m_internalValue;
	jnc::IfaceHdr* m_internalObject;
	char m_internalData [256];

public:
	~TestClass ();

	static 
	TestClass*
	operatorNew (
		jnc::ClassType* type,
		int value
		);

	static
	void
	enumGcRoots (
		jnc::GcHeap* gcHeap,
		TestClass* self
		);

	int 
	AXL_CDECL
	addAssign (int delta);

	int 
	AXL_CDECL
	subAssign (int delta);

	int
	AXL_CDECL
	foo_0 ();

	int
	AXL_CDECL
	foo_1 (int value);

	int
	AXL_CDECL
	foo_2 (TestClass* src);

	void
	AXL_CDECL
	setProp (jnc::DataPtr ptr);

protected:
	int
	setInternalValue (int value);
};

//.............................................................................
