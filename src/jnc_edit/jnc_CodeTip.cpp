#include "pch.h"
#include "jnc_CodeTip.h"
#include "jnc_Highlighter.h"
#include "moc_jnc_CodeTip.cpp"

namespace jnc {

//..............................................................................

CodeTip::CodeTip(
	QWidget* parent,
	const EditTheme* theme
):
	QLabel(parent, Qt::ToolTip | Qt::BypassGraphicsProxyWidget) {
	m_theme = theme;
	m_functionTypeOverload = NULL;
	m_functionTypeOverloadIdx = 0;
	m_argumentIdx = 0;
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

void
CodeTip::nextFunctionTypeOverload() {
	ASSERT(isFunctionTypeOverload());

	size_t lastIdx = m_functionTypeOverload->getOverloadCount() - 1;
	if (m_functionTypeOverloadIdx < lastIdx)
		m_functionTypeOverloadIdx++;
	else
		m_functionTypeOverloadIdx = 0;

	setTipText(getArgumentTipText());
}

void
CodeTip::prevFunctionTypeOverload() {
	ASSERT(isFunctionTypeOverload());

	if (m_functionTypeOverloadIdx)
		m_functionTypeOverloadIdx--;
	else
		m_functionTypeOverloadIdx = m_functionTypeOverload->getOverloadCount() - 1;

	setTipText(getArgumentTipText());
}

void
CodeTip::showQuickInfoTip(
	const QPoint& pos,
	ModuleItem* item
) {
	m_functionTypeOverload = NULL;
	m_functionTypeOverloadIdx = 0;
	m_argumentIdx = 0;

	showText(pos, item->getSynopsis_v());
}

void
CodeTip::showArgumentTip(
	const QPoint& pos,
	FunctionTypeOverload* overload,
	size_t argumentIdx
) {
	m_functionTypeOverload = overload;
	m_argumentIdx = argumentIdx;

	size_t overloadCount = overload->getOverloadCount();
	if (!isVisible() || m_functionTypeOverloadIdx >= overloadCount)
		m_functionTypeOverloadIdx = 0;

	showText(pos, getArgumentTipText());
}

QString
CodeTip::getArgumentTipText() {
	ASSERT(m_functionTypeOverload);

	size_t overloadCount = m_functionTypeOverload->getOverloadCount();
	ASSERT(m_functionTypeOverloadIdx < overloadCount);

	FunctionType* type = m_functionTypeOverload->getOverload(m_functionTypeOverloadIdx);
	QString text = getArgumentTipText(type, m_argumentIdx);

	if (overloadCount > 1)
		text = QString("%1 of %2<hr>%3").
			arg(m_functionTypeOverloadIdx + 1).
			arg(overloadCount).
			arg(text);

	return text;
}

QString
CodeTip::getArgumentTipText(
	FunctionType* type,
	size_t argumentIdx
) {
	#define ML_ARG_INDENT "&nbsp;&nbsp;&nbsp;&nbsp;"

	bool isConst = false;
	FunctionType* shortType = type->getShortType();
	if (shortType != type) { // a member function
		isConst = type->getArgCount() && (type->getArg(0)->getType()->getFlags() & PtrTypeFlag_Const);
		type = shortType;

		if (argumentIdx)
			argumentIdx--;
	}

	Type* returnType = type->getReturnType();
	size_t argCount = type->getArgCount();
	size_t lastArgIdx = argCount - 1;

	bool isMl = argCount >= 2;

	QString text = highlightJancySource(returnType->getTypeString(), m_theme);
	text += isMl ? " (<br>" ML_ARG_INDENT : " (";

	for (size_t i = 0; i < argCount; i++) {
		FunctionArg* arg = type->getArg(i);
		Type* argType = arg->getType();

		if (i == argumentIdx)
			text += "<b>";

		text += highlightJancySource(argType->getTypeStringPrefix(), m_theme);
		text += ' ';
		text += arg->getDecl()->getName();
		text += highlightJancySource(argType->getTypeStringSuffix(), m_theme);

		if (arg->hasDefaultValue()) {
			text += " = ";
			text += highlightJancySource(arg->getDefaultValueString(), m_theme);
		}

		if (i == argumentIdx)
			text += "</b>";

		if (i != lastArgIdx)
			text += ",<br>" ML_ARG_INDENT;
	}

	if (type->getFlags() & FunctionTypeFlag_VarArg)
		text += isMl ? ",<br>" ML_ARG_INDENT "..." : ", ...";

	text += isMl ? "<br>)" : ")";

	if (isConst)
		text += highlightJancySource(" const", m_theme);

	return text;
}

void
CodeTip::showText(
	const QPoint& pos,
	const QString& text
) {
	if (text.isEmpty()) {
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
) {
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
CodeTip::paintEvent(QPaintEvent* e) {
	QStyleOptionFrame option;
	option.init(this);

	QStylePainter p(this);
	p.drawPrimitive(QStyle::PE_PanelTipLabel, option);
	p.end();

	QLabel::paintEvent(e);
}

void
CodeTip::resizeEvent(QResizeEvent* e) {
	QStyleOption option;
	option.init(this);

	QStyleHintReturnMask frameMask;
	if (style()->styleHint(QStyle::SH_ToolTip_Mask, &option, this, &frameMask))
		setMask(frameMask.region);

	QLabel::resizeEvent(e);
}

void
CodeTip::leaveEvent(QEvent* e) {
	QLabel::leaveEvent(e);
	onLeave();
}

int
CodeTip::getTipScreen(const QPoint& pos) {
	if (QApplication::desktop()->isVirtualDesktop())
		return QApplication::desktop()->screenNumber(pos);
	else
		return QApplication::desktop()->screenNumber(parentWidget());
}

void
CodeTip::setTipText(const QString& text) {
	setText(text);

	QFontMetrics fm(font());
	QSize extra(1, 0);

	// Make it look good with the default ToolTip font on Mac, which has a small descent.

	if (fm.descent() == 2 && fm.ascent() >= 11)
		++extra.rheight();

	resize(sizeHint() + extra);
}

void
CodeTip::placeTip(const QPoint& pos) {
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

void
CodeTip::onLeave() {
	QWidget* widget = qApp->widgetAt(QCursor::pos());
	QWidget* edit = parentWidget();

	while (widget) {
		if (widget == edit)
			return;

		widget = widget->parentWidget();
	}

	close();
}

//..............................................................................

} // namespace jnc
