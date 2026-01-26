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

#include "pch.h"
#include "jnc_EditBase_p.h"
#include "moc_jnc_EditBase.cpp"
#include "moc_jnc_EditBase_p.cpp"
#include "jnc_LineNumberMargin.h"
#include "jnc_HighlighterBase.h"
#include "jnc_CursorUtils.h"

namespace jnc {

//..............................................................................

struct PairBrace {
	QChar m_c;
	bool m_isBackwardSearch;

	PairBrace() {
		m_isBackwardSearch = false;
	}

	PairBrace(
		QChar c,
		bool isBackwardSearch = false
	) {
		m_c = c;
		m_isBackwardSearch = isBackwardSearch;
	}

	operator bool () const {
		return !m_c.isNull();
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
bool
isLeftBrace(QChar c) {
	switch (c.unicode()) {
	case '(':
	case '[':
	case '{':
		return true;

	default:
		return false;
	}
}

inline
bool
isRightBrace(QChar c) {
	switch (c.unicode()) {
	case ')':
	case ']':
	case '}':
		return true;

	default:
		return false;
	}
}

QChar
getLeftBrace(QChar c) {
	switch (c.unicode()) {
	case ')':
		return '(';

	case ']':
		return '[';

	case '}':
		return '{';

	default:
		return c;
	}
}

QChar
getRightBrace(QChar c) {
	switch (c.unicode()) {
	case '(':
		return ')';

	case '[':
		return ']';

	case '{':
		return '}';

	default:
		return c;
	}
}

PairBrace
checkBraceMatch(QChar ch) {
	switch (ch.unicode()) {
	case '(':
		return PairBrace(')');

	case '[':
		return PairBrace(']');

	case '{':
		return PairBrace('}');

	case ')':
		return PairBrace('(', true);

	case ']':
		return PairBrace('[', true);

	case '}':
		return PairBrace('{', true);

	default:
		return PairBrace();
	}
}

bool
isBraceAutoComplete(QChar nextCh) {
	switch (nextCh.unicode()) {
	case 0:
	case ' ':
	case '\t':
	case ')':
	case ']':
	case '}':
	case ',':
	case ';':
		return true;

	default:
		return false;
	}
}

//..............................................................................

EditBase::EditBase(QWidget* parent):
	QPlainTextEdit(parent),
	d_ptr(new EditBasePrivate) {

	Q_D(EditBase);
	d->q_ptr = this;
	d->init();
}

EditBase::EditBase(
	QWidget* parent,
	EditBasePrivate* d
):
	QPlainTextEdit(parent),
	d_ptr(d) {}

EditBase::~EditBase() {}

bool
EditBase::isLineNumberMarginEnabled() {
	Q_D(EditBase);
	return d->m_lineNumberMargin != NULL;
}

void
EditBase::enableLineNumberMargin(bool isEnabled) {
	Q_D(EditBase);
	d->enableLineNumberMargin(isEnabled);
}

int EditBase::lineNumberMarginWidth() {
	Q_D(EditBase);
	return d->m_lineNumberMargin ? d->m_lineNumberMargin->width() : 0;
}

bool
EditBase::isCurrentLineHighlightingEnabled() {
	Q_D(EditBase);
	return d->m_isCurrentLineHighlightingEnabled;
}

void
EditBase::enableCurrentLineHighlighting(bool isEnabled) {
	Q_D(EditBase);
	d->enableCurrentLineHighlighting(isEnabled);
}

bool
EditBase::isSyntaxHighlightingEnabled() {
	Q_D(EditBase);
	return d->m_syntaxHighlighter != NULL;
}

void
EditBase::enableSyntaxHighlighting(bool isEnabled) {
	Q_D(EditBase);
	d->enableSyntaxHighlighting(isEnabled);
}

int
EditBase::tabWidth() {
	Q_D(EditBase);
	return d->m_tabWidth;
}

void
EditBase::setTabWidth(int width) {
	Q_D(EditBase);

	d->m_tabWidth = width;
	setTabStopWidth(fontMetrics().width(' ') * width);
}

const EditTheme*
EditBase::theme() {
	Q_D(EditBase);
	return &d->m_theme;
}

void
EditBase::setTheme(const EditTheme* theme) {
	Q_D(EditBase);

	d->m_theme = *theme;
	d->applyTheme();
	viewport()->update();
}

QString
EditBase::fileName() {
	Q_D(EditBase);
	return d->m_fileName;
}

void
EditBase::setFileName(const QString& fileName) {
	Q_D(EditBase);
	d->m_fileName = fileName;
}

void
EditBase::setTextCursorLineCol(
	int line,
	int col
) {
	Q_D(EditBase);
	setTextCursor(d->getCursorFromLineCol(line, col));
}

void
EditBase::highlightLineTemp(
	int line,
	const QColor& backColor,
	const QColor& textColor
) {
	Q_D(EditBase);

	QTextEdit::ExtraSelection selection;
	selection.cursor = d->getCursorFromLineCol(line, 0);
	selection.format.setProperty(QTextFormat::FullWidthSelection, true);

	if (backColor.isValid())
		selection.format.setBackground(backColor);

	if (textColor.isValid())
		selection.format.setForeground(textColor);

	d->m_highlighTable[EditBasePrivate::HighlightKind_Temp] = selection;
	d->updateExtraSelections();
}

void
EditBase::indentSelection() {
	Q_D(EditBase);
	d->indentSelection();
}

void
EditBase::unindentSelection() {
	Q_D(EditBase);
	d->unindentSelection();
}

void
EditBase::setReadOnly(bool isReadOnly) {
	Q_D(EditBase);
	QPlainTextEdit::setReadOnly(isReadOnly);
	d->applyPalette();
	d->updateExtraSelections();
}

void
EditBase::changeEvent(QEvent* e) {
	Q_D(EditBase);

	QPlainTextEdit::changeEvent(e);

	QEvent::Type type = e->type();
	switch (type) {
	case QEvent::FontChange:
		d->updateFont();
		break;

	case QEvent::EnabledChange:
		d->updateExtraSelections();
		break;

	// QEvent::ReadOnlyChange was introduced in QT-5.4; to support older QTs, don't rely on it.
	}
}

void
EditBase::resizeEvent(QResizeEvent *e) {
	Q_D(EditBase);

	QPlainTextEdit::resizeEvent(e);

	if (d->m_lineNumberMargin)
		d->updateLineNumberMarginGeometry();
}

void
EditBase::keyPressEvent(QKeyEvent* e) {
	Q_D(EditBase);

	int key = e->key();
	QString text = e->text();
	QChar ch = text.isEmpty() ? QChar() : text.at(0);

	switch (key) {
	case Qt::Key_Enter:
	case Qt::Key_Return:
		d->keyPressEnter(e);
		break;

	case Qt::Key_Tab:
		d->keyPressTab(e);
		break;

	case Qt::Key_Backtab:
		d->keyPressBacktab(e);
		break;

	case Qt::Key_Backspace:
		d->keyPressBackspace(e);
		break;

	case Qt::Key_Home:
		d->keyPressHome(e);
		break;

	default:
		if (ch.isPrint())
			d->keyPressPrintChar(e);
		else
			QPlainTextEdit::keyPressEvent(e);
	}
}

//..............................................................................

EditBasePrivate::EditBasePrivate() {
	q_ptr = NULL;
	m_syntaxHighlighter = NULL;
	m_lineNumberMargin = NULL;
	m_tabWidth = 4;
	m_isCurrentLineHighlightingEnabled = false;
	m_isExtraSelectionUpdateRequired = false;
	m_highlighTable[HighlightKind_CurrentLine].format.setProperty(QTextFormat::FullWidthSelection, true);
}

void
EditBasePrivate::init() {
	Q_Q(EditBase);

#if (_JNC_OS_DARWIN)
	QFont font("Menlo", 12);
#elif (_JNC_OS_WIN)
	QFont font("Consolas", 10);
#else
	QFont font("Monospace", 9);
#endif

	font.setFixedPitch(true);
	font.setKerning(false);
	font.setStyleHint(
		QFont::Monospace,
		(QFont::StyleStrategy)(QFont::NoFontMerging | QFont::ForceIntegerMetrics)
	);

	q->setFont(font);
	q->setWordWrapMode(QTextOption::NoWrap);
	q->setMouseTracking(true);

	enableSyntaxHighlighting(m_syntaxHighlighter != NULL);
	enableLineNumberMargin(true);
	enableCurrentLineHighlighting(true);

	QObject::connect(
		q, SIGNAL(cursorPositionChanged()),
		this, SLOT(onCursorPositionChanged())
	);

	applyTheme();
}

void
EditBasePrivate::applyTheme() {
	applyPalette();

	QColor color;

	color = m_theme.color(EditTheme::CurrentLineBack);
	if (color.isValid())
		m_highlighTable[HighlightKind_CurrentLine].format.setBackground(color);
	else
		m_highlighTable[HighlightKind_CurrentLine].format.clearBackground();

	color = m_theme.color(EditTheme::BraceMatchBack);
	if (color.isValid()) {
		m_highlighTable[HighlightKind_AnchorBrace].format.setBackground(color);
		m_highlighTable[HighlightKind_PairBrace].format.setBackground(color);
	} else {
		m_highlighTable[HighlightKind_AnchorBrace].format.clearBackground();
		m_highlighTable[HighlightKind_PairBrace].format.clearBackground();
	}

	color = m_theme.color(EditTheme::BraceMatchText);
	if (color.isValid()) {
		m_highlighTable[HighlightKind_AnchorBrace].format.setForeground(color);
		m_highlighTable[HighlightKind_PairBrace].format.setForeground(color);
	} else {
		m_highlighTable[HighlightKind_AnchorBrace].format.clearForeground();
		m_highlighTable[HighlightKind_PairBrace].format.clearForeground();
	}

	if (m_syntaxHighlighter)
		m_syntaxHighlighter->setTheme(&m_theme);

	if (m_lineNumberMargin)
		m_lineNumberMargin->update();

	updateExtraSelections();
}

void
EditBasePrivate::applyPalette() {
	Q_Q(EditBase);

	q->setPalette(
		q->isReadOnly() ?
			m_theme.readOnlyPalette() :
			m_theme.palette()
	);
}

void
EditBasePrivate::enableSyntaxHighlighting(bool isEnabled) {
	Q_Q(EditBase);

	if (isEnabled) {
		if (!m_syntaxHighlighter)
			m_syntaxHighlighter = q->createSyntaxHighlighter();
	} else if (m_syntaxHighlighter) {
		m_syntaxHighlighter->setDocument(NULL);
		delete m_syntaxHighlighter;
		m_syntaxHighlighter = NULL;
	}
}

void
EditBasePrivate::enableLineNumberMargin(bool isEnabled) {
	Q_Q(EditBase);

	if (isEnabled) {
		if (m_lineNumberMargin)
			return;

		m_lineNumberMargin = new LineNumberMargin(q);
		q->setViewportMargins(m_lineNumberMargin->width(), 0, 0, 0);
		updateLineNumberMarginGeometry();
		m_lineNumberMargin->show();

		QObject::connect(
			q, SIGNAL(updateRequest(const QRect&, int)),
			this, SLOT(updateLineNumberMargin(const QRect&, int))
		);
	} else {
		if (!m_lineNumberMargin)
			return;

		QObject::disconnect(
			q, SIGNAL(updateRequest(const QRect&, int)),
			this, SLOT(updateLineNumberMargin(const QRect&, int))
		);

		q->setViewportMargins(0, 0, 0, 0);
		delete m_lineNumberMargin;
		m_lineNumberMargin = NULL;
	}
}

void
EditBasePrivate::enableCurrentLineHighlighting(bool isEnabled) {
	Q_Q(EditBase);

	if (isEnabled == m_isCurrentLineHighlightingEnabled)
		return;

	if (isEnabled)
		highlightCurrentLine();
	else
		m_highlighTable[HighlightKind_CurrentLine].cursor = QTextCursor();

	m_isCurrentLineHighlightingEnabled = isEnabled;
	updateExtraSelections();
}

void
EditBasePrivate::updateExtraSelections() {
	Q_Q(EditBase);

	QList<QTextEdit::ExtraSelection> list;

	if (q->isEnabled() && !q->isReadOnly())
		for (size_t i = 0; i < countof(m_highlighTable); i++)
			if (!m_highlighTable[i].cursor.isNull())
				list.append(m_highlighTable[i]);

	q->setExtraSelections(list);
	m_isExtraSelectionUpdateRequired = false;
}

void
EditBasePrivate::updateFont() {
	Q_Q(EditBase);

	q->setTabStopWidth(q->fontMetrics().width(' ') * m_tabWidth);

	if (m_lineNumberMargin) {
		m_lineNumberMargin->updateFontMetrics();
		q->setViewportMargins(m_lineNumberMargin->width(), 0, 0, 0);
	}
}

void
EditBasePrivate::updateLineNumberMarginGeometry() {
	ASSERT(m_lineNumberMargin);

	Q_Q(EditBase);

	QRect rect = q->contentsRect();

	m_lineNumberMargin->setGeometry(
		rect.left(),
		rect.top(),
		m_lineNumberMargin->width(),
		rect.height()
	);
}

void
EditBasePrivate::updateLineNumberMargin(
	const QRect& rect,
	int dy
) {
	ASSERT(m_lineNumberMargin);

	if (dy)
		m_lineNumberMargin->scroll(0, dy);
	else
		m_lineNumberMargin->update(
			0,
			rect.y(),
			m_lineNumberMargin->width(),
			rect.height()
		);
}

void
EditBasePrivate::highlightCurrentLine() {
	Q_Q(EditBase);

	QTextCursor cursor = q->textCursor();
	cursor.clearSelection();
	m_highlighTable[HighlightKind_CurrentLine].cursor = cursor;

	m_isExtraSelectionUpdateRequired = true;
}

QTextCursor
EditBasePrivate::getCursorFromLineCol(
	int line,
	int col
) {
	Q_Q(EditBase);

	QTextCursor cursor = q->textCursor();
	cursor.setPosition(0);
	cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line);
	cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, col);
	return cursor;
}

QTextCursor
EditBasePrivate::getCursorFromOffset(size_t offset) {
	Q_Q(EditBase);

	QString prefix = q->toPlainText().toUtf8().left(offset);
	int position = prefix.length();
	QTextCursor cursor = q->textCursor();
	cursor.setPosition(position);
	return cursor;
}

void
EditBasePrivate::indentSelection() {
	Q_Q(EditBase);

	QTextCursor cursor = q->textCursor();
	int start = cursor.selectionStart();
	int end = cursor.selectionEnd();

	cursor.beginEditBlock();
	cursor.setPosition(start);
	cursor.movePosition(QTextCursor::StartOfLine);

	QTextCursor endCursor = cursor;
	endCursor.setPosition(end);

	for (; cursor < endCursor; moveCursorWithLimit(&cursor, endCursor, QTextCursor::Down))
		cursor.insertText(QChar('\t'));

	cursor.endEditBlock();
}

void
EditBasePrivate::unindentSelection() {
	Q_Q(EditBase);

	QTextCursor cursor = q->textCursor();

	int start = cursor.selectionStart();
	int end = isCursorMultiLineSelection(cursor) ?
		cursor.selectionEnd() :
		getCursorEndOfLinePosition(cursor);

	cursor.beginEditBlock();
	cursor.setPosition(start);
	cursor.movePosition(QTextCursor::StartOfLine);

	QTextCursor endCursor = cursor;
	endCursor.setPosition(end);

	for (; cursor < endCursor; moveCursorWithLimit(&cursor, endCursor, QTextCursor::Down)) {
		if (!isCursorOnIndent(cursor))
			continue;

		cursor.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);

		QString indent = cursor.selectedText();
		int length = indent.length();
		int delta = AXL_MIN(m_tabWidth, length);

		for (int i = 0; i < delta; i++)
			if (indent.at(i) == '\t') {
				delta = i + 1;
				break;
			}

		cursor.movePosition(QTextCursor::StartOfLine);
		cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, delta);
		cursor.removeSelectedText();
	}

	cursor.endEditBlock();
}

