#include "pch.h"
#include "CmdLine.h"

//.............................................................................

bool
CmdLineParser::onValue (const char* value)
{
	m_cmdLine->m_fileName = value;
	return true;
}

bool
CmdLineParser::onSwitch (
	SwitchKind switchKind,
	const char* value
	)
{
	switch (switchKind)
	{
	case CmdLineSwitch_Help:
		m_cmdLine->m_flags |= CmdLineFlag_Help;
		break;

	case CmdLineSwitch_Version:
		m_cmdLine->m_flags |= CmdLineFlag_Version;
		break;

	case CmdLineSwitch_SourceFile:
		m_cmdLine->m_flags |= CmdLineFlag_SourceFile;
		m_cmdLine->m_sourceFileName = value;
		break;
	}

	return true;
}

bool
CmdLineParser::finalize ()
{
	if (!(m_cmdLine->m_flags & (CmdLineFlag_Help | CmdLineFlag_Version)) &&
		m_cmdLine->m_fileName.isEmpty ())
	{
		err::setFormatStringError ("missing input (file-name or --stdin)");
		return false;
	}

	return true;
}

//.............................................................................
