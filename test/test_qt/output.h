#ifndef _OUTPUT_H
#define _OUTPUT_H

#include "editor.h"

#define OutputBase Editor

class Output : public OutputBase
{
	Q_OBJECT

public:
	Output(QWidget *parent);

	QSize sizeHint() const { return QSize(300, 300); }

protected:
	void mouseDoubleClickEvent(QMouseEvent *e);

private:
	bool parseLine(
		const QTextCursor &cursor, 
		int &documentLine,
		int &documentCol,
		QString &filePath
		);
};

#endif