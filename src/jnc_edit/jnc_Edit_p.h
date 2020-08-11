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

#include "jnc_Edit.h"
#include "jnc_LineNumberMargin.h"
#include "jnc_Highlighter.h"

namespace jnc {

//..............................................................................

class EditPrivate: public QObject
{
	Q_OBJECT
	Q_DECLARE_PUBLIC(Edit)

protected:
	enum Color
	{
		Color_CurrentLineBack = 0xe8eff8,
	};

	enum CodeAssistKind
	{
		CodeAssistKind_QuickInfoTip,
		CodeAssistKind_ArgumentTip,
		CodeAssistKind_AutoComplete,
		CodeAssistKind_AutoCompleteList,
		CodeAssistKind_GotoDefinition,
	};

protected:
    Edit* q_ptr;
	JancyHighlighter* m_syntaxHighlighter;
	LineNumberMargin* m_lineNumberMargin;
	Edit::CodeAssistTriggers m_codeAssistTriggers;
	QStringList m_importDirList;
	bool m_isCurrentLineHighlightingEnabled;

protected:
	EditPrivate();

	void
	setupEditor();

	void
	enableSyntaxHighlighting(bool isEnabled);

	void
	enableLineNumberMargin(bool isEnabled);

	void
	enableCurrentLineHighlighting(bool isEnabled);

	void
	updateLineNumberMarginGeometry();

	void
	requestCodeAssist(
		CodeAssistKind codeAssistKind,
		int position,
		bool isSync = false
		);

private slots:
	void updateLineNumberMargin(const QRect&, int);
	void highlightCurrentLine();
	void onCodeAssistReady();
};

//..............................................................................

} // namespace jnc
