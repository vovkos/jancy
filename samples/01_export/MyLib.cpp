#include "pch.h"
#include "MyLib.h"
#include "axl_g_WarningSuppression.h"

//.............................................................................

int
MyLib::stdPrintf (
	const char* format,
	...
	)
{
	AXL_VA_DECL (va, format);
	return vprintf (format, va.m_va);
}

void
MyLib::foo_0 (int x)
{
	printf ("  MyLib::foo_0 (%d)\n", x);
}

void
MyLib::foo_1 (double x)
{
	printf ("  MyLib::foo_1 (%f)\n", x);
}

void
MyLib::foo_2 (jnc::DataPtr ptr)
{
	printf ("  MyLib::foo_2 (%s)\n", ptr.m_p);
}

static int g_simplePropValue = 0;

int
MyLib::getSimpleProp ()
{
	printf ("  MyLib::getSimpleProp () => %d\n", g_simplePropValue);
	return g_simplePropValue;
}

void
MyLib::setSimpleProp (int x)
{
	printf ("  MyLib::setSimpleProp (%d)\n", x);
	g_simplePropValue = x;
}

char g_propBuffer [64] = { 0 };
jnc::DataPtr g_propValue = { 0 };

jnc::DataPtr 
MyLib::getProp ()
{
	printf ("  MyLib::getProp () => %s\n", g_propValue.m_p);
	return g_propValue;
}

void
MyLib::setProp_0 (int x)
{
	printf ("  MyLib::setProp_0 (%d)\n", x);
	sprintf (g_propBuffer, "%d", x);
	g_propValue.m_p = g_propBuffer;
	g_propValue.m_rangeBegin = g_propBuffer;
	g_propValue.m_rangeEnd = g_propBuffer + sizeof (g_propBuffer);
	g_propValue.m_object = jnc::getStaticBox ();
}

void
MyLib::setProp_1 (double x)
{
	printf ("  MyLib::setProp_1 (%f)\n", x);
	sprintf (g_propBuffer, "%f", x);
	g_propValue.m_p = g_propBuffer;
	g_propValue.m_rangeBegin = g_propBuffer;
	g_propValue.m_rangeEnd = g_propBuffer + sizeof (g_propBuffer);
	g_propValue.m_object = jnc::getStaticBox ();
}

void
MyLib::setProp_2 (jnc::DataPtr ptr)
{
	printf ("  MyLib::setProp_2 (%s)\n", ptr.m_p);
	g_propValue = ptr;
}

//.............................................................................