void
EditBasePrivate::keyPressHome(QKeyEvent* e) {
	Q_Q(EditBase);

	Qt::KeyboardModifiers modifiers = e->modifiers();
	if (modifiers & Qt::ControlModifier) {
		q->QPlainTextEdit::keyPressEvent(e);
		return;
	}

	QTextCursor::MoveMode moveMode = (modifiers & Qt::ShiftModifier) ?
		QTextCursor::KeepAnchor :
		QTextCursor::MoveAnchor;

	QTextCursor cursor = q->textCursor();
	bool isNextWord;

	if (isCursorAtStartOfLine(cursor))
		isNextWord = isCursorOnIndent(cursor);
	else {
		bool wasOnIndent = isCursorOnIndent(cursor);
		cursor.movePosition(QTextCursor::StartOfLine, moveMode);
		isNextWord = !wasOnIndent && isCursorOnIndent(cursor);
	}

	if (isNextWord)
		cursor.movePosition(QTextCursor::NextWord, moveMode);

	q->setTextCursor(cursor);
}

void
EditBasePrivate::keyPressTab(QKeyEvent* e) {
	Q_Q(EditBase);

	if (e->modifiers() & Qt::ShiftModifier) // unindent
		return keyPressBacktab(e);

	QTextCursor cursor = q->textCursor();
	if (isCursorMultiLineSelection(cursor))
		indentSelection();
	else
		q->QPlainTextEdit::keyPressEvent(e);
}

