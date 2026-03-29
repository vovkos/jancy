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

#include "jnc_EditBase.h"

namespace jnc {

class EditPrivate;

//..............................................................................

class JNC_EDIT_EXPORT Edit: public EditBase {
	Q_OBJECT
	Q_DECLARE_PRIVATE(Edit)
	Q_DISABLE_COPY(Edit)

public:
	Edit(QWidget* parent = NULL);
	~Edit();

protected:
	virtual HighlighterBase* createSyntaxHighlighter();
	virtual CodeAssistThreadBase* createCodeAssistThread();
	virtual int calcActiveCodeAssistPosition();
	virtual void activateCompleter(const QModelIndex& index);

	virtual void autoIndent(
		QTextCursor* cursor,
		const QString& baseIndent,
		const QString& tailWord
	);

	virtual void showCodeAssist(CodeAssistThreadBase* thread);
	virtual void releaseCodeAssist();
	virtual void keyPressPrintChar(QKeyEvent* e);
};

//..............................................................................

} // namespace jnc
