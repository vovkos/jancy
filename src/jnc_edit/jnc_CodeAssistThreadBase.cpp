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
#include "jnc_CodeAssistThreadBase.h"
#include "moc_jnc_CodeAssistThreadBase.cpp"

namespace jnc {

//..............................................................................

CodeAssistThreadBase::CodeAssistThreadBase(QObject* parent):
	QThread(parent) {
	m_codeAssistKind = CodeAssistKind_Undefined;
	m_offset = 0;
}

void
CodeAssistThreadBase::request(
	const QString& fileName,
	CodeAssistKind kind,
	int position,
	const QString& source
) {
	QByteArray fileNameUtf8 = fileName.toUtf8();
	QByteArray sourceUtf8 = source.toUtf8();
	size_t offset = source.left(position).toUtf8().count();

	request(
		sl::StringRef(fileNameUtf8.data(), fileNameUtf8.size()),
		kind,
		offset,
		sl::StringRef(sourceUtf8.data(), sourceUtf8.size())
	);
}

void
CodeAssistThreadBase::request(
	const sl::StringRef& fileName,
	CodeAssistKind kind,
	size_t offset,
	const sl::StringRef& source
) {
	m_fileName = fileName;
	m_codeAssistKind = kind;
	m_offset = offset;
	m_source = source;

	start();
}

//..............................................................................

} // namespace jnc
