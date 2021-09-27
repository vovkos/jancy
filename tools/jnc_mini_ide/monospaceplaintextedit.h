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

class MonospacePlainTextEdit: public QPlainTextEdit {
	Q_OBJECT

public:
	MonospacePlainTextEdit(QWidget* parent = NULL);

	void highlightLine(
		const QTextCursor& cursor,
		const QTextCharFormat& format
	);

	void highlightLine(
		const QTextCursor& cursor,
		const QColor& backColor
	);

	void highlightLine(
		const QTextCursor& cursor,
		const QColor& backColor,
		const QColor& textColor
	);

	void appendString(const QString &string) {
		moveCursor(QTextCursor::End);
		insertPlainText(string);
	}
};
