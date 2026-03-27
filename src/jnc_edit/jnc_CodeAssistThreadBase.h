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

class CodeAssistThreadBase: public QThread {
	Q_OBJECT

protected:
	sl::String m_fileName;
	sl::String m_source;
	CodeAssistKind m_codeAssistKind;
	size_t m_offset;

public:
	QStringList m_importDirList;
	QStringList m_importList;
	sl::String m_extraSource;

public:
	CodeAssistThreadBase(QObject* parent = NULL);

	~CodeAssistThreadBase() {
		wait();
	}

	CodeAssistKind
	getCodeAssistKind() {
		return m_codeAssistKind;
	}

	void
	request(
		const QString& fileName,
		CodeAssistKind kind,
		int position,
		const QString& source
	);

	void
	request(
		const sl::StringRef& fileName,
		CodeAssistKind kind,
		size_t offset,
		const sl::StringRef& source
	);

	virtual
	void
	cancel() = 0;

protected:
	virtual
	void
	run() = 0;

signals:
	void error(const lex::LineCol& lineCol, size_t length);
	void ready();
};

//..............................................................................

} // namespace jnc
