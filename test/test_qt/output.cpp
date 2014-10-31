#include "pch.h"
#include "output.h"
#include "mainwindow.h"
#include "mdichild.h"
#include "moc_output.cpp"

#define LINE_SELECTION_BACK		QColor(51, 51, 183)

Output::Output(QWidget *parent)
	: OutputBase(parent)
{
	setReadOnly(true);
	setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	setLineWrapMode (QPlainTextEdit::NoWrap);
}
 
void Output::mouseDoubleClickEvent(QMouseEvent *e)
{
	int documentLine;
	int documentCol;
	QString filePath;

	if(parseLine(textCursor(), documentLine, documentCol, filePath))
	{	
		MdiChild *child = GetMainWindow()->findMdiChild(filePath);
		if (child)
		{
			QColor foreColor(Qt::white);
			highlightSingleLine(textCursor(), LINE_SELECTION_BACK, &foreColor);

			child->selectLineCol(documentLine, documentCol);
			child->setFocus();

			e->ignore();
		}
	}
	else
	{
		e->accept();
		OutputBase::mouseDoubleClickEvent(e);
	}
}

bool Output::parseLine(
	const QTextCursor &cursor, 
	int &documentLine,
	int &documentCol,
	QString &filePath
	)
{
	QString text = cursor.block().text();

	QRegExp regExp("\\(([0-9]+),([0-9]+)\\):");
	int pos = regExp.indexIn(text);
	if(pos == -1)
		return false;

	filePath = text.left(pos);
	QString lineNumber = regExp.capturedTexts ().at (1);
	QString colNumber = regExp.capturedTexts ().at (2);
	
	documentLine = lineNumber.toInt() - 1;
	documentCol = colNumber.toInt() - 1;

	return true;
}