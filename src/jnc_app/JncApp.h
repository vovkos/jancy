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

struct TimeReport {
	uint64_t m_parseTime;
	uint64_t m_compileTime;
	uint64_t m_optimizeTime;
	uint64_t m_jitTime;
	uint64_t m_runTime;
	uint64_t m_documentationTime;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class JncApp {
protected:
	CmdLine* m_cmdLine;
	jnc::AutoModule m_module;
	jnc::AutoRuntime m_runtime;
	sl::Array<char> m_stdInBuffer;
	TimeReport m_timeReport;

public:
	JncApp(CmdLine* cmdLine);

	const TimeReport&
	getTimeReport() const {
		return m_timeReport;
	}

	bool
	parse();

	bool
	compile();

	bool
	jit();

	bool
	generateDocumentation();

	bool
	runFunction(int* returnValue = NULL);

	void
	printLlvmIr() {
		printf("%s", m_module->getLlvmIrString_v());
	}

	void
	printTimeReport();

protected:
	static
	bool_t
	compileErrorHandler(
		void* context,
		jnc::ModuleCompileErrorKind errorKind
	);
};

//..............................................................................
