#include "pch.h"
#include "editor.h"
#include "moc_editor.cpp"

Editor::Editor(QWidget *parent)
	: QPlainTextEdit(parent)
{
#if (_AXL_POSIX == AXL_POSIX_DARWIN)
	QFont font ("Menlo", 11);
#else
	QFont font ("Monospace", 10);
#endif
	font.setFixedPitch (true);
	font.setKerning (false);
	font.setStyleHint (
		QFont::Monospace,
		(QFont::StyleStrategy) (QFont::NoFontMerging | QFont::ForceIntegerMetrics)
		);
	
	setFont(font);
	setTabStopWidth(fontMetrics().width(' ') * 4);
}

void Editor::highlightSingleLine(const QTextCursor &cursor, const QColor &back,
	QColor *fore)
{
	QTextEdit::ExtraSelection selection;

	if(fore)
		selection.format.setForeground(*fore);

	selection.format.setBackground(back);
	selection.format.setProperty(QTextFormat::FullWidthSelection, true);

	selection.cursor = cursor;
	selection.cursor.clearSelection();

	QList<QTextEdit::ExtraSelection> extraSelections;
	extraSelections.append(selection);
	setExtraSelections(extraSelections);
}

void Editor::select(int startPos, int endPos)
{
	QTextCursor cursor = textCursor();
	cursor.setPosition(startPos);
	cursor.setPosition(endPos, QTextCursor::KeepAnchor);
	setTextCursor(cursor);
}

void Editor::selectLine(int line, bool isHighlighted)
{
	QTextCursor cursor = textCursor();

	cursor.setPosition(0);
	cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line);

	if (isHighlighted)
		cursor.select(QTextCursor::LineUnderCursor);

	setTextCursor(cursor);
}

void Editor::selectLineCol(int line, int col)
{
	QTextCursor cursor = textCursor();

	cursor.setPosition(0);
	cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line);
	cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, col);

	setTextCursor(cursor);
}

int Editor::posFromLine(int line)
{
	QTextCursor cursor = textCursor();

	cursor.setPosition(0);
	cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line);

	return cursor.block().position();
}