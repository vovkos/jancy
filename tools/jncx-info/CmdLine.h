#pragma once

//.............................................................................

enum CmdLineFlag
{
	CmdLineFlag_Help    = 0x0001,
	CmdLineFlag_Version = 0x0002,
	CmdLineFlag_SourceFile = 0x0010,
};

struct CmdLine
{
	uint_t m_flags;
	sl::String m_fileName;
	sl::String m_sourceFileName;

	CmdLine ()
	{
		m_flags = 0;
	}
};

//.............................................................................

enum CmdLineSwitch
{
	CmdLineSwitch_Undefined = 0,
	CmdLineSwitch_Help,
	CmdLineSwitch_Version,
	CmdLineSwitch_SourceFile = sl::CmdLineSwitchFlag_HasValue,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

AXL_SL_BEGIN_CMD_LINE_SWITCH_TABLE (CmdLineSwitchTable, CmdLineSwitch)
	AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitch_Help,
		"h", "help", NULL,
		"Display this help"
		)
	AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitch_Version,
		"v", "version", NULL,
		"Display compiler version"
		)

	AXL_SL_CMD_LINE_SWITCH_2 (
		CmdLineSwitch_SourceFile,
		"f", "file", "<file>",
		"Extract source file <file> from extension"
		)
AXL_SL_END_CMD_LINE_SWITCH_TABLE ()

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CmdLineParser: public sl::CmdLineParser <CmdLineParser, CmdLineSwitchTable>
{
	friend class sl::CmdLineParser <CmdLineParser, CmdLineSwitchTable>;

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
