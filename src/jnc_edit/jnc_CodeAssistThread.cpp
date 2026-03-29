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
	CodeAssistThreadBase(parent) {
	rc::Ptr<AutoModule> module = AXL_RC_NEW(rc::Box<AutoModule>);
	m_module.attach(module->p(), module.getRefCount());
	module.detach();
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

	if (!m_extraSource.isEmpty()) {
		QByteArray src = m_extraSource.toUtf8();
		m_module->addSourceImport(NULL, src.constData(), src.length());
	}

	QByteArray source = m_source.toUtf8();
	size_t offset = m_source.left(m_position).toUtf8().count();

	m_module->generateCodeAssist(
		m_fileName.toUtf8().constData(),
		m_codeAssistKind,
		offset,
		source.constData(),
		source.length()
	);

	m_module->unloadDynamicLibs();
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
