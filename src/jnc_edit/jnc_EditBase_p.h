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
#include "jnc_CodeTipBase.h"

namespace jnc {

class LineNumberMargin;
class HighlighterBase;
class CodeAssistThreadBase;

//..............................................................................

class EditBasePrivate: public QObject {
	Q_OBJECT
	Q_DECLARE_PUBLIC(EditBase)

protected:
	enum HighlightKind {
		HighlightKind_CurrentLine,
		HighlightKind_AnchorBrace,
		HighlightKind_PairBrace,
		HighlightKind_Temp,
		HighlightKind__Count,
	};

	enum Limit {
		Limit_MaxVisibleItemCount = 16,
		Limit_MaxNameWidth        = 256,
		Limit_MaxSynopsisWidth    = 512,
	};

protected:
	EditBase* q_ptr;
	EditTheme m_theme;
	HighlighterBase* m_syntaxHighlighter;
	LineNumberMargin* m_lineNumberMargin;
	QString m_fileName;
	QTextEdit::ExtraSelection m_highlighTable[HighlightKind__Count];
	int m_tabWidth;
	bool m_isExtraSelectionUpdateRequired;
	bool m_isCurrentLineHighlightingEnabled;
	bool m_isTabsToSpacesEnabled;
	EditBase::CodeAssistTriggers m_codeAssistTriggers;
	QStringList m_importDirList;
	QStringList m_importList;
	QString m_extraSource;
	CodeAssistThreadBase* m_codeAssistThread;
	CodeAssistKind m_pendingCodeAssistKind;
	int m_pendingCodeAssistPosition;
	CodeAssistKind m_activeCodeAssistKind;
	int m_activeCodeAssistPosition;
	CodeTipBase* m_codeTip;
	QCompleter* m_completer;
	QRect m_completerRect;
	QIcon m_iconTable[EditBase::CompleterIconCount];
	QBasicTimer m_codeAssistTimer;

protected:
	EditBasePrivate();

	void
	init();

	void
	setReadOnly(bool isReadOnly);

	void
	applyTheme();

	void
	applyPalette();

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

	QTextCursor
	cursorFromLineCol(
		int line,
		int col
	);

	QTextCursor
	cursorFromOffset(size_t offset);

	void
	highlightCurrentLine();

	void
	indentSelection();

	void
	unindentSelection();

	void
	matchBraces();

	void
	requestCodeAssist(
		int delay,
		CodeAssistKind kind
	);

	void
	requestCodeAssist(
		int delay,
		CodeAssistKind kind,
		const QTextCursor& cursor
	) {
		requestCodeAssist(delay, kind, cursor.position());
	}

	void
	requestCodeAssist(
		int delay,
		CodeAssistKind kind,
		int position
	);

	void
	requestQuickInfoTip(
		int delay,
		const QPoint& pos
	);

	void
	startCodeAssistThread(
		CodeAssistKind kind,
		int position
	);

	void
	hideCodeAssist();

	CodeTipBase*
	ensureCodeTip();

	bool
	isCompleterVisible() {
		return m_completer && m_completer->popup()->isVisible();
	}

	bool
	isCodeTipVisible() {
		return m_codeTip && m_codeTip->isVisible();
	}

	QCompleter*
	ensureCompleter();

	void
	updateCompleter(bool isForced = false);

	void
	applyCompleter();

	QTextCursor
	activeCodeAssistCursor();

	QRect
	activeCodeAssistCursorRect();

	QPoint
	activeCodeTipPoint(bool isBelowCurrentCursor = false);

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
	keyPressControlSpace(QKeyEvent* e);

	virtual
	void
	timerEvent(QTimerEvent* e);

private slots:
	void updateLineNumberMargin(const QRect&, int);
	void onCursorPositionChanged();
	void onCompleterActivated(const QModelIndex& index);
	void onCodeAssistThreadReady();
	void onCodeAssistThreadFinished();
};

//..............................................................................

class CompleterItemDelegate: public QStyledItemDelegate {
protected:
	const EditTheme* m_theme;

public:
	CompleterItemDelegate(
		QObject* parent,
		const EditTheme* theme
	):
		QStyledItemDelegate(parent) {
		m_theme = theme;
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

} // namespace jnc
