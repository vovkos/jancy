#ifndef EDITOR_H
#define EDITOR_H

class Editor : public QPlainTextEdit
{
	Q_OBJECT

public:
	Editor(QWidget *parent = 0);

	void highlightSingleLine(const QTextCursor &cursor, const QColor &back,
		QColor *fore = 0);

	void select(int startPos, int endPos);
	void selectLine(int line, bool isHighlighted = false);	
	void selectLineCol(int line, int col);	

	int posFromLine(int line);

	void appendString (const QString &string)
	{
		moveCursor (QTextCursor::End);
		insertPlainText (string);
	}

	void appendText (const sl::StringRef& text)
	{
		appendString (QString::fromUtf8 (text.cp (), text.getLength ()));
	}

	void appendFormat_va (const char* format, va_list va)
	{
		QString string;
		string.vsprintf (format, va);
		appendString (string);
	}

	void appendFormat (const char* format, ...)
	{
		va_list va;
		va_start (va, format);
		appendFormat_va (format, va);
	}
};

#endif