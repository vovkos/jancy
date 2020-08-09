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

//..............................................................................

enum JncFlag
{
	JncFlag_Help             = 0x000001,
	JncFlag_Version          = 0x000002,
	JncFlag_LlvmIr           = 0x000004,
	JncFlag_Compile          = 0x000008,
	JncFlag_DisableCodeGen   = 0x000010,
	JncFlag_Jit              = 0x000020,
	JncFlag_Run              = 0x000080,
	JncFlag_StdInSrc         = 0x000400,
	JncFlag_PrintReturnValue = 0x000800,
};

struct CmdLine
{
	uint_t m_flags;
	uint_t m_compileFlags;
	uint_t m_optLevel;
	jnc::GcSizeTriggers m_gcSizeTriggers;

	sl::String m_srcNameOverride;
	sl::String m_functionName;
	sl::String m_extensionSrcFileName;
	sl::String m_outputDir;
	sl::String m_excludeRegex;

	sl::BoxList<sl::String> m_fileNameList;
	sl::BoxList<sl::String> m_importDirList;
	sl::BoxList<sl::String> m_sourceDirList;
	sl::BoxList<sl::String> m_ignoredImportList;
	sl::BoxList<sl::String> m_requireList;

	CmdLine();
};

//..............................................................................

enum CmdLineSwitch
{
	CmdLineSwitch_Undefined = 0,
	CmdLineSwitch_Help,
	CmdLineSwitch_Version,
	CmdLineSwitch_StdInSrc,
	CmdLineSwitch_SrcNameOverride,
	CmdLineSwitch_CompileOnly,
	CmdLineSwitch_DisableCodeGen,
	CmdLineSwitch_Require,
	CmdLineSwitch_Documentation,
	CmdLineSwitch_Exclude,
	CmdLineSwitch_OutputDir,
	CmdLineSwitch_SourceDir,
	CmdLineSwitch_ImportDir,
	CmdLineSwitch_IgnoreImport,
	CmdLineSwitch_IgnoreOpaqueClassTypeInfo,
	CmdLineSwitch_PrintReturnValue,
	CmdLineSwitch_LlvmIr,
	CmdLineSwitch_DebugInfo,
	CmdLineSwitch_Jit,
	CmdLineSwitch_McJit,
	CmdLineSwitch_SimpleGcSafePoint,
	CmdLineSwitch_StdLibDoc,
	CmdLineSwitch_DisableDoxyComment,
	CmdLineSwitch_Run,
	CmdLineSwitch_RunFunction,
	CmdLineSwitch_GcAllocSizeTrigger,
	CmdLineSwitch_GcPeriodSizeTrigger,
	CmdLineSwitch_GcSafePointInPrologue,
	CmdLineSwitch_OptLevel,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

AXL_SL_BEGIN_CMD_LINE_SWITCH_TABLE(CmdLineSwitchTable, CmdLineSwitch)
	AXL_SL_CMD_LINE_SWITCH_GROUP("General options")
	AXL_SL_CMD_LINE_SWITCH_2(
		CmdLineSwitch_Help,
		"h", "help", NULL,
		"Display this help"
		)
	AXL_SL_CMD_LINE_SWITCH_2(
		CmdLineSwitch_Version,
		"v", "version", NULL,
		"Display compiler version"
		)
	AXL_SL_CMD_LINE_SWITCH(
		CmdLineSwitch_StdInSrc,
		"stdin", NULL,
		"Get source from STDIN rather than from the file"
		)
	AXL_SL_CMD_LINE_SWITCH_2(
		CmdLineSwitch_SrcNameOverride,
		"n", "source-name", "<name>",
		"Override source name for STDIN (defaults to 'stdin')"
		)

	AXL_SL_CMD_LINE_SWITCH_GROUP("Compilation options")
	AXL_SL_CMD_LINE_SWITCH_2(
		CmdLineSwitch_CompileOnly,
		"c", "compile-only", NULL,
		"Compile only (no run)"
		)
	AXL_SL_CMD_LINE_SWITCH(
		CmdLineSwitch_DisableCodeGen,
		"disable-code-gen", NULL,
		"Disable code generation"
		)
	AXL_SL_CMD_LINE_SWITCH(
		CmdLineSwitch_Require,
		"require", "<symbol>",
		"Marks a symbol as required"
		)
	AXL_SL_CMD_LINE_SWITCH_3(
		CmdLineSwitch_Documentation,
		"d", "doc", "documentation", NULL,
		"Generate documentation"
		)
	AXL_SL_CMD_LINE_SWITCH_2(
		CmdLineSwitch_OutputDir,
		"X", "xml-dir", "<dir>",
		"Specify the output XML directory (for documentation)"
		)
	AXL_SL_CMD_LINE_SWITCH_3(
		CmdLineSwitch_SourceDir,
		"S", "src-dir", "source-dir", "<dir>",
		"Add a directory with source files"
		)
	AXL_SL_CMD_LINE_SWITCH_2(
		CmdLineSwitch_Exclude,
		"x", "exclude", "<regex>",
		"Exclude source file names matching a regular expression"
		)
	AXL_SL_CMD_LINE_SWITCH_2(
		CmdLineSwitch_ImportDir,
		"I", "import-dir", "<dir>",
		"Add an import directory"
		)
	AXL_SL_CMD_LINE_SWITCH(
		CmdLineSwitch_IgnoreImport,
		"ignore-import", "<file>",
		"Ignore imports of a specific file (for documentation)"
		)
	AXL_SL_CMD_LINE_SWITCH(
		CmdLineSwitch_IgnoreOpaqueClassTypeInfo,
		"ignore-opaque", NULL,
		"Ignore opaque class type information (for testing)"
		)
	AXL_SL_CMD_LINE_SWITCH_2(
		CmdLineSwitch_LlvmIr,
		"l", "llvm-ir", NULL,
		"Emit LLVM IR (lli-compatible)"
		)
	AXL_SL_CMD_LINE_SWITCH_2(
		CmdLineSwitch_DebugInfo,
		"g", "debug-info", NULL,
		"Generate debug information (does not work with legacy JIT)"
		)
	AXL_SL_CMD_LINE_SWITCH_2(
		CmdLineSwitch_Jit,
		"j", "jit", NULL,
		"JIT compiled module"
		)
	AXL_SL_CMD_LINE_SWITCH_2(
		CmdLineSwitch_McJit,
		"m", "mcjit", NULL,
		"Use MC-JIT engine (does not work on Windows)"
		)
	AXL_SL_CMD_LINE_SWITCH(
		CmdLineSwitch_SimpleGcSafePoint,
		"simple-gc-safe-point", NULL,
		"Use simple GC safe-point call (rather than guard page)"
		)
	AXL_SL_CMD_LINE_SWITCH(
		CmdLineSwitch_GcSafePointInPrologue,
		"gc-safe-point-prologue", NULL,
		"Add GC safe-points to function prologues"
		)
	AXL_SL_CMD_LINE_SWITCH_2(
		CmdLineSwitch_OptLevel,
		"O", "opt", "<n>",
		"Enable LLVM optimization passes level <n> (0 to 3)"
		)
	AXL_SL_CMD_LINE_SWITCH(
		CmdLineSwitch_StdLibDoc,
		"std-lib-doc", NULL,
		"Enable documentation of standard libraries"
		)
	AXL_SL_CMD_LINE_SWITCH(
		CmdLineSwitch_DisableDoxyComment,
		"no-doxy-comment", "<doxy-comment>",
		"Disable specific doxy comment (1-4): ///, //!, /**, /*!"
		)

	AXL_SL_CMD_LINE_SWITCH_GROUP("Runtime options")
	AXL_SL_CMD_LINE_SWITCH_2(
		CmdLineSwitch_Run,
		"r", "run", NULL,
		"Compile and run (default)"
		)
	AXL_SL_CMD_LINE_SWITCH_3(
		CmdLineSwitch_RunFunction,
		"f", "runf", "run-func", "<name>",
		"Override entry function (defaults to 'main')"
		)
	AXL_SL_CMD_LINE_SWITCH_2(
		CmdLineSwitch_PrintReturnValue,
		"p", "print-retval", NULL,
		"Print return value (rather than use it for exit-code)"
		)
	AXL_SL_CMD_LINE_SWITCH(
		CmdLineSwitch_GcAllocSizeTrigger,
		"gc-alloc-size-trigger", "<size>",
		"Specify the GC alloc size trigger"
		)
	AXL_SL_CMD_LINE_SWITCH(
		CmdLineSwitch_GcPeriodSizeTrigger,
		"gc-period-size-trigger", "<size>",
		"Specify the GC period size trigger"
		)
AXL_SL_END_CMD_LINE_SWITCH_TABLE()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

AXL_SL_BEGIN_STRING_HASH_TABLE(DoxyCommentMap, uint_t)
	AXL_SL_HASH_TABLE_ENTRY("///", jnc::ModuleCompileFlag_DisableDoxyComment1)
	AXL_SL_HASH_TABLE_ENTRY("//!", jnc::ModuleCompileFlag_DisableDoxyComment2)
	AXL_SL_HASH_TABLE_ENTRY("/**", jnc::ModuleCompileFlag_DisableDoxyComment3)
	AXL_SL_HASH_TABLE_ENTRY("/*!", jnc::ModuleCompileFlag_DisableDoxyComment4)
	AXL_SL_HASH_TABLE_ENTRY("1",   jnc::ModuleCompileFlag_DisableDoxyComment1)
	AXL_SL_HASH_TABLE_ENTRY("2",   jnc::ModuleCompileFlag_DisableDoxyComment2)
	AXL_SL_HASH_TABLE_ENTRY("3",   jnc::ModuleCompileFlag_DisableDoxyComment3)
	AXL_SL_HASH_TABLE_ENTRY("4",   jnc::ModuleCompileFlag_DisableDoxyComment4)
AXL_SL_END_STRING_HASH_TABLE()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CmdLineParser: public sl::CmdLineParser<CmdLineParser, CmdLineSwitchTable>
{
	friend class sl::CmdLineParser<CmdLineParser, CmdLineSwitchTable>;

protected:
	CmdLine* m_cmdLine;

public:
	CmdLineParser(CmdLine* cmdLine)
	{
		m_cmdLine = cmdLine;
	}

protected:
	bool
	onValue(const sl::StringRef& value);

	bool
	onSwitch(
		SwitchKind switchKind,
		const sl::StringRef& value
		);

	bool
	finalize();

	bool
	scanSourceDirs();
};

//..............................................................................
