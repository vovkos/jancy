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

class CodeTip;

//..............................................................................

class EditPrivate: public QObject
{
	Q_OBJECT
	Q_DECLARE_PUBLIC(Edit)

public:
	enum Timeout
	{
		Timeout_QuickInfo = 500,
	};

	enum Color
	{
		Color_SelectionBack         = 0x99c9ef,
		Color_SelectionBackInactive = 0xe0e0e0,
		Color_CurrentLineBack       = 0xe8eff8,
		Color_SynopsisColumnText    = 0x808080,
		Color_BraceMatch            = 0xfff080,
	};

	enum Column
	{
		Column_Name     = 0,
		Column_Synopsis = 1,
	};

	enum Limit
	{
		Limit_MaxVisibleItemCount = 16,
		Limit_MaxNameWidth        = 256,
		Limit_MaxSynopsisWidth    = 512,
	};

protected:
	enum Role
	{
		Role_CaseInsensitiveSort = Qt::UserRole + 1,
		Role_ModuleItem,
	};

	enum Icon
	{
		Icon_Object,
		Icon_Namespace,
		Icon_Event,
		Icon_Function,
		Icon_Property,
		Icon_Variable,
		Icon_Field,
		Icon_Const,
		Icon_Type,
		Icon_Typedef,
		Icon__Count,
	};

	enum HighlightKind
	{
		HighlightKind_CurrentLine,
		HighlightKind_AnchorBrace,
		HighlightKind_PairBrace,
		HighlightKind_Temp,
		HighlightKind__Count,
	};

protected:
    Edit* q_ptr;
	JancyHighlighter* m_syntaxHighlighter;
	LineNumberMargin* m_lineNumberMargin;
	int m_tabWidth;
	Edit::CodeAssistTriggers m_codeAssistTriggers;
	QStringList m_importDirList;
	QStringList m_importList;
	CodeAssistThread* m_thread;
	ref::Ptr<Module> m_lastCodeAssistModule;
	CodeAssistKind m_lastCodeAssistKind;
	size_t m_lastCodeAssistOffset;
	int m_lastCodeAssistPosition;
	int m_pendingCodeAssistPosition;
	CodeTip* m_codeTip;
	QCompleter* m_completer;
	QRect m_completerRect;
	QIcon m_iconTable[Icon__Count];
	QBasicTimer m_quickInfoTipTimer;
	QFileIconProvider m_fileIconProvider;
	QTextEdit::ExtraSelection m_highlighTable[HighlightKind__Count];
	bool m_isExtraSelectionUpdateRequired;
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
	updateFont();

	void
	updateLineNumberMarginGeometry();

	void
	updateExtraSelections();

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

	void
	requestQuickInfoTip(const QPoint& pos);

	void
	hideCodeAssist();

	void
	ensureCodeTip();

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
	createQuickInfoTip(ModuleItem* item);

	void
	createArgumentTip(
		FunctionType* type,
		size_t argumentIdx
		);

	void
	createAutoCompleteList(
		Namespace* nspace,
		uint_t flags
		);

	void
	createImportAutoCompleteList(Module* module);

	void
	addAutoCompleteNamespace(
		QStandardItemModel* model,
		Namespace* nspace
		);

	void
	addFile(
		QStandardItemModel* model,
		const QString& fileName
		);

	size_t
	getItemIconIdx(ModuleItem* item);

	QTextCursor
	getCursorFromLineCol(
		int line,
		int col
		);

	QTextCursor
	getCursorFromOffset(size_t offset);

	void
	highlightCurrentLine();

	QTextCursor
	getLastCodeAssistCursor();

	QRect
	getLastCodeAssistCursorRect();

	int
	getLastCodeAssistPosition()
	{
		return m_lastCodeAssistPosition != -1 ? m_lastCodeAssistPosition : calcLastCodeAssistPosition();
	}

	int
	calcLastCodeAssistPosition();

	QPoint
	getLastCodeTipPoint(bool isBelowCurrentCursor = false);

	Function*
	getAutoCompleteDeclFunction(const QModelIndex& index);

	void
	indentSelection();

	void
	unindentSelection();

	void
	matchBraces();

	void
	keyPressControlSpace(QKeyEvent* e);

	void
	keyPressHome(QKeyEvent* e);

	void
	keyPressTab(QKeyEvent* e);

	void
	keyPressBacktab(QKeyEvent* e);

	void
	keyPressEnter(QKeyEvent* e);

	void
	keyPressBackspace(QKeyEvent* e);

	void
	keyPressPrintChar(QKeyEvent* e);

	virtual
	void
	timerEvent(QTimerEvent* e);

private slots:
	void updateLineNumberMargin(const QRect&, int);
	void onCursorPositionChanged();
	void onCompleterActivated(const QModelIndex& index);
	void onCodeAssistReady();
	void onThreadFinished();
};

//..............................................................................

class CompleterItemDelegate: public QStyledItemDelegate
{
public:
	CompleterItemDelegate(QObject* parent = NULL):
		QStyledItemDelegate(parent)
	{
	}

	virtual
	void
	paint(
		QPainter* painter,
		const QStyleOptionViewItem& option0,
		const QModelIndex& index
		) const;
};

//..............................................................................

struct PairBrace
{
	QChar m_c;
	bool m_isBackwardSearch;

	PairBrace()
	{
		m_isBackwardSearch = false;
	}

	PairBrace(
		QChar c,
		bool isBackwardSearch = false
		)
	{
		m_c = c;
		m_isBackwardSearch = isBackwardSearch;
	}

	operator bool () const
	{
		return !m_c.isNull();
	}
};

//..............................................................................

} // namespace jnc
