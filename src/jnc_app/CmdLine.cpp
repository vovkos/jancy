#include "pch.h"
#include "CmdLine.h"

//.............................................................................

CmdLine::CmdLine ()
{
	m_flags = 0;
	m_serverPort = 0;
	m_gcAllocSizeTrigger = jnc::GcDef_AllocSizeTrigger;
	m_gcPeriodSizeTrigger = jnc::GcDef_PeriodSizeTrigger;
	m_stackSizeLimit = jnc::RuntimeDef_StackSizeLimit;
}

//.............................................................................

size_t
parseSizeString (const char* string)
{
	if (!string)
		return 0;

	size_t length = strlen (string);
	if (!length)
		return 0;

	size_t multiplier = 1;

	char c = string [length - 1];
	if (isalpha (c))
	{
		switch (c)
		{
		case 'K':
			multiplier = 1024;
			break;

		case 'M':
			multiplier = 1024 * 1024;
			break;

		default:
			return 0;
		}
	}

	return strtoul (string, NULL, 10) * multiplier;
}

//.............................................................................

bool
CmdLineParser::onValue (const char* value)
{
	m_cmdLine->m_srcFileName = value;
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
	case CmdLineSwitchKind_Help:
		m_cmdLine->m_flags |= JncFlag_Help;
		break;

	case CmdLineSwitchKind_Version:
		m_cmdLine->m_flags |= JncFlag_Version;
		break;

	case CmdLineSwitchKind_StdInSrc:
		m_cmdLine->m_flags |= JncFlag_StdInSrc;
		break;

	case CmdLineSwitchKind_SrcNameOverride:
		m_cmdLine->m_srcNameOverride = value;
		break;

	case CmdLineSwitchKind_DebugInfo:
		m_cmdLine->m_flags |= JncFlag_DebugInfo;
		break;

	case CmdLineSwitchKind_Jit:
		m_cmdLine->m_flags |= JncFlag_Jit;
		break;

	case CmdLineSwitchKind_McJit:
		m_cmdLine->m_flags |= JncFlag_Jit | JncFlag_McJit;
		break;

	case CmdLineSwitchKind_LlvmIr:
		m_cmdLine->m_flags |= JncFlag_LlvmIr;
		break;

	case CmdLineSwitchKind_SimpleGcSafePoint:
		m_cmdLine->m_flags |= JncFlag_SimpleGcSafePoint;
		break;

	case CmdLineSwitchKind_Run:
		m_cmdLine->m_flags |= JncFlag_Jit | JncFlag_RunFunction;
		m_cmdLine->m_functionName = "main";
		break;

	case CmdLineSwitchKind_RunFunction:
		m_cmdLine->m_flags |= JncFlag_Jit | JncFlag_RunFunction;
		m_cmdLine->m_functionName = value;
		break;

	case CmdLineSwitchKind_Server:
		m_cmdLine->m_flags |= JncFlag_Server;
		m_cmdLine->m_serverPort = atoi (value);
		if (!m_cmdLine->m_serverPort)
		{
			err::setFormatStringError ("invalid server TCP port '%s'", value);
			return false;
		}

		break;

	case CmdLineSwitchKind_GcAllocSizeTrigger:
		m_cmdLine->m_gcAllocSizeTrigger = parseSizeString (value);
		break;

	case CmdLineSwitchKind_GcPeriodSizeTrigger:
		m_cmdLine->m_gcPeriodSizeTrigger = parseSizeString (value);
		break;

	case CmdLineSwitchKind_StackSizeLimit:
		m_cmdLine->m_stackSizeLimit = parseSizeString (value);
		if (!m_cmdLine->m_stackSizeLimit)
		{
			err::setFormatStringError ("invalid stack size '%s'", value);
			return false;
		}

		break;
	}

	return true;
}

bool
CmdLineParser::finalize ()
{
	if (!(m_cmdLine->m_flags & (
			JncFlag_Help |
			JncFlag_Version |
			JncFlag_Server |
			JncFlag_StdInSrc
			)) &&
		m_cmdLine->m_srcFileName.isEmpty ())
	{
		err::setFormatStringError ("missing input (source-file-name or --stdin)");
		return false;
	}

	return true;
}

//.............................................................................
