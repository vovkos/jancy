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

class CodeAssistThread: public QThread
{
	Q_OBJECT

protected:
	AutoModule m_module;
	sl::String m_source;
	size_t m_offset;
	CodeAssistKind m_codeAssistKind;

public:
	QStringList m_importDirList;

public:
	CodeAssistThread(QObject* parent = NULL);
	~CodeAssistThread();

	CodeAssist*
	getCodeAssist()
	{
		return m_module->getCodeAssist();
	}

	void
	request(
		CodeAssistKind kind,
		const QString& text,
		int position
		);

	void
	request(
		CodeAssistKind kind,
		const sl::StringRef& source,
		size_t offset
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
