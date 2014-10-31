#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

class Highlighter : public QSyntaxHighlighter
{
	Q_OBJECT

public:
	Highlighter(QTextDocument *parent = 0);

protected:
	void highlightBlock(const QString &text);
	void colorize(char *tokenStart, char *tokenEnd, Qt::GlobalColor color, bool bold = false);

	virtual void ragelExecPreEvent(int &ragelState) { }
	virtual void ragelExecPostEvent(int ragelState) { }

	virtual void ragelInit() = 0;
	virtual void ragelExec() = 0;
	virtual int getRagelState(int blockState) = 0;

	int cs;
	int act;
	char *ts;
	char *te;
	char *p;
	char *const_p;
	char *pe;
	char *eof;
};

#endif