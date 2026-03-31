#pragma once

#include "jnc_CodeTipBase.h"

namespace jnc {

class EditTheme;

//..............................................................................

class CodeTip: public CodeTipBase {
	Q_OBJECT

protected:
	Template* m_template;
	FunctionTypeOverload* m_functionTypeOverload;

public:
	CodeTip(EditBase* parent);

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

	void
	showArgumentTip(
		const QPoint& pos,
		Template* templ,
		size_t argumentIdx
	);

protected:
	virtual
	QString
	createMultiTipText(size_t tipIdx) {
		ASSERT(codeAssistKind() == CodeAssistKind_ArgumentTip);
		return getArgumentTipText(tipIdx);
	}

protected:
	QString
	getArgumentTipText(size_t overloadIdx);

	QString
	getArgumentTipText(
		FunctionType* type,
		size_t argumentIdx
	);

	QString
	getArgumentTipText(
		Template* templ,
		size_t argumentIdx
	);
};

//..............................................................................

} // namespace jnc
