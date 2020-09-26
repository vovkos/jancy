#pragma once

namespace jnc {

//..............................................................................

class CodeTip : public QLabel
{
	Q_OBJECT

public:
	CodeTip(QWidget* parent);

	void
	showText(
		const QPoint& pos,
		const QString& text
		);

protected:
	virtual
	bool
	eventFilter(
		QObject* o,
		QEvent* e
		);

	virtual
	void
	paintEvent(QPaintEvent* e);

	virtual
	void
	resizeEvent(QResizeEvent* e);

	virtual
	void
	leaveEvent(QEvent* e);

protected:
	int
	getTipScreen(const QPoint &pos);

	void
	setTipText(const QString &text);

	void
	placeTip(const QPoint &pos);

	void
	onLeave();
};

//..............................................................................

} // namespace jnc
