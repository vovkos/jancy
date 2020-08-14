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
#include "jnc_CodeAssistThread.h"

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

protected:
    Edit* q_ptr;
	JancyHighlighter* m_syntaxHighlighter;
	LineNumberMargin* m_lineNumberMargin;
	Edit::CodeAssistTriggers m_codeAssistTriggers;
	QStringList m_importDirList;
	CodeAssistThread* m_thread;
	QCompleter* m_completer;
	QRect m_completerRect;
	bool m_isCurrentLineHighlightingEnabled;

protected:
	EditPrivate();

	void
	init();

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
		CodeAssistKind kind,
		int position,
		bool isSync = false
		);

	bool
	isCompleterVisible()
	{
		return m_completer && m_completer->popup()->isVisible();
	}

	void
	ensureCompleter();

	void
	applyCompleter();

	void
	updateCompleter(bool isForced = false);

	void
	hideCompleter()
	{
		ASSERT(m_completer);
		m_completer->popup()->hide();
	}

	void
	createQuickInfoTip(
		const lex::LineColOffset& pos,
		ModuleItem* item
		);

	void
	createArgumentTip(
		const lex::LineColOffset& pos,
		Function* function,
		size_t argumentIdx
		);

	void
	createAutoCompleteList(
		const lex::LineColOffset& pos,
		Namespace* nspace,
		uint_t flags
		);

private slots:
	void updateLineNumberMargin(const QRect&, int);
	void highlightCurrentLine();
	void onCompleterActivated(const QString &completion);
	void onCodeAssistReady();
	void onThreadFinished();
};

//..............................................................................

} // namespace jnc
