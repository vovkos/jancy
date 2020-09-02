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
#include "jnc_LineNumberMargin.h"
#include "jnc_Edit.h"
#include "moc_jnc_LineNumberMargin.cpp"

// #define _JNC_EDIT_LINE_NUMBER_MARGIN_BG 1

namespace jnc {

//..............................................................................

LineNumberMargin::LineNumberMargin(Edit* edit):
	QWidget(edit)
{
	updateFontMetrics();
}

void
LineNumberMargin::updateFontMetrics()
{
	int digitWidth = parentWidget()->fontMetrics().width('0');
	m_anchorPos = digitWidth * 4;
	setFixedWidth(digitWidth * 5);
}

void
LineNumberMargin::paintEvent(QPaintEvent* e)
{
	QPainter painter(this);
	QRectF paintRect = e->rect();

	Edit* edit = (Edit*)parentWidget();
	int lineHeight = edit->fontMetrics().height();

	QTextBlock block = edit->firstVisibleBlock();
	int blockNumber = block.blockNumber() + 1;
	qreal top = edit->blockBoundingGeometry(block).translated(edit->contentOffset()).top();
	qreal bottom = top + edit->blockBoundingRect(block).height();

	QFontInfo fontInfo(edit->font()); // no f-ing idea why simply setting font doesn't work
	painter.setFont(QFont(fontInfo.family(), fontInfo.pointSize()));
	painter.setPen(Color_Text);

#if (_JNC_EDIT_LINE_NUMBER_MARGIN_BG)
	painter.fillRect(paintRect, Color_Back);
#endif

	while (block.isValid() && top <= paintRect.bottom())
	{
		if (block.isVisible() && bottom >= paintRect.top())
			painter.drawText(
				0,
				(int)top,
				m_anchorPos,
				lineHeight,
				Qt::AlignRight,
				QString::number(blockNumber)
				);

		block = block.next();
		top = bottom;
		bottom = top + edit->blockBoundingRect(block).height();
		blockNumber++;
	}
}

//..............................................................................

} // namespace jnc
