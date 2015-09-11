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