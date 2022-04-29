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

#pragma once

namespace jnc {

//..............................................................................

class CodeAssistThread: public QThread {
	Q_OBJECT

protected:
	rc::Ptr<Module> m_module;
	rc::Ptr<Module> m_cacheModule;
	CodeAssistKind m_codeAssistKind;
	sl::String m_source;
	size_t m_offset;

public:
	QStringList m_importDirList;
	QStringList m_importList;
	sl::String m_extraSource;

public:
	CodeAssistThread(QObject* parent = NULL);

	~CodeAssistThread() {
		wait();
	}

	const rc::Ptr<Module>&
	getModule() {
		return m_module;
	}

	CodeAssistKind
	getCodeAssistKind() {
		return m_codeAssistKind;
	}

	void
	request(
		CodeAssistKind kind,
		const rc::Ptr<Module>& cacheModule,
		int position,
		const QString& source
	);

	void
	request(
		CodeAssistKind kind,
		const rc::Ptr<Module>& cacheModule,
		size_t offset,
		const sl::StringRef& source
	);

	void
	cancel();

protected:
	virtual
	void
	run();

	static
	bool_t
	compileErrorHandler(
		void* context,
		jnc::ModuleCompileErrorKind errorKind
	);

signals:
	void error(const lex::LineCol& lineCol, size_t length);
	void ready();
};

//..............................................................................

} // namespace jnc
