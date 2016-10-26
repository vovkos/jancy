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
#include "highlighter.h"
#include "moc_highlighter.cpp"

Highlighter::Highlighter(QTextDocument *parent)
	: QSyntaxHighlighter(parent)
{

}

void Highlighter::highlightBlock(const QString &text)
{
	cs = 0;
	act = 0;
	ts = 0;
	te = 0;

	ragelInit();

	QByteArray byteArray = text.toLocal8Bit();

	p = const_cast<char *>(byteArray.data());
	const_p = p;

	pe = p + text.length();
	eof = pe;

	ragelExecPreEvent(cs);

	ragelExec();

	ragelExecPostEvent(cs);
}

void Highlighter::colorize(char *tokenStart, char *tokenEnd, Qt::GlobalColor color, bool bold)
{
	int index = tokenStart - const_p;
	int length = tokenEnd - tokenStart;

	QTextCharFormat format;
	format.setForeground(color);

	if (bold)
		format.setFontWeight(QFont::Bold);

	setFormat(index, length, format);
}
