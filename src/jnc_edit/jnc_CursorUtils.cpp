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
#include "jnc_CursorUtils.h"

namespace jnc {

//..............................................................................

lex::LineCol
getCursorLineCol(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	cursor.movePosition(QTextCursor::StartOfLine);

	int line = 0;
	while (cursor.positionInBlock() > 0) {
		line++;
		cursor.movePosition(QTextCursor::Up);
	}

	QTextBlock block = cursor.block().previous();
	while (block.isValid()) {
		line += block.lineCount();
		block = block.previous();
	}

	return lex::LineCol(line, cursor0.columnNumber());
}

bool
isCursorMultiLineSelection(const QTextCursor& cursor0) {
	if (!cursor0.hasSelection())
		return false;

	QTextCursor cursor = cursor0;
	int start = cursor.anchor();
	int end = cursor.position();

	if (start > end) {
		int t = start;
		start = end;
		end = t;
	}

	cursor.setPosition(start);
	cursor.movePosition(QTextCursor::StartOfLine);
	cursor.movePosition(QTextCursor::Down);
	return cursor.position() <= end;
}

QChar
getCursorPrevChar(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	int position = cursor.position();
	cursor.setPosition(position);

	int sol = getCursorStartOfLinePosition(cursor);
	if (position <= sol)
		return QChar();

	cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
	QString selection = cursor.selectedText();
	return !selection.isEmpty() ? selection.at(0) : QChar();
}

QChar
getCursorNextChar(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	int position = cursor.position();
	cursor.setPosition(position);

	int eol = getCursorEndOfLinePosition(cursor);
	if (position >= eol)
		return QChar();

	cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
	QString selection = cursor.selectedText();
	return !selection.isEmpty() ? selection.at(0) : QChar();
}

bool
isCursorOnIndent(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	int position = cursor.position();
	cursor.movePosition(QTextCursor::StartOfLine);

	if (cursor.position() == position) // already was at start-of-line
		return getCursorNextChar(cursor).isSpace();

	cursor.setPosition(position, QTextCursor::KeepAnchor);
	QString selection = cursor.selectedText();
	return !selection.isEmpty() && selection.at(0).isSpace() && selection.trimmed().isEmpty();
}

bool
hasCursorHighlightColor(const QTextCursor& cursor) {
	if (cursor.atBlockEnd() && cursor.block().userState() != 0)
		return true;

#if (QT_VERSION >= 0x050600)
	QVector<QTextLayout::FormatRange> formats = cursor.block().layout()->formats();
#else
	QList<QTextLayout::FormatRange> formats = cursor.block().layout()->additionalFormats();
#endif

	// binary search

	int pos = cursor.positionInBlock();
	int left = 0;
	int right = formats.count();

	while (left < right) {
		int i = (left + right) / 2;

		QTextLayout::FormatRange range = formats[i];
		int end = range.start + range.length;

		if (pos < range.start)
			right = i;
		else if (pos > end)
			left = i + 1;
		else
			return true;
	}

	return false;
}

void
moveCursorWithLimit(
	QTextCursor* cursor,
	const QTextCursor& limitCursor,
	QTextCursor::MoveOperation op,
	QTextCursor::MoveMode mode,
	int n
) {
	int pos = cursor->position();
	cursor->movePosition(op, mode, n);

	if (cursor->position() == pos) // didn't change, use limit
		*cursor = limitCursor;
}

//..............................................................................

} // namespace jnc
