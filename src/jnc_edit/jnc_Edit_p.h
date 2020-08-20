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
	CodeAssist* m_codeAssist;
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
		bool isSync = false
		);

	void
	requestCodeAssist(
		CodeAssistKind kind,
		const QTextCursor& cursor,
		bool isSync = false
		)
	{
		requestCodeAssist(kind, cursor.position(), isSync);
	}

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
		const lex::LineCol& pos,
		ModuleItem* item
		);

	void
	createArgumentTip(
		const lex::LineCol& pos,
		FunctionType* type,
		size_t argumentIdx
		);

	void
	createAutoCompleteList(
		const lex::LineCol& pos,
		Namespace* nspace,
		uint_t flags
		);

	static
	lex::LineCol
	getLineColFromCursor(const QTextCursor& cursor);

	QRect
	getCursorRectFromLineCol(const lex::LineCol& pos);

	QPoint
	getToolTipPointFromLineCol(const lex::LineCol& pos);

	void
	highlightCurrentLine();

private slots:
	void updateLineNumberMargin(const QRect&, int);
	void onCursorPositionChanged();
	void onCompleterActivated(const QString &completion);
	void onCodeAssistReady();
	void onThreadFinished();
};

//..............................................................................

} // namespace jnc