void
EditBasePrivate::keyPressBacktab(QKeyEvent* e) {
	Q_Q(EditBase);

	QTextCursor cursor = q->textCursor();
	if (isCursorMultiLineSelection(cursor) || isCursorOnIndent(cursor))
		unindentSelection();
}

void
EditBasePrivate::keyPressEnter(QKeyEvent* e) {
	Q_Q(EditBase);

	QTextCursor cursor = q->textCursor();
	if (cursor.hasSelection())
		cursor.setPosition(cursor.selectionStart());

	int position = cursor.position();
	cursor.movePosition(QTextCursor::StartOfLine);
	if (!isCursorOnIndent(cursor)) {
		q->QPlainTextEdit::keyPressEvent(e);
		return;
	}

	bool isSol = cursor.position() == position;
	cursor.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
	QString indent = cursor.selectedText();

	cursor = q->textCursor();
	cursor.beginEditBlock();
	cursor.insertText(QChar('\n'));

	if (isCursorOnIndent(cursor))
		cursor.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor); // select new "random" indent

	cursor.insertText(indent); // insert proper indent

	if (isSol) {
		cursor.movePosition(QTextCursor::StartOfLine);
		q->setTextCursor(cursor);
	}

	cursor.movePosition(QTextCursor::Up);
	cursor.movePosition(QTextCursor::EndOfLine);
	cursor.movePosition(QTextCursor::PreviousWord);
	cursor.movePosition(QTextCursor::EndOfWord);
	cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
	cursor.removeSelectedText();
	cursor.endEditBlock();
}

