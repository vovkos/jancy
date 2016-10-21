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

#ifndef LLVMIRHIGHLIGHTER_H
#define LLVMIRHIGHLIGHTER_H

#include "highlighter.h"

class LlvmIrHighlighter : public Highlighter
{
	Q_OBJECT

public:
	LlvmIrHighlighter(QTextDocument *parent = 0) : Highlighter(parent) { }

protected:
	void ragelInit();
	void ragelExec();
};

#endif
