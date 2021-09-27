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

#pragma once

#include "monospaceplaintextedit.h"

class Output: public MonospacePlainTextEdit {
	Q_OBJECT

public:
	Output(QWidget *parent);

	QSize sizeHint() const {
		return QSize(300, 300);
	}

protected:
	void mouseDoubleClickEvent(QMouseEvent* e);

private:
	bool parseLine(
		QString* filePath,
		int* line,
		int* col,
		const QTextCursor& cursor
	);
};
