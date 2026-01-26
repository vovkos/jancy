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

bool
isCursorAtStartOfLine(const QTextCursor& cursor0);

bool
isCursorAtEndOfLineIgnoreSpace(const QTextCursor& cursor0);

int
getCursorStartOfLinePosition(const QTextCursor& cursor0);

int
getCursorEndOfLinePosition(const QTextCursor& cursor0);

QString
getCursorLinePrefix(const QTextCursor& cursor0);

QString
getCursorLineSuffix(const QTextCursor& cursor0);

bool
isCursorLineEmpty(const QTextCursor& cursor0);

bool
isCursorNextLineEmpty(const QTextCursor& cursor0);

QString
getCursorPrevWord(const QTextCursor& cursor0);

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
