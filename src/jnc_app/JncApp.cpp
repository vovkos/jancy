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
#include "JncApp.h"
#include "CmdLine.h"

//..............................................................................

// {39B98823-0E9F-4C72-BC77-A254E855925F}
JNC_DEFINE_GUID	(
	g_jncLibGuid,
	0x39b98823, 0xe9f, 0x4c72, 0xbc, 0x77, 0xa2, 0x54, 0xe8, 0x55, 0x92, 0x5f
);

JNC_DEFINE_LIB(
	JncLib,
	g_jncLibGuid,
	"JncLib",
	"Jancy CLI executable extension library"
)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE(JncLib)
JNC_END_LIB_SOURCE_FILE_TABLE()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE(JncLib)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE()

JNC_BEGIN_LIB_FUNCTION_MAP(JncLib)
JNC_END_LIB_FUNCTION_MAP()

//..............................................................................

JncApp::JncApp(CmdLine* cmdLine) {
	m_cmdLine = cmdLine;
	m_module->initialize("jnc_module", cmdLine->m_compileFlags);
	m_module->setCompileErrorHandler(compileErrorHandler, this);

	if (!(cmdLine->m_compileFlags & jnc::ModuleCompileFlag_StdLibDoc)) {
		// m_module->addStaticLib(jnc::StdLib_getLib());
		// m_module->addStaticLib(jnc::SysLib_getLib());
		// m_module->addStaticLib(JncLib_getLib());
	}

	sl::BoxIterator<sl::String> it = cmdLine->m_importDirList.getHead();
	for (; it; it++)
		m_module->addImportDir(*it);

	it = cmdLine->m_ignoredImportList.getHead();
	for (; it; it++)
		m_module->addIgnoredImport(*it);

	m_module->addImportDir(io::getExeDir());

	sl::BoxIterator<sl::String> requireIt = cmdLine->m_requireList.getHead();
	for (; requireIt; requireIt++)
		m_module->require(jnc::ModuleItemKind_Undefined, *requireIt);

	if (cmdLine->m_flags & JncFlag_Run)
		m_module->require(jnc::ModuleItemKind_Function, cmdLine->m_functionName);
}

bool_t
JncApp::compileErrorHandler(
	void* context,
	jnc::ModuleCompileErrorKind errorKind
) {
	JncApp* self = (JncApp*)context;
	fprintf(stderr, "%s\n", jnc::getLastError()->getDescription().sz());
	return true;
}

bool
JncApp::parse() {
	bool result;

	if (m_cmdLine->m_flags & JncFlag_StdInSrc) {
#if (_JNC_OS_WIN)
		int stdInFile = _fileno(stdin);
#endif
		for (;;) {
			char buffer[1024];
#if (_JNC_OS_WIN)
			int size = _read(stdInFile, buffer, sizeof(buffer));
#else
			int size = read(STDIN_FILENO, buffer, sizeof(buffer));
#endif
			if (size <= 0)
				break;

			m_stdInBuffer.append(buffer, size);
		}

		const char* srcName = !m_cmdLine->m_srcNameOverride.isEmpty() ?
			m_cmdLine->m_srcNameOverride.sz() :
			"stdin";

		result = m_module->parse(srcName, m_stdInBuffer, m_stdInBuffer.getCount());
		if (!result)
			return false;
	} else {
		sl::BoxIterator<sl::String> fileNameIt = m_cmdLine->m_fileNameList.getHead();
		ASSERT(fileNameIt);

		for (; fileNameIt; fileNameIt++) {
			result = m_module->parseFile(*fileNameIt);
			if (!result)
				return false;
		}
	}

	return m_module->parseImports();
}

bool
JncApp::runFunction(int* returnValue) {
	bool result;

	jnc::FindModuleItemResult findResult = m_module->getGlobalNamespace()->getNamespace()->findItem(m_cmdLine->m_functionName);
	ASSERT(findResult.m_item && findResult.m_item->getItemKind() == jnc::ModuleItemKind_Function);

	jnc::Function* function = (jnc::Function*)findResult.m_item;
	jnc::FunctionType* functionType = function->getType();
	jnc::TypeKind returnTypeKind = functionType->getReturnType()->getTypeKind();
	size_t argCount = functionType->getArgCount();
	if (returnTypeKind != jnc::TypeKind_Void && returnTypeKind != jnc::TypeKind_Int || argCount) {
		err::setFormatStringError("'%s' has invalid signature: %s\n", m_cmdLine->m_functionName.sz(), functionType->getTypeString());
		return false;
	}

	m_runtime->getGcHeap()->setSizeTriggers(&m_cmdLine->m_gcSizeTriggers);

	result = m_runtime->startup(m_module);
	if (!result)
		return false;

	if (returnTypeKind == jnc::TypeKind_Int) {
		result = jnc::callFunction(m_runtime, function, returnValue);
	} else {
		result = jnc::callVoidFunction(m_runtime, function);
		*returnValue = 0;
	}

	if (!result)
		return false;

	m_runtime->shutdown();

	return true;
}

bool
JncApp::generateDocumentation() {
	return m_module->generateDocumentation(m_cmdLine->m_outputDir);
}

//..............................................................................
