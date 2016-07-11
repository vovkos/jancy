#pragma once

#include "MyLibGlobals.h"

//.............................................................................

class TestClass: public jnc::IfaceHdr
{
public:
	JNC_OPAQUE_CLASS_TYPE_INFO (TestClass, &TestClass::markOpaqueGcRoots)

	JNC_BEGIN_CLASS_TYPE_MAP ("TestClass", g_myLibCacheSlot, MyLibTypeCacheSlot_TestClass)
		JNC_MAP_CONSTRUCTOR (&(sl::construct <TestClass, int>))
		JNC_MAP_DESTRUCTOR (&sl::destruct <TestClass>)
		JNC_MAP_BINARY_OPERATOR (jnc::BinOpKind_AddAssign, &TestClass::addAssign)
		JNC_MAP_BINARY_OPERATOR (jnc::BinOpKind_SubAssign, &TestClass::subAssign)
		JNC_MAP_FUNCTION ("foo", &TestClass::foo_0)
		JNC_MAP_OVERLOAD (&TestClass::foo_1)
		JNC_MAP_OVERLOAD (&TestClass::foo_2)
		JNC_MAP_PROPERTY ("m_prop", &TestClass::setProp, &TestClass::setProp)
	JNC_END_CLASS_TYPE_MAP ()

public: // these fields are accessible from Jancy
	jnc::ClassBox <jnc::Multicast> m_onNegative;
	jnc::DataPtr m_propValue;

protected: // opaque section
	int m_internalValue;
	jnc::IfaceHdr* m_internalObject;
	char m_internalData [256];

public:
	TestClass (int value);

	~TestClass ();
	
	void
	AXL_CDECL
	markOpaqueGcRoots (jnc::rt::GcHeap* gcHeap);

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
