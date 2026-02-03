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

#include "jnc_EditBase.h"

namespace jnc {

//..............................................................................

lex::LineCol
getCursorLineCol(const QTextCursor& cursor0);

bool
isCursorMultiLineSelection(const QTextCursor& cursor0);

inline
bool
isCursorAtStartOfLine(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	int position = cursor.position();
	cursor.movePosition(QTextCursor::StartOfLine);
	return cursor.position() == position;
}

inline
bool
isCursorAtEndOfLineIgnoreSpace(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	cursor.setPosition(cursor.position());
	cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
	return cursor.selectedText().trimmed().isEmpty();
}

inline
int
getCursorStartOfLinePosition(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	cursor.movePosition(QTextCursor::StartOfLine);
	return cursor.position();
}

inline
int
getCursorEndOfLinePosition(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	cursor.movePosition(QTextCursor::EndOfLine);
	return cursor.position();
}

inline
QString
getCursorLinePrefix(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	int position = cursor.position();
	cursor.setPosition(position);
	cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
	return cursor.selectedText();
}

inline
QString
getCursorLineSuffix(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	int position = cursor.position();
	cursor.setPosition(position);
	cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
	return cursor.selectedText();
}

inline
bool
isCursorLineEmpty(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	cursor.select(QTextCursor::LineUnderCursor);
	return cursor.selectedText().trimmed().isEmpty();
}

inline
bool
isCursorNextLineEmpty(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	cursor.movePosition(QTextCursor::Down);
	return isCursorLineEmpty(cursor);
}

inline
QString
getCursorPrevWord(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
	return cursor.selectedText();
}

QChar
getCursorPrevChar(const QTextCursor& cursor0);

QChar
getCursorNextChar(const QTextCursor& cursor0);

bool
isCursorOnIndent(const QTextCursor& cursor0);

bool
hasCursorHighlightColor(const QTextCursor& cursor);

void
moveCursorWithLimit(
	QTextCursor* cursor,
	const QTextCursor& limitCursor,
	QTextCursor::MoveOperation op,
	QTextCursor::MoveMode mode = QTextCursor::MoveAnchor,
	int n = 1
);

//..............................................................................

} // namespace jnc
