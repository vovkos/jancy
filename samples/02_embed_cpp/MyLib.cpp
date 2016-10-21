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
#include "MyLib.h"
#include "TestStruct.h"
#include "TestClass.h"

//..............................................................................

// TODO: add a gc root for g_propValue

jnc::DataPtr g_propValue = jnc::g_nullPtr;

int
stdPrintf (
	const char* format,
	...
	)
{
	va_list va;
	va_start (va, format);
	return vprintf (format, va);
}

void
foo_0 (int x)
{
	printf ("  foo_0 (%d)\n", x);
}

void
foo_1 (double x)
{
	printf ("  foo_1 (%f)\n", x);
}

void
foo_2 (jnc::DataPtr ptr)
{
	printf ("  foo_2 (%s)\n", ptr.m_p);
}

static int g_simplePropValue = 0;

int
getSimpleProp ()
{
	printf ("  getSimpleProp () => %d\n", g_simplePropValue);
	return g_simplePropValue;
}

void
setSimpleProp (int x)
{
	printf ("  setSimpleProp (%d)\n", x);
	g_simplePropValue = x;
}

jnc::DataPtr
getProp ()
{
	printf ("  getProp () => %s\n", g_propValue.m_p);
	return g_propValue;
}

void
setProp_0 (int x)
{
	printf ("  setProp_0 (%d)\n", x);

	char buffer [32];
	int length = sprintf (buffer, "%d", x);
	g_propValue = jnc::strDup (buffer, length);
}

void
setProp_1 (double x)
{
	printf ("  setProp_1 (%f)\n", x);

	char buffer [32];
	int length = sprintf (buffer, "%f", x);
	g_propValue = jnc::strDup (buffer, length);
}

void
setProp_2 (jnc::DataPtr ptr)
{
	printf ("  setProp_2 (%s)\n", ptr.m_p);
	g_propValue = ptr;
}

//..............................................................................

JNC_DEFINE_LIB (
	MyLib,
	g_myLibGuid,
	"MyLib",
	"Sample extension library"
	)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE (MyLib)
JNC_END_LIB_SOURCE_FILE_TABLE ()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE (MyLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (TestClass)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE ()

JNC_BEGIN_LIB_FUNCTION_MAP (MyLib)
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
JNC_END_LIB_FUNCTION_MAP ()

//..............................................................................
