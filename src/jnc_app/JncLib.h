#pragma once

#include "CmdLine.h"

//.............................................................................

class JncLib: public jnc::ext::ExtensionLib
{
public:
	JNC_BEGIN_LIB_MAP ()
		JNC_MAP_FUNCTION ("printf",  &printf)
	JNC_END_LIB_MAP ()

	static
	int
	printf (
		const char* format,
		...
		);
};

//.............................................................................
