#pragma once

//.............................................................................

enum JncFlag
{
	JncFlag_Help        = 0x0001,
	JncFlag_Version     = 0x0002,
	JncFlag_LlvmIr      = 0x0004,
	JncFlag_LlvmIr_c    = 0x0008,
	JncFlag_Jit         = 0x0010,
	JncFlag_Jit_mc      = 0x0020,
	JncFlag_RunFunction = 0x0080,
	JncFlag_Server      = 0x0100,
	JncFlag_DebugInfo   = 0x0200,
	JncFlag_StdInSrc    = 0x0400,
};

struct CmdLine
{
	uint_t m_flags;
	uint16_t m_serverPort;
	size_t m_stackSizeLimit;
	size_t m_gcAllocSizeTrigger;
	size_t m_gcPeriodSizeTrigger;

	rtl::String m_srcFileName;
	rtl::String m_srcNameOverride;
	rtl::String m_functionName;

	CmdLine ();
};

//.............................................................................

enum CmdLineSwitchKind
{
	CmdLineSwitchKind_Undefined = 0,
	CmdLineSwitchKind_Help,
	CmdLineSwitchKind_Version,
	CmdLineSwitchKind_StdInSrc,
	CmdLineSwitchKind_LlvmIr,
	CmdLineSwitchKind_LlvmIrComments,
	CmdLineSwitchKind_DebugInfo,
	CmdLineSwitchKind_Jit,
	CmdLineSwitchKind_McJit,
	CmdLineSwitchKind_Run,
	CmdLineSwitchKind_RunFunction = rtl::CmdLineSwitchFlag_HasValue,
	CmdLineSwitchKind_Server,
	CmdLineSwitchKind_GcAllocSizeTrigger,
	CmdLineSwitchKind_GcPeriodSizeTrigger,
	CmdLineSwitchKind_StackSizeLimit,
	CmdLineSwitchKind_SrcNameOverride,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

AXL_RTL_BEGIN_CMD_LINE_SWITCH_TABLE (CmdLineSwitchTable, CmdLineSwitchKind)
	AXL_RTL_CMD_LINE_SWITCH_GROUP ("General options")
	AXL_RTL_CMD_LINE_SWITCH_2 (
		CmdLineSwitchKind_Help,
		"h", "help", NULL,
		"Display this help"
		)
	AXL_RTL_CMD_LINE_SWITCH_2 (
		CmdLineSwitchKind_Version,
		"v", "version", NULL,
		"Display compiler version"
		)
	AXL_RTL_CMD_LINE_SWITCH_2 (
		CmdLineSwitchKind_Server,
		"s", "server", "<port>",
		"Run compiler server on TCP port <port>"
		)
	AXL_RTL_CMD_LINE_SWITCH (
		CmdLineSwitchKind_StdInSrc,
		"stdin", NULL,
		"Get source from STDIN rather than from the file"
		)
	AXL_RTL_CMD_LINE_SWITCH_2 (
		CmdLineSwitchKind_SrcNameOverride,
		"n", "source-name", "<name>",
		"Override source name (defaults to full-path/'stdin')"
		)

	AXL_RTL_CMD_LINE_SWITCH_GROUP ("LLVM IR options")
	AXL_RTL_CMD_LINE_SWITCH_2 (
		CmdLineSwitchKind_LlvmIr,
		"l", "llvm-ir", NULL,
		"Emit LLVM IR (lli-compatible)"
		)
	AXL_RTL_CMD_LINE_SWITCH_2 (
		CmdLineSwitchKind_LlvmIrComments,
		"c", "llvm-ir-comments", NULL,
		"Emit LLVM IR (manual mode w/ comments)"
		)
	AXL_RTL_CMD_LINE_SWITCH_2 (
		CmdLineSwitchKind_DebugInfo,
		"g", "debug-info", NULL,
		"Generate debug information (does not work with legacy JIT)"
		)

	AXL_RTL_CMD_LINE_SWITCH_GROUP ("JIT options")
	AXL_RTL_CMD_LINE_SWITCH_2 (
		CmdLineSwitchKind_Jit,
		"j", "jit", NULL,
		"JIT compiled module"
		)
	AXL_RTL_CMD_LINE_SWITCH_2 (
		CmdLineSwitchKind_McJit,
		"m", "mcjit", NULL,
		"Use MC-JIT engine (does not work on Windows)"
		)
	AXL_RTL_CMD_LINE_SWITCH_2 (
		CmdLineSwitchKind_Run,
		"r", "run", NULL,
		"Run function 'main' (implies JITting)"
		)
	AXL_RTL_CMD_LINE_SWITCH (
		CmdLineSwitchKind_RunFunction,
		"run-function", "<function>",
		"Run function <function> (implies JITting)"
		)
	AXL_RTL_CMD_LINE_SWITCH (
		CmdLineSwitchKind_GcAllocSizeTrigger,
		"gc-alloc-size-trigger", "<size>",
		"Specify the GC alloc size trigger"
		)
	AXL_RTL_CMD_LINE_SWITCH (
		CmdLineSwitchKind_GcPeriodSizeTrigger,
		"gc-period-size-trigger", "<size>",
		"Specify the GC period size trigger"
		)
	AXL_RTL_CMD_LINE_SWITCH (
		CmdLineSwitchKind_StackSizeLimit,
		"stack-size-limit", "<size>",
		"Specify the stack size limit"
		)
AXL_RTL_END_CMD_LINE_SWITCH_TABLE ()

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CmdLineParser: public rtl::CmdLineParser <CmdLineParser, CmdLineSwitchTable>
{
	friend class rtl::CmdLineParser <CmdLineParser, CmdLineSwitchTable>;

protected:
	CmdLine* m_cmdLine;

public:
	CmdLineParser (CmdLine* cmdLine)
	{
		m_cmdLine = cmdLine;
	}

protected:
	bool
	onValue (const char* value);

	bool
	onSwitch (
		SwitchKind switchKind,
		const char* value
		);

	bool
	finalize ();
};

//.............................................................................
