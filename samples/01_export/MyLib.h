#pragma once

#include "TestStruct.h"
#include "TestClass.h"

//.............................................................................

class MyLib: public jnc::StdLib
{
public:
	JNC_BEGIN_LIB ()
		JNC_STD_FUNCTION (jnc::StdFunction_Printf,  &stdPrintf)
		JNC_FUNCTION ("foo",  &foo_0)
		JNC_OVERLOAD (&foo_1)
		JNC_OVERLOAD (&foo_2)
		JNC_PROPERTY ("g_simpleProp",  &getSimpleProp, &setSimpleProp)
		JNC_PROPERTY ("g_prop",  &getProp, &setProp_0)
		JNC_OVERLOAD (&setProp_1)
		JNC_OVERLOAD (&setProp_2)
		JNC_TYPE (TestStruct)
		JNC_TYPE (TestClass)
		JNC_LIB (jnc::StdLib)
	JNC_END_LIB ()

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

//.............................................................................
