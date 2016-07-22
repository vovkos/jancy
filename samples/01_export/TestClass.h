#pragma once

#include "MyLib.h"

JNC_DECLARE_OPAQUE_CLASS_TYPE (TestClass)

//.............................................................................

class TestClass: public jnc::IfaceHdr
{
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
	markOpaqueGcRoots (jnc::GcHeap* gcHeap);

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
