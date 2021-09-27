#pragma once

namespace jnc {

//..............................................................................

class CodeTip: public QLabel {
	Q_OBJECT

protected:
	FunctionTypeOverload* m_functionTypeOverload; // needed when we press up/down
	size_t m_functionTypeOverloadIdx;
	size_t m_argumentIdx;

public:
	CodeTip(QWidget* parent);

	bool
	isFunctionTypeOverload() {
		return m_functionTypeOverload && m_functionTypeOverload->getOverloadCount() > 1;
	}

	void
	nextFunctionTypeOverload();

	void
	prevFunctionTypeOverload();

	void
	showQuickInfoTip(
		const QPoint& pos,
		ModuleItem* item
	);

	void
	showArgumentTip(
		const QPoint& pos,
		FunctionTypeOverload* typeOverload,
		size_t argumentIdx
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
	static
	QString
	getArgumentTipText(
		FunctionType* type,
		size_t argumentIdx
	);

	QString
	getArgumentTipText();

	void
	showText(
		const QPoint& pos,
		const QString& text
	);

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
