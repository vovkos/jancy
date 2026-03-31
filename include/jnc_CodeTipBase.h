#pragma once

#include "jnc_EditPch.h"

namespace jnc {

class CodeTipBasePrivate;
class EditTheme;
class EditBase;

//..............................................................................

class JNC_EDIT_EXPORT CodeTipBase: public QLabel {
	Q_OBJECT
	Q_DECLARE_PRIVATE(CodeTipBase)
	Q_DISABLE_COPY(CodeTipBase)

public:
	CodeTipBase(EditBase* parent);
	~CodeTipBase(); // otherwise QScopedPointer<CodeTipBasePrivate> would produce incomplete type error

	const EditTheme* theme() const;
	bool isMultiTip() const;
	size_t tipCount() const;
	size_t tipIndex() const;
	size_t argumentIndex() const;
	bool nextTip();
	bool prevTip();

protected:
	CodeAssistKind codeAssistKind() const;

	void showTip(
		CodeAssistKind codeAssistKind,
		const QPoint& pos,
		const QString& text,
		size_t tipCount,
		size_t tipIdx,
		size_t argumentIdx
	);

	void setTipText(const QString &text);
	void placeTip(const QPoint &pos);
	void onLeave();

protected:
	// overridables

	virtual QString createMultiTipText(size_t tipIdx) = 0;

	virtual bool eventFilter(
		QObject* o,
		QEvent* e
	);

	virtual void closeEvent(QCloseEvent* e);
	virtual void leaveEvent(QEvent* e);
	virtual void paintEvent(QPaintEvent* e);
	virtual void resizeEvent(QResizeEvent* e);

protected:
	QScopedPointer<CodeTipBasePrivate> d_ptr;
};

//..............................................................................

} // namespace jnc
