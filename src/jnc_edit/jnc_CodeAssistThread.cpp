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
#include "jnc_CodeAssistThread.h"
#include "moc_jnc_CodeAssistThread.cpp"

namespace jnc {

//..............................................................................

CodeAssistThread::CodeAssistThread(QObject* parent):
	QThread(parent)
{
	printf("CodeAssistThread::CodeAssistThread(%p)\n", this);
	m_codeAssistKind = CodeAssistKind_Undefined;
}

CodeAssistThread::~CodeAssistThread()
{
	printf("CodeAssistThread::~CodeAssistThread(%p)\n", this);
	wait();
}

void
CodeAssistThread::request(
	CodeAssistKind kind,
	const ref::Ptr<Module>& cacheModule,
	int position,
	const QString& source
	)
{
	QByteArray sourceUtf8 = source.toUtf8();
	size_t offset = source.left(position).toUtf8().count();
	request(kind, cacheModule, offset, sl::StringRef(sourceUtf8.data(), sourceUtf8.size()));
}

void
CodeAssistThread::request(
	CodeAssistKind kind,
	const ref::Ptr<Module>& cacheModule,
	size_t offset,
	const sl::StringRef& source
	)
{
	m_codeAssistKind = kind;
	m_cacheModule = cacheModule;
	m_offset = offset;
	m_source = source;

	start();
}

void
CodeAssistThread::cancel()
{
	printf("CodeAssistThread::cancel -- not yet supported\n");
}

void
CodeAssistThread::run()
{
	m_module->initialize(
		"code-assist-module",
		ModuleCompileFlag_DisableCodeGen |
		ModuleCompileFlag_IgnoreOpaqueClassTypeInfo
		);

	m_module->setCompileErrorHandler(compileErrorHandler, this);
	m_module->addStaticLib(jnc::StdLib_getLib());
	m_module->addStaticLib(jnc::SysLib_getLib());

	QStringList::ConstIterator it = m_importDirList.begin();
	for (; it != m_importDirList.end(); it++)
		m_module->addImportDir((*it).toUtf8().constData());

	m_module->generateCodeAssist(
		m_codeAssistKind == CodeAssistKind_AutoComplete ?
			CodeAssistKind_AutoCompleteList : // one-item lists will be commited as auto-complete
			m_codeAssistKind,
		m_cacheModule,
		m_offset,
		m_source.cp(),
		m_source.getLength()
		);

	emit ready();
}

bool_t
CodeAssistThread::compileErrorHandler(
	void* context,
	jnc::ModuleCompileErrorKind errorKind
	)
{
	return true;
}

//..............................................................................

} // namespace jnc
