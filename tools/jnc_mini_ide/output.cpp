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
#include "output.h"
#include "mainwindow.h"
#include "mdichild.h"
#include "moc_output.cpp"

Output::Output(QWidget* parent):
	MonospacePlainTextEdit(parent) {
	setReadOnly(true);
	setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	setLineWrapMode(QPlainTextEdit::NoWrap);
}

void Output::mouseDoubleClickEvent(QMouseEvent* e) {
	QString filePath;
	int line;
	int col;

	if (parseLine(&filePath, &line, &col, textCursor())) {
		MdiChild* child = getMainWindow()->findMdiChild(filePath);
		if (child) {
			const jnc::EditTheme* theme = child->theme();

			highlightLine(textCursor(), jnc::g_defaultLightTheme.color(jnc::EditTheme::ErrorBack), jnc::g_defaultLightTheme.color(jnc::EditTheme::ErrorText));

			child->setTextCursorLineCol(line, col);
			child->highlightLineTemp(line, theme->color(jnc::EditTheme::ErrorBack), theme->color(jnc::EditTheme::ErrorText));
			child->setFocus();

			e->ignore();
		}
	} else {
		e->accept();
		QPlainTextEdit::mouseDoubleClickEvent(e);
	}
}

bool Output::parseLine(
	QString* filePath,
	int* line,
	int* col,
	const QTextCursor& cursor
) {
	QString text = cursor.block().text();

	QRegExp regexp("\\(([0-9]+),([0-9]+)\\):");
	int pos = regexp.indexIn(text);
	if(pos == -1)
		return false;

	*filePath = text.left(pos);
	QString lineString = regexp.capturedTexts().at(1);
	QString colString = regexp.capturedTexts().at(2);

	*line = lineString.toInt() - 1;
	*col = colString.toInt() - 1;
	return true;
}
