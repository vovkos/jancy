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

#pragma once

struct CmdLine;

//..............................................................................

class JncApp
{
protected:
	CmdLine* m_cmdLine;
	jnc::AutoModule m_module;
	jnc::AutoRuntime m_runtime;
	sl::Array <char> m_stdInBuffer;

public:
	JncApp (CmdLine* cmdLine);

	bool
	parse ();

	bool
	compile ()
	{
		return m_module->compile ();
	}

	bool
	jit ()
	{
		return m_module->jit ();
	}

	void
	printLlvmIr ()
	{
		printf ("%s", m_module->getLlvmIrString_v ());
	}

	bool
	generateDocumentation ();

	bool
	runFunction (int* returnValue = NULL);
};

//..............................................................................
