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

#include "jnc_EditPch.h"

namespace jnc {

//..............................................................................

class JNC_EDIT_EXPORT CodeAssistThreadBase: public QThread {
	Q_OBJECT

protected:
	QString m_fileName;
	QString m_source;
	CodeAssistKind m_codeAssistKind;
	int m_position;

public:
	QStringList m_importDirList;
	QStringList m_importList;
	QString m_extraSource;

public:
	CodeAssistThreadBase(QObject* parent = NULL);
	~CodeAssistThreadBase();

	CodeAssistKind codeAssistKind() {
		return m_codeAssistKind;
	}

	int position() {
		return m_position;
	}

	void request(
		const QString& fileName,
		CodeAssistKind kind,
		int position,
		const QString& source
	);

	virtual void cancel() {}

protected:
	virtual void run() = 0;

signals:
	void error(int line, int col, size_t length);
	void ready();
};

//..............................................................................

inline
void CodeAssistThreadBase::request(
	const QString& fileName,
	CodeAssistKind kind,
	int position,
	const QString& source
) {
	m_fileName = fileName;
	m_codeAssistKind = kind;
	m_position = position;
	m_source = source;
	start();
}

//..............................................................................

} // namespace jnc
