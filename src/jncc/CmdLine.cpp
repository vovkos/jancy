#include "pch.h"
#include "CmdLine.h"

//.............................................................................

TCmdLine::TCmdLine ()
{
	m_Flags = 0;
	m_ServerPort = 0;
	m_GcHeapSize = 16 * 1024;
	m_StackSize = 16 * 1024;
}

//.............................................................................

size_t
ParseSizeString (const char* pString)
{
	if (!pString)
		return 0;

	size_t Length = strlen (pString);
	if (!Length)
		return 0;

	size_t Multiplier = 1;

	char c = pString [Length - 1];
	if (isalpha (c))
	{
		switch (c)
		{
		case 'K':
			Multiplier = 1024;
			break;

		case 'M':
			Multiplier = 1024 * 1024;
			break;

		default:
			return 0;
		}
	}

	return strtoul (pString, NULL, 10) * Multiplier;
}

//.............................................................................

bool
CCmdLineParser::OnValue (const char* pValue)
{
	m_pCmdLine->m_SrcFileName = pValue;
	return true;
}

bool
CCmdLineParser::OnSwitch (
	ESwitch Switch,
	const char* pValue
	)
{
	switch (Switch)
	{
	case ECmdLineSwitch_Help:
		m_pCmdLine->m_Flags |= EJncFlag_Help;
		break;

	case ECmdLineSwitch_Version:
		m_pCmdLine->m_Flags |= EJncFlag_Version;
		break;

	case ECmdLineSwitch_StdInSrc:
		m_pCmdLine->m_Flags |= EJncFlag_StdInSrc;
		break;

	case ECmdLineSwitch_SrcNameOverride:
		m_pCmdLine->m_SrcNameOverride = pValue;
		break;

	case ECmdLineSwitch_DebugInfo:
		m_pCmdLine->m_Flags |= EJncFlag_DebugInfo;
		break;

	case ECmdLineSwitch_Jit:
		m_pCmdLine->m_Flags |= EJncFlag_Jit;
		break;

	case ECmdLineSwitch_McJit:
		m_pCmdLine->m_Flags |= EJncFlag_Jit;
		m_pCmdLine->m_Flags |= EJncFlag_Jit_mc;
		break;

	case ECmdLineSwitch_LlvmIr:
		m_pCmdLine->m_Flags |= EJncFlag_LlvmIr;
		break;

	case ECmdLineSwitch_LlvmIrComments:
		m_pCmdLine->m_Flags |= EJncFlag_LlvmIr;
		m_pCmdLine->m_Flags |= EJncFlag_LlvmIr_c;
		break;

	case ECmdLineSwitch_Disassembly:
		m_pCmdLine->m_Flags |= EJncFlag_Disassembly;
		break;

	case ECmdLineSwitch_Run:
		m_pCmdLine->m_Flags |= EJncFlag_Jit;
		m_pCmdLine->m_Flags |= EJncFlag_RunFunction;
		m_pCmdLine->m_FunctionName = "main";
		break;

	case ECmdLineSwitch_RunFunction:
		m_pCmdLine->m_Flags |= EJncFlag_Jit;
		m_pCmdLine->m_Flags |= EJncFlag_RunFunction;
		m_pCmdLine->m_FunctionName = pValue;
		break;

	case ECmdLineSwitch_Server:
		m_pCmdLine->m_Flags |= EJncFlag_Server;
		m_pCmdLine->m_ServerPort = atoi (pValue);
		if (!m_pCmdLine->m_ServerPort)
		{
			err::SetFormatStringError ("invalid server TCP port '%s'", pValue);
			return false;
		}

		break;

	case ECmdLineSwitch_HeapSize:
		m_pCmdLine->m_GcHeapSize = ParseSizeString (pValue);
		if (!m_pCmdLine->m_GcHeapSize)
		{
			err::SetFormatStringError ("invalid GC heap size '%s'", pValue);
			return false;
		}

		break;

	case ECmdLineSwitch_StackSize:
		m_pCmdLine->m_StackSize = ParseSizeString (pValue);
		if (!m_pCmdLine->m_StackSize)
		{
			err::SetFormatStringError ("invalid stack size '%s'", pValue);
			return false;
		}

		break;
	}

	return true;
}

bool
CCmdLineParser::Finalize ()
{
	if (!(m_pCmdLine->m_Flags & (
			EJncFlag_Help |
			EJncFlag_Version |
			EJncFlag_Server |
			EJncFlag_StdInSrc
			)) &&
		m_pCmdLine->m_SrcFileName.IsEmpty ())
	{
		err::SetFormatStringError ("missing input (source-file-name or --stdin)");
		return false;
	}

	return true;
}

//.............................................................................
