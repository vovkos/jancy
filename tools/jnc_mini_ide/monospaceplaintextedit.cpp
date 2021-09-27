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
#include "monospaceplaintextedit.h"
#include "moc_monospaceplaintextedit.cpp"

MonospacePlainTextEdit::MonospacePlainTextEdit(QWidget* parent):
	QPlainTextEdit(parent) {
#if (_JNC_OS_DARWIN)
	QFont font("Menlo", 11);
#else
	QFont font("Monospace", 9);
#endif
	font.setFixedPitch(true);
	font.setKerning(false);
	font.setStyleHint(
		QFont::Monospace,
		(QFont::StyleStrategy)(QFont::NoFontMerging | QFont::ForceIntegerMetrics)
	);

	setFont(font);
	setTabStopWidth(fontMetrics().width(' ') * 4);
	setReadOnly(false);
}

void MonospacePlainTextEdit::highlightLine(
	const QTextCursor& cursor,
	const QTextCharFormat& format
) {
	QTextEdit::ExtraSelection selection;
	selection.format = format;
	selection.cursor = cursor;
	selection.cursor.clearSelection();

	QList<QTextEdit::ExtraSelection> extraSelections;
	extraSelections.append(selection);
	setExtraSelections(extraSelections);
}

void MonospacePlainTextEdit::highlightLine(
	const QTextCursor& cursor,
	const QColor& backColor
) {
	QTextCharFormat format;
	format.setBackground(backColor);
	format.setProperty(QTextFormat::FullWidthSelection, true);
	highlightLine(cursor, format);
}

void MonospacePlainTextEdit::highlightLine(
	const QTextCursor& cursor,
	const QColor& backColor,
	const QColor& textColor
) {
	QTextCharFormat format;
	format.setBackground(backColor);
	format.setForeground(textColor);
	format.setProperty(QTextFormat::FullWidthSelection, true);
	highlightLine(cursor, format);
}
