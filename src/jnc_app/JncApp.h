#pragma once

struct CmdLine;

//.............................................................................

class JncApp
{
protected:
	CmdLine* m_cmdLine;
	jnc::ct::Module m_module;
	jnc::rt::Runtime m_runtime;

public:
	JncApp (CmdLine* cmdLine)
	{
		m_cmdLine = cmdLine;
	}

	bool
	initialize ();

	bool
	parse ();

	bool
	compile ()
	{
		return m_module.compile ();
	}

	bool
	jit ()
	{
		return m_module.jit ();
	}

	void
	printLlvmIr ()
	{
		printf ("%s", m_module.getLlvmIrString ().cc ());
	}

	bool
	generateDocumentation ();

	bool
	runFunction (int* returnValue = NULL);
};

//.............................................................................
