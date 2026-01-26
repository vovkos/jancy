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

class LineNumberMargin;
class HighlighterBase;

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
	getCursorFromLineCol(
		int line,
		int col
	);

	QTextCursor
	getCursorFromOffset(size_t offset);

	void
	highlightCurrentLine();

	void
	indentSelection();

	void
	unindentSelection();

	void
	matchBraces();

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

private slots:
	void updateLineNumberMargin(const QRect&, int);
	void onCursorPositionChanged();
};

//..............................................................................

} // namespace jnc