void
EditBasePrivate::keyPressBackspace(QKeyEvent* e) {
	Q_Q(EditBase);

	QTextCursor cursor = q->textCursor();
	if (cursor.hasSelection()) {
		q->QPlainTextEdit::keyPressEvent(e);
		return;
	}

	QChar prev = getCursorPrevChar(cursor);
	if (isLeftBrace(prev)) {
		QChar next = getCursorNextChar(cursor);
		if (next == getRightBrace(prev)) {
			cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
			cursor.removeSelectedText();
		}
	}

	q->QPlainTextEdit::keyPressEvent(e);
}

void
EditBasePrivate::keyPressPrintChar(QKeyEvent* e) {
	Q_Q(EditBase);

	QString text = e->text();
	QChar ch = text.isEmpty() ? QChar() : text.at(0);
	int c = ch.toLatin1();

	QTextCursor cursor = q->textCursor();

	switch (c) {
	case ')':
	case ']':
	case '}':
		if (cursor.hasSelection() || getCursorNextChar(cursor) != c || hasCursorHighlightColor(cursor)) {
			q->QPlainTextEdit::keyPressEvent(e);
			break;
		}

		cursor.movePosition(QTextCursor::NextCharacter);
		q->setTextCursor(cursor);
		break;

	case '(':
	case '[':
	case '{':
		q->QPlainTextEdit::keyPressEvent(e);

		if (hasCursorHighlightColor(cursor))
			break;

		if (isBraceAutoComplete(getCursorNextChar(cursor))) {
			cursor = q->textCursor();
			cursor.insertText(getRightBrace(c));
			cursor.movePosition(QTextCursor::PreviousCharacter);
			q->setTextCursor(cursor);
		}

		break;

	default:
		q->QPlainTextEdit::keyPressEvent(e);
	}
}

