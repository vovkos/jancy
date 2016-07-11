#pragma once

#include "TestStruct.h"
#include "TestClass.h"

//.............................................................................

class MyLib: public jnc::ext::ExtensionLib
{
public:
	JNC_BEGIN_LIB_MAP ()
		JNC_MAP_FUNCTION ("printf", &stdPrintf)
		JNC_MAP_FUNCTION ("foo",    &foo_0)
		JNC_MAP_OVERLOAD (&foo_1)
		JNC_MAP_OVERLOAD (&foo_2)
		JNC_MAP_PROPERTY ("g_simpleProp",  &getSimpleProp, &setSimpleProp)
		JNC_MAP_PROPERTY ("g_prop",  &getProp, &setProp_0)
		JNC_MAP_OVERLOAD (&setProp_1)
		JNC_MAP_OVERLOAD (&setProp_2)
		JNC_MAP_TYPE (TestStruct)
		JNC_MAP_TYPE (TestClass)
	JNC_END_LIB_MAP ()

	JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE ()
		JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (TestClass)
	JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE ()

	static
	int
	stdPrintf (
		const char* format,
		...
		);

	static
	void
	foo_0 (int x);

	static
	void
	foo_1 (double x);

	static
	void
	foo_2 (jnc::DataPtr ptr);

	static
	int
	getSimpleProp ();

	static
	void
	setSimpleProp (int x);

	static
	jnc::DataPtr
	getProp ();

	static
	void
	setProp_0 (int x);

	static
	void
	setProp_1 (double x);

	static
	void
	setProp_2 (jnc::DataPtr ptr);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

MyLib*
getMyLib (jnc::ext::ExtensionLibHost* host);

//.............................................................................
