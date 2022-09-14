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

namespace jnc {

class Edit;

//..............................................................................

class LineNumberMargin: public QWidget {
	Q_OBJECT

protected:
	int m_anchorPos;

public:
	LineNumberMargin(Edit* edit);

	void
	updateFontMetrics();

protected:
	virtual
	void
	paintEvent(QPaintEvent* e);
};

//..............................................................................

} // namespace jnc