void
EditBasePrivate::matchBraces() {
	Q_Q(EditBase);

	if (!m_highlighTable[HighlightKind_AnchorBrace].cursor.isNull()) { // clear first
		m_highlighTable[HighlightKind_AnchorBrace].cursor = QTextCursor();
		m_highlighTable[HighlightKind_PairBrace].cursor = QTextCursor();
		m_isExtraSelectionUpdateRequired = true;
	}

	QTextCursor cursor = q->textCursor();
	if (cursor.hasSelection())
		return;

	QChar brace = getCursorNextChar(cursor);
	PairBrace pair = checkBraceMatch(brace);
	if (!pair) {
		brace = getCursorPrevChar(cursor);
		pair = checkBraceMatch(brace);
		if (!pair)
			return;

		cursor.movePosition(QTextCursor::PreviousCharacter);
	}

	if (hasCursorHighlightColor(cursor))
		return;

	QString text = q->toPlainText();

	int pos = cursor.position();
	int matchPos = -1;

	if (pair.m_isBackwardSearch)
		for (int i = pos - 1, level = 1; i >= 0; i--) {
			QChar c = text.at(i);

			if (c == brace) {
				cursor.setPosition(i);
				if (!hasCursorHighlightColor(cursor))
					level++;
			} else if (c == pair.m_c) {
				cursor.setPosition(i);
				if (!hasCursorHighlightColor(cursor) && !--level) {
					matchPos = i;
					break;
				}
			}
		} else
		for (int i = pos + 1, level = 1, length = text.length(); i < length; i++) {
			QChar c = text.at(i);
			if (c == brace) {
				cursor.setPosition(i);
				if (!hasCursorHighlightColor(cursor))
					level++;
			} else if (c == pair.m_c) {
				cursor.setPosition(i);
				if (!hasCursorHighlightColor(cursor) && !--level) {
					matchPos = i;
					break;
				}
			}
		}

	if (matchPos == -1)
		return;

	cursor.setPosition(pos);
	m_highlighTable[HighlightKind_AnchorBrace].cursor = cursor;
	m_highlighTable[HighlightKind_AnchorBrace].cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);

	cursor.setPosition(matchPos);
	m_highlighTable[HighlightKind_PairBrace].cursor = cursor;
	m_highlighTable[HighlightKind_PairBrace].cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);

	m_isExtraSelectionUpdateRequired = true;
}

void
EditBasePrivate::onCursorPositionChanged() {
	if (m_isCurrentLineHighlightingEnabled)
		highlightCurrentLine();

	if (!m_highlighTable[HighlightKind_Temp].cursor.isNull()) {
		m_highlighTable[HighlightKind_Temp].cursor = QTextCursor();
		m_isExtraSelectionUpdateRequired = true;
	}

	matchBraces();

	if (m_isExtraSelectionUpdateRequired)
		updateExtraSelections();
}

//..............................................................................

} // namespace jnc
