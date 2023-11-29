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
	QThread(parent) {
	m_codeAssistKind = CodeAssistKind_Undefined;

	rc::Ptr<AutoModule> autoModule = AXL_RC_NEW(rc::Box<AutoModule>);
	m_module.attach(autoModule->p(), autoModule.getRefCount());
	autoModule.detach();
}

void
CodeAssistThread::request(
	CodeAssistKind kind,
	const rc::Ptr<Module>& cacheModule,
	int position,
	const QString& source
) {
	QByteArray sourceUtf8 = source.toUtf8();
	size_t offset = source.left(position).toUtf8().count();
	request(kind, cacheModule, offset, sl::StringRef(sourceUtf8.data(), sourceUtf8.size()));
}

void
CodeAssistThread::request(
	CodeAssistKind kind,
	const rc::Ptr<Module>& cacheModule,
	size_t offset,
	const sl::StringRef& source
) {
	m_codeAssistKind = kind;
	m_cacheModule = cacheModule;
	m_offset = offset;
	m_source = source;

	start();
}

void
CodeAssistThread::cancel() {
	m_module->cancelCodeAssist();
}

void
CodeAssistThread::run() {
	ModuleConfig config = g_defaultModuleConfig;
	config.m_compileFlags =
		ModuleCompileFlag_DisableCodeGen |
		ModuleCompileFlag_IgnoreOpaqueClassTypeInfo;

	m_module->initialize("code-assist-module", &config);
	m_module->setCompileErrorHandler(compileErrorHandler, this);
	m_module->addStaticLib(jnc::StdLib_getLib());
	m_module->addStaticLib(jnc::SysLib_getLib());

	QStringList::ConstIterator it = m_importDirList.begin();
	for (; it != m_importDirList.end(); it++)
		m_module->addImportDir((*it).toUtf8().constData());

	it = m_importList.begin();
	for (; it != m_importList.end(); it++)
		m_module->addImport((*it).toUtf8().constData());

	if (!m_extraSource.isEmpty())
		m_module->addSourceImport(NULL, m_extraSource.cp(), m_extraSource.getLength());

	m_module->generateCodeAssist(
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
) {
	TRACE("CodeAssistThread::compileErrorHandler: %s\n", jnc::getLastErrorDescription_v());
	return true;
}

//..............................................................................

} // namespace jnc
