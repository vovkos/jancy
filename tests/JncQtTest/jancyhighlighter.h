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
	int getRagelState(int blockState);
	
	void ragelExecPreEvent(int &ragelState);
	void ragelExecPostEvent(int ragelState);
};

#endif