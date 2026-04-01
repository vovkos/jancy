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

#include "jnc_Pch.h"

namespace jnc {

class EditBase;

//..............................................................................

class LineNumberMargin: public QWidget {
	Q_OBJECT

protected:
	int m_anchorPos;

public:
	LineNumberMargin(EditBase* edit);

	void
	updateFontMetrics();

protected:
	virtual void paintEvent(QPaintEvent* e);
	virtual void enterEvent(QEvent* e);

	virtual void mousePressEvent(QMouseEvent* e) {
		forwardMouseEvent(e);
	}

	virtual void mouseReleaseEvent(QMouseEvent* e) {
		forwardMouseEvent(e);
	}

	virtual void mouseMoveEvent(QMouseEvent* e) {
		forwardMouseEvent(e);
	}

	void forwardMouseEvent(QMouseEvent* e);
};

//..............................................................................

} // namespace jnc
