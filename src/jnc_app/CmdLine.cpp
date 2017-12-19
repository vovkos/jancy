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

#include "pch.h"
#include "CmdLine.h"

//..............................................................................

CmdLine::CmdLine ()
{
	m_flags = JncFlag_Run;
	m_doxyCommentFlags = 0;
	m_functionName = "main";
	m_outputDir = ".";
	m_gcSizeTriggers.m_allocSizeTrigger = jnc::GcDef_AllocSizeTrigger;
	m_gcSizeTriggers.m_periodSizeTrigger = jnc::GcDef_PeriodSizeTrigger;
	m_stackSizeLimit = jnc::RuntimeDef_StackSizeLimit;
}

//..............................................................................

size_t
parseSizeString (const sl::StringRef& string)
{
	size_t length = string.getLength ();
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

	return strtoul (string.cp (), NULL, 10) * multiplier;
}

//..............................................................................

bool
CmdLineParser::onValue (const sl::StringRef& value)
{
	m_cmdLine->m_fileNameList.insertTail (value);
	return true;
}

bool
CmdLineParser::onSwitch (
	SwitchKind switchKind,
	const sl::StringRef& value
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
		m_cmdLine->m_flags |= JncFlag_Compile | JncFlag_Jit;
		break;

	case CmdLineSwitch_McJit:
		m_cmdLine->m_flags |= JncFlag_Compile | JncFlag_Jit | JncFlag_McJit;
		break;

	case CmdLineSwitch_LlvmIr:
		m_cmdLine->m_flags |= JncFlag_Compile | JncFlag_LlvmIr;
		break;

	case CmdLineSwitch_SimpleGcSafePoint:
		m_cmdLine->m_flags |= JncFlag_SimpleGcSafePoint;
		break;

	case CmdLineSwitch_CompileOnly:
		m_cmdLine->m_flags &= ~JncFlag_Run;
		break;

	case CmdLineSwitch_Documentation:
		m_cmdLine->m_flags &= ~JncFlag_Run;
		m_cmdLine->m_flags |= JncFlag_Documentation;
		break;

	case CmdLineSwitch_StdLibDoc:
		m_cmdLine->m_flags |= JncFlag_StdLibDoc;
		break;

	case CmdLineSwitch_Run:
		m_cmdLine->m_flags |= JncFlag_Run;
		break;

	case CmdLineSwitch_RunFunction:
		m_cmdLine->m_flags |= JncFlag_Run;
		m_cmdLine->m_functionName = value;
		break;

	case CmdLineSwitch_PrintReturnValue:
		m_cmdLine->m_flags |= JncFlag_PrintReturnValue;
		break;

	case CmdLineSwitch_GcAllocSizeTrigger:
		m_cmdLine->m_gcSizeTriggers.m_allocSizeTrigger = parseSizeString (value);
		break;

	case CmdLineSwitch_GcPeriodSizeTrigger:
		m_cmdLine->m_gcSizeTriggers.m_periodSizeTrigger = parseSizeString (value);
		break;

	case CmdLineSwitch_StackSizeLimit:
		m_cmdLine->m_stackSizeLimit = parseSizeString (value);
		if (!m_cmdLine->m_stackSizeLimit)
		{
			err::setFormatStringError ("invalid stack size '%s'", value.sz ());
			return false;
		}

		break;

	case CmdLineSwitch_SourceDir:
		m_cmdLine->m_sourceDirList.insertTail (value);
		break;

	case CmdLineSwitch_OutputDir:
		m_cmdLine->m_outputDir = value;
		break;

	case CmdLineSwitch_ImportDir:
		m_cmdLine->m_importDirList.insertTail (value);
		break;

	case CmdLineSwitch_IgnoreImport:
		m_cmdLine->m_ignoredImportList.insertTail (value);
		break;

	case CmdLineSwitch_DisableDoxyComment:
		DoxyCommentMap::Iterator it = DoxyCommentMap::find (value);
		if (it)
			m_cmdLine->m_doxyCommentFlags |= it->m_value;
		break;
	}

	return true;
}

bool
CmdLineParser::finalize ()
{
	static char jncSuffix [] = ".jnc";
	static char doxSuffix [] = ".dox";

	enum
	{
		SuffixLength = lengthof (jncSuffix)
	};

	bool includeDox = (m_cmdLine->m_flags & JncFlag_Documentation) != 0;

	sl::BoxIterator <sl::String> it = m_cmdLine->m_sourceDirList.getHead ();
	for (; it; it++)
	{
		sl::String dir = *it;
		if (dir.isEmpty ())
			continue;

		if (dir [dir.getLength () - 1])
			dir += '/';

		io::FileEnumerator fileEnum;
		bool result = fileEnum.openDir (dir);
		if (!result)
		{
			printf ("warning: %s\n", err::getLastErrorDescription ().sz ());
			continue;
		}

		while (fileEnum.hasNextFile ())
		{
			sl::String filePath = dir + fileEnum.getNextFileName ();
			if (io::isDir (filePath))
				continue;

			size_t length = filePath.getLength ();
			if (length < SuffixLength)
				continue;

			const char* suffix = filePath.sz () + length - SuffixLength;

			if (memcmp (suffix, jncSuffix, SuffixLength) == 0 ||
				includeDox && memcmp (suffix, doxSuffix, SuffixLength) == 0)
			{
				m_cmdLine->m_fileNameList.insertTail (filePath);
			}
		}
	}

	if (!(m_cmdLine->m_flags & (
		JncFlag_Help |
		JncFlag_Version |
		JncFlag_StdInSrc
		)) &&
		m_cmdLine->m_fileNameList.isEmpty ())
	{
		err::setFormatStringError ("missing input (file-name or --stdin)");
		return false;
	}

	if (m_cmdLine->m_flags & JncFlag_Run)
		m_cmdLine->m_flags |= JncFlag_Jit;

	return true;
}

//..............................................................................
