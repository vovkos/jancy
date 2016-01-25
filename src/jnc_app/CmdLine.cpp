#include "pch.h"
#include "CmdLine.h"

//.............................................................................

CmdLine::CmdLine ()
{
	m_flags = 0;
	m_serverPort = 0;
	m_functionName = "main";
	m_gcAllocSizeTrigger = jnc::rt::GcDef_AllocSizeTrigger;
	m_gcPeriodSizeTrigger = jnc::rt::GcDef_PeriodSizeTrigger;
	m_stackSizeLimit = jnc::rt::RuntimeDef_StackSizeLimit;
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
		m_cmdLine->m_flags |= JncFlag_Help;
		break;

	case CmdLineSwitch_Version:
		m_cmdLine->m_flags |= JncFlag_Version;
		break;

	case CmdLineSwitch_StdInSrc:
		m_cmdLine->m_flags |= JncFlag_StdInSrc;
		break;

	case CmdLineSwitch_SrcNameOverride:
		m_cmdLine->m_srcNameOverride = value;
		break;

	case CmdLineSwitch_DebugInfo:
		m_cmdLine->m_flags |= JncFlag_DebugInfo;
		break;

	case CmdLineSwitch_Jit:
		m_cmdLine->m_flags |= JncFlag_Jit;
		break;

	case CmdLineSwitch_McJit:
		m_cmdLine->m_flags |= JncFlag_Jit | JncFlag_McJit;
		break;

	case CmdLineSwitch_LlvmIr:
		m_cmdLine->m_flags |= JncFlag_LlvmIr;
		break;

	case CmdLineSwitch_SimpleGcSafePoint:
		m_cmdLine->m_flags |= JncFlag_SimpleGcSafePoint;
		break;

	case CmdLineSwitch_CompileOnly:
		m_cmdLine->m_flags |= JncFlag_CompileOnly;
		break;

	case CmdLineSwitch_Run:
		if (m_cmdLine->m_flags & JncFlag_CompileOnly)
		{
			err::setStringError ("conflicting flags (--compile-only/--run)");
			return false;
		}

		break;

	case CmdLineSwitch_RunFunction:
		if (m_cmdLine->m_flags & JncFlag_CompileOnly)
		{
			err::setStringError ("conflicting flags (--compile-only/--run-func)");
			return false;
		}

		m_cmdLine->m_functionName = value;
		break;

	case CmdLineSwitch_PrintReturnValue:
		m_cmdLine->m_flags |= JncFlag_PrintReturnValue;
		break;

	case CmdLineSwitch_Server:
		m_cmdLine->m_flags |= JncFlag_Server;
		m_cmdLine->m_serverPort = atoi (value);
		if (!m_cmdLine->m_serverPort)
		{
			err::setFormatStringError ("invalid server TCP port '%s'", value);
			return false;
		}

		break;

	case CmdLineSwitch_GcAllocSizeTrigger:
		m_cmdLine->m_gcAllocSizeTrigger = parseSizeString (value);
		break;

	case CmdLineSwitch_GcPeriodSizeTrigger:
		m_cmdLine->m_gcPeriodSizeTrigger = parseSizeString (value);
		break;

	case CmdLineSwitch_StackSizeLimit:
		m_cmdLine->m_stackSizeLimit = parseSizeString (value);
		if (!m_cmdLine->m_stackSizeLimit)
		{
			err::setFormatStringError ("invalid stack size '%s'", value);
			return false;
		}

		break;

	case CmdLineSwitch_ImportDir:
		m_cmdLine->m_importDirList.insertTail (value);
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
		m_cmdLine->m_fileName.isEmpty ())
	{
		err::setFormatStringError ("missing input (file-name or --stdin)");
		return false;
	}

	if (!(m_cmdLine->m_flags & JncFlag_CompileOnly))
		m_cmdLine->m_flags |= JncFlag_Jit;

	return true;
}

//.............................................................................
