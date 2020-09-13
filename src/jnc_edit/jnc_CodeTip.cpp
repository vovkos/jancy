#include "pch.h"
#include "jnc_CodeTip.h"
#include "moc_jnc_CodeTip.cpp"

namespace jnc {

//..............................................................................

CodeTip::CodeTip(QWidget* parent):
	QLabel(parent, Qt::ToolTip | Qt::BypassGraphicsProxyWidget)
{
	setForegroundRole(QPalette::ToolTipText);
	setBackgroundRole(QPalette::ToolTipBase);
	ensurePolished();
	setMargin(1 + style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, 0, this));
	setFrameStyle(QFrame::NoFrame);
	setAlignment(Qt::AlignLeft);
	setIndent(1);
	qApp->installEventFilter(this);
	setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, 0, this) / 255.0);
	setWordWrap(false);
	setMouseTracking(true);
}

void
CodeTip::showText(
	const QPoint& pos,
	const QString& text
	)
{
	if (text.isEmpty())
	{
		close();
		return;
	}

	if (isVisible() && pos == QLabel::pos() && text == QLabel::text()) // nothing changed
		return;

	setTipText(text);
	placeTip(pos);

	if (!isVisible())
		showNormal();
}

bool
CodeTip::eventFilter(
	QObject* o,
	QEvent* e
	)
{
	switch (e->type())
	{
	case QEvent::Leave:
		close();
		break;

	case QEvent::WindowActivate:
	case QEvent::WindowDeactivate:
	case QEvent::FocusIn:
	case QEvent::FocusOut:
	case QEvent::Close: // For QTBUG-55523 (QQC) specifically: Hide tooltip when windows are closed
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonRelease:
	case QEvent::MouseButtonDblClick:
	case QEvent::Wheel:
		close();
		break;

	default:
		break;
	}

	return false;
}

void
CodeTip::paintEvent(QPaintEvent* e)
{
	QStyleOptionFrame option;
	option.init(this);

	QStylePainter p(this);
	p.drawPrimitive(QStyle::PE_PanelTipLabel, option);
	p.end();

	QLabel::paintEvent(e);
}

void
CodeTip::resizeEvent(QResizeEvent* e)
{
	QStyleOption option;
	option.init(this);

	QStyleHintReturnMask frameMask;
	if (style()->styleHint(QStyle::SH_ToolTip_Mask, &option, this, &frameMask))
		setMask(frameMask.region);

	QLabel::resizeEvent(e);
}

int
CodeTip::getTipScreen(const QPoint& pos)
{
	if (QApplication::desktop()->isVirtualDesktop())
		return QApplication::desktop()->screenNumber(pos);
	else
		return QApplication::desktop()->screenNumber(parentWidget());
}

void
CodeTip::setTipText(const QString& text)
{
	setText(text);

	QFontMetrics fm(font());
	QSize extra(1, 0);

	// Make it look good with the default ToolTip font on Mac, which has a small descent.

	if (fm.descent() == 2 && fm.ascent() >= 11)
		++extra.rheight();

	resize(sizeHint() + extra);
}

void
CodeTip::placeTip(const QPoint& pos)
{
	QRect screen = QApplication::desktop()->screenGeometry(getTipScreen(pos));

	QPoint p = pos;

	if (p.x() + width() > screen.x() + screen.width())
		p.rx() -= 4 + width();

	if (p.y() + height() > screen.y() + screen.height())
		p.ry() -= 24 + height();

	if (p.y() < screen.y())
		p.setY(screen.y());

	if (p.x() + width() > screen.x() + screen.width())
		p.setX(screen.x() + screen.width() - width());

	if (p.x() < screen.x())
		p.setX(screen.x());

	if (p.y() + height() > screen.y() + screen.height())
		p.setY(screen.y() + screen.height() - height());

	move(p);
}

//..............................................................................

} // namespace jnc
