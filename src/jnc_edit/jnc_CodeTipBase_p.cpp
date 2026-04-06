#include "pch.h"
#include "jnc_CodeTipBase_p.h"
#include "moc_jnc_CodeTipBase.cpp"
#include "moc_jnc_CodeTipBase_p.cpp"
#include "jnc_EditBase.h"
#include "jnc_Highlighter.h"

namespace jnc {

//..............................................................................

CodeTipBase::CodeTipBase(EditBase* parent):
	QLabel(parent, Qt::WindowFlags(Qt::ToolTip | Qt::BypassGraphicsProxyWidget)),
	d_ptr(new CodeTipBasePrivate) {

	Q_D(CodeTipBase);
	d->q_ptr = this;
	d->m_theme = parent->theme();

	setFont(parent->font());
	setForegroundRole(QPalette::ToolTipText);
	setBackgroundRole(QPalette::ToolTipBase);
	ensurePolished();
	setMargin(1 + style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, 0, this));
	setFrameStyle(QFrame::NoFrame);
	setAlignment(Qt::AlignLeft);
	setTextFormat(Qt::RichText);
	setIndent(1);
	qApp->installEventFilter(this);
	setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, 0, this) / 255.0);
	setWordWrap(false);
	setMouseTracking(true);
}

CodeTipBase::~CodeTipBase() {
}

JNC_EDIT_EXPORT
const EditTheme*
CodeTipBase::theme() const {
	Q_D(const CodeTipBase);
	return d->m_theme;
}

CodeAssistKind
CodeTipBase::codeAssistKind() const {
	Q_D(const CodeTipBase);
	return d->m_codeAssistKind;
}

bool
CodeTipBase::isMultiTip() const {
	Q_D(const CodeTipBase);
	return d->m_tipCount > 1;
}

size_t
CodeTipBase::tipCount() const {
	Q_D(const CodeTipBase);
	return d->m_tipCount;
}

size_t
CodeTipBase::tipIndex() const {
	Q_D(const CodeTipBase);
	return d->m_tipIdx;
}

size_t
CodeTipBase::argumentIndex() const {
	Q_D(const CodeTipBase);
	return d->m_argumentIdx;
}

bool
CodeTipBase::nextTip() {
	Q_D(CodeTipBase);
	if (d->m_tipIdx >= d->m_tipCount - 1)
		return false;

	QString text = createMultiTipText(++d->m_tipIdx);
	setTipText(text);
	return true;
}

bool
CodeTipBase::prevTip() {
	Q_D(CodeTipBase);

	if (!d->m_tipIdx)
		return false;

	QString text = createMultiTipText(--d->m_tipIdx);
	setTipText(text);
	return false;
}

void
CodeTipBase::showTip(
	CodeAssistKind codeAssistKind,
	const QPoint& pos,
	const QString& text,
	size_t tipCount,
	size_t tipIdx,
	size_t argumentIdx
) {
	Q_D(CodeTipBase);

	if (text.isEmpty()) {
		close();
		return;
	}

	if (isVisible() &&
		codeAssistKind == d->m_codeAssistKind &&
		tipCount == d->m_tipCount &&
		tipIdx == d->m_tipIdx &&
		argumentIdx == d->m_argumentIdx &&
		pos == QLabel::pos() &&
		text == QLabel::text()
	) // nothing changed
		return;

	d->m_codeAssistKind = codeAssistKind;
	d->m_tipCount = tipCount;
	d->m_tipIdx = tipIdx;
	d->m_argumentIdx = argumentIdx;

	setTipText(text);
	placeTip(pos);

	if (!isVisible())
		showNormal();
}

void
CodeTipBase::setTipText(const QString& text) {
	setText(text);

	QFontMetrics fm(font());
	QSize extra(1, 0);

	// Make it look good with the default ToolTip font on Mac, which has a small descent.

	if (fm.descent() == 2 && fm.ascent() >= 11)
		++extra.rheight();

	resize(sizeHint() + extra);
}

void
CodeTipBase::placeTip(const QPoint& pos) {
	int screenNumber = QApplication::desktop()->isVirtualDesktop() ?
		QApplication::desktop()->screenNumber(pos) :
		QApplication::desktop()->screenNumber(parentWidget());

	QRect screen = QApplication::desktop()->screenGeometry(screenNumber);

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

void
CodeTipBase::onLeave() {
	QWidget* widget = qApp->widgetAt(QCursor::pos());
	EditBase* edit = qobject_cast<EditBase*>(parentWidget());
	ASSERT(edit);

	while (widget) {
		if (widget == edit)
			return;

		widget = widget->parentWidget();
	}

	close();
}

bool
CodeTipBase::eventFilter(
	QObject* o,
	QEvent* e
) {
	Q_D(CodeTipBase);

	switch (e->type()) {
	case QEvent::Leave:
		onLeave();
		break;

	case QEvent::WindowActivate:
	case QEvent::WindowDeactivate:
	case QEvent::FocusIn:
	case QEvent::FocusOut:
	case QEvent::Close: // For QTBUG-55523 (QQC) specifically: Hide tooltip when windows are closed
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
CodeTipBase::closeEvent(QCloseEvent* e) {
	Q_D(CodeTipBase);

	QLabel::closeEvent(e);
	d->reset();
}

void
CodeTipBase::leaveEvent(QEvent* e) {
	Q_D(CodeTipBase);

	QLabel::leaveEvent(e);
	onLeave();
}

void
CodeTipBase::paintEvent(QPaintEvent* e) {
	QStyleOptionFrame option;
	option.init(this);

	QStylePainter p(this);
	p.drawPrimitive(QStyle::PE_PanelTipLabel, option);
	p.end();

	QLabel::paintEvent(e);
}

void
CodeTipBase::resizeEvent(QResizeEvent* e) {
	QStyleOption option;
	option.init(this);

	QStyleHintReturnMask frameMask;
	if (style()->styleHint(QStyle::SH_ToolTip_Mask, &option, this, &frameMask))
		setMask(frameMask.region);

	QLabel::resizeEvent(e);
}

//..............................................................................

} // namespace jnc
