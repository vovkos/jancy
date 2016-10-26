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

#ifndef JANCYHIGHLIGHTER_H
#define JANCYHIGHLIGHTER_H

#include "highlighter.h"

class JancyHighlighter : public Highlighter
{
	Q_OBJECT

public:
	JancyHighlighter(QTextDocument *parent = 0) : Highlighter(parent) { }

protected:
	void ragelInit();
	void ragelExec();

	void ragelExecPreEvent(int &ragelState);
	void ragelExecPostEvent(int ragelState);
};

#endif
