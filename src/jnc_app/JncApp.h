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

#include "CmdLine.h"

//..............................................................................

class JncApp
{
protected:
	CmdLine* m_cmdLine;
	jnc::AutoModule m_module;
	jnc::AutoRuntime m_runtime;
	sl::Array<char> m_stdInBuffer;

public:
	JncApp(CmdLine* cmdLine);

	bool
	parse();

	bool
	compile()
	{
		return
			m_module->compile() &&
			((m_cmdLine->m_compileFlags & jnc::ModuleCompileFlag_DisableCodeGen) ||
			m_module->optimize(m_cmdLine->m_optLevel));
	}

	bool
	jit()
	{
		return m_module->jit();
	}

	void
	printLlvmIr()
	{
		printf("%s", m_module->getLlvmIrString_v ());
	}

	bool
	generateDocumentation();

	bool
	runFunction(int* returnValue = NULL);

protected:
	static
	bool_t
	compileErrorHandler(
		void* context,
		jnc::ModuleCompileErrorKind errorKind
		);
};

//..............................................................................
