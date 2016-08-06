#include "pch.h"
#include "JncApp.h"
#include "CmdLine.h"

//.............................................................................

JNC_DEFINE_LIB (JncLib)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE (JncLib)
JNC_END_LIB_SOURCE_FILE_TABLE ()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE (JncLib)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE ()

JNC_BEGIN_LIB_FUNCTION_MAP (JncLib)
	JNC_MAP_FUNCTION ("printf",  &printf)
JNC_END_LIB_FUNCTION_MAP ()

//.............................................................................

bool
JncApp::initialize ()
{
	uint_t compileFlags = jnc::ModuleCompileFlag_StdFlags;
	if (m_cmdLine->m_flags & JncFlag_DebugInfo)
		compileFlags |= jnc::ModuleCompileFlag_DebugInfo;

	if (m_cmdLine->m_flags & JncFlag_McJit)
		compileFlags |= jnc::ModuleCompileFlag_McJit;

	if (m_cmdLine->m_flags & JncFlag_SimpleGcSafePoint)
		compileFlags |= jnc::ModuleCompileFlag_SimpleGcSafePoint;

	bool result = m_module->initialize ("jnc_module", compileFlags);
	if (!result)
		return false;

	m_module->addStaticLib (jnc::StdLib_getLib ());
	m_module->addStaticLib (jnc::SysLib_getLib ());
	m_module->addStaticLib (JncLib_getLib ());

	sl::BoxIterator <sl::String> it = m_cmdLine->m_importDirList.getHead ();
	for (; it; it++)
		m_module->addImportDir (*it);

	m_module->addImportDir (io::getExeDir ());

	return true;
}

bool
JncApp::parse ()
{
	bool result;

	if (m_cmdLine->m_flags & JncFlag_StdInSrc)
	{
#if (_AXL_ENV == AXL_ENV_WIN)
		int stdInFile = _fileno (stdin);
#endif
		sl::Array <char> stdInBuffer;

		for (;;)
		{
			char buffer [1024];
#if (_AXL_ENV == AXL_ENV_WIN)
			int size = _read (stdInFile, buffer, sizeof (buffer));
#else
			int size = read (STDIN_FILENO, buffer, sizeof (buffer));
#endif
			if (size <= 0)
				break;

			stdInBuffer.append (buffer, size);
		}

		const char* srcName = !m_cmdLine->m_srcNameOverride.isEmpty () ?
			m_cmdLine->m_srcNameOverride.cc () :
			"stdin";

		result = m_module->parse (srcName, stdInBuffer, stdInBuffer.getCount ());
		if (!result)
			return false;
	}
	else
	{
		sl::BoxIterator <sl::String> fileNameIt = m_cmdLine->m_fileNameList.getHead ();
		ASSERT (fileNameIt);

		for (; fileNameIt; fileNameIt++)
		{
			result = m_module->parseFile (*fileNameIt);
			if (!result)
				return false;
		}
	}

	return m_module->parseImports ();
}

bool
JncApp::runFunction (int* returnValue)
{
	bool result;

	jnc::ModuleItem* functionItem = m_module->findItem (m_cmdLine->m_functionName);
	if (!functionItem || functionItem->getItemKind () != jnc::ModuleItemKind_Function)
	{
		err::setFormatStringError ("'%s' is not found or not a function\n", m_cmdLine->m_functionName.cc ());
		return false;
	}

	jnc::Function* function = (jnc::Function*) functionItem;
	jnc::FunctionType* functionType = function->getType ();
	jnc::TypeKind returnTypeKind = functionType->getReturnType ()->getTypeKind ();
	size_t argCount = functionType->getArgCount ();
	if (returnTypeKind != jnc::TypeKind_Void && returnTypeKind != jnc::TypeKind_Int || argCount)
	{
		err::setFormatStringError ("'%s' has invalid signature: %s\n", m_cmdLine->m_functionName.cc (), functionType->getTypeString ());
		return false;
	}

	m_runtime->setStackSizeLimit (m_cmdLine->m_stackSizeLimit);
	m_runtime->getGcHeap ()->setSizeTriggers (&m_cmdLine->m_gcSizeTriggers);

	result = m_runtime->startup (m_module);
	if (!result)
		return false;

	if (returnTypeKind == jnc::TypeKind_Int)
	{
		jnc::callFunction (m_runtime, function, returnValue);
	}
	else
	{
		jnc::callVoidFunction (m_runtime, function);
		*returnValue = 0;
	}

	if (!result)
		return false;

	m_runtime->shutdown ();

	return true;
}

bool
JncApp::generateDocumentation ()
{
	return m_module->generateDocumentation (m_cmdLine->m_outputDir);
}

//.............................................................................
