#include "pch.h"
#include "JncApp.h"
#include "JncLib.h"
#include "CmdLine.h"

//.............................................................................

bool
JncApp::initialize ()
{
	bool result;

	uint_t compileFlags = jnc::ct::ModuleCompileFlag_StdFlags;
	if (m_cmdLine->m_flags & JncFlag_DebugInfo)
		compileFlags |= jnc::ct::ModuleCompileFlag_DebugInfo;

	if (m_cmdLine->m_flags & JncFlag_McJit)
		compileFlags |= jnc::ct::ModuleCompileFlag_McJit;

	if (m_cmdLine->m_flags & JncFlag_SimpleGcSafePoint)
		compileFlags |= jnc::ct::ModuleCompileFlag_SimpleGcSafePoint;

	jnc::ext::ExtensionLibHost* libHost = jnc::ext::getStdExtensionLibHost ();

	result = 
		m_module.create ("jnc_module", compileFlags) &&
		m_module.m_extensionLibMgr.addStaticLib (jnc::ext::getStdLib (libHost)) &&
		m_module.m_extensionLibMgr.addStaticLib (jnc::ext::getSysLib (libHost)) &&
		m_module.m_extensionLibMgr.addStaticLib (sl::getSimpleSingleton <JncLib> ());

	if (!result)
		return false;

	m_module.m_importMgr.m_importDirList.copy (m_cmdLine->m_importDirList);
	m_module.m_importMgr.m_importDirList.insertTail (io::getExeDir ());

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

		result = m_module.parse (srcName, stdInBuffer, stdInBuffer.getCount ());
		if (!result)
			return false;
	}
	else
	{
		sl::BoxIterator <sl::String> fileNameIt = m_cmdLine->m_fileNameList.getHead ();
		ASSERT (fileNameIt);

		if (!m_cmdLine->m_srcNameOverride.isEmpty ())
		{
			result = m_module.parseFile (*fileNameIt, m_cmdLine->m_srcNameOverride);
			if (!result)
				return false;

			fileNameIt++;
		}

		for (; fileNameIt; fileNameIt++)
		{
			result = m_module.parseFile (*fileNameIt);
			if (!result)
				return false;
		}
	}

	return m_module.parseImports ();
}

bool
JncApp::runFunction (int* returnValue)
{
	bool result;

	jnc::ct::ModuleItem* functionItem = m_module.m_namespaceMgr.getGlobalNamespace ()->findItem (m_cmdLine->m_functionName);
	if (!functionItem || functionItem->getItemKind () != jnc::ct::ModuleItemKind_Function)
	{
		err::setFormatStringError ("'%s' is not found or not a function\n", m_cmdLine->m_functionName.cc ());
		return false;
	}

	jnc::ct::Function* function = (jnc::ct::Function*) functionItem;
	jnc::ct::FunctionType* functionType = function->getType ();
	jnc::ct::TypeKind returnTypeKind = functionType->getReturnType ()->getTypeKind ();
	size_t argCount = functionType->getArgArray ().getCount ();
	if (returnTypeKind != jnc::ct::TypeKind_Void && returnTypeKind != jnc::ct::TypeKind_Int || argCount)
	{
		err::setFormatStringError ("'%s' has invalid signature: %s\n", m_cmdLine->m_functionName.cc (), functionType->getTypeString ().cc ());
		return false;
	}

	m_runtime.setStackSizeLimit (m_cmdLine->m_stackSizeLimit);
	m_runtime.m_gcHeap.setSizeTriggers (
		m_cmdLine->m_gcAllocSizeTrigger,
		m_cmdLine->m_gcPeriodSizeTrigger
		);

	result = m_runtime.startup (&m_module);
	if (!result)
		return false;

	if (returnTypeKind == jnc::ct::TypeKind_Int)
	{
		jnc::rt::callFunction (&m_runtime, function, returnValue);
	}
	else
	{
		jnc::rt::callVoidFunction (&m_runtime, function);
		*returnValue = 0;
	}

	if (!result)
		return false;

	m_runtime.shutdown ();

	return true;
}

bool
JncApp::generateDocumentation ()
{
	jnc::ct::GlobalNamespace* nspace = m_module.m_namespaceMgr.getGlobalNamespace ();
	sl::String documentation = nspace->generateDocumentation (m_cmdLine->m_outputDir);
	if (documentation.isEmpty ())
	{
		err::setStringError ("module does not contain any documentable items");
		return false;
	}

	io::File indexFile;
	indexFile.open (m_cmdLine->m_outputDir + "/index.xml");
	indexFile.write (documentation.cc (), documentation.getLength ());
	return true;
}

//.............................................................................
