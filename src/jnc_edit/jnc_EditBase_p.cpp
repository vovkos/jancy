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
#include "jnc_CodeAssistThreadBase.h"

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
EditBase::setReadOnly(bool isReadOnly) {
	Q_D(EditBase);
	QPlainTextEdit::setReadOnly(isReadOnly);
	d->applyPalette();
	d->updateExtraSelections();
}

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

bool
EditBase::isTabsToSpacesEnabled() {
	Q_D(EditBase);
	return d->m_isTabsToSpacesEnabled;
}

void
EditBase::enableTabsToSpaces(bool isEnabled) {
	Q_D(EditBase);
	d->m_isTabsToSpacesEnabled = true;
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

EditBase::CodeAssistTriggers
EditBase::codeAssistTriggers() {
	Q_D(EditBase);
	return d->m_codeAssistTriggers;
}

void
EditBase::setCodeAssistTriggers(CodeAssistTriggers triggers) {
	Q_D(EditBase);
	d->m_codeAssistTriggers = triggers;
}

QStringList
EditBase::importDirList() {
	Q_D(EditBase);
	return d->m_importDirList;
}

void
EditBase::setImportDirList(const QStringList& dirList) {
	Q_D(EditBase);
	d->m_importDirList = dirList;
}

QStringList
EditBase::importList() {
	Q_D(EditBase);
	return d->m_importList;
}

void
EditBase::setImportList(const QStringList& importList) {
	Q_D(EditBase);
	d->m_importList = importList;
}

QString
EditBase::extraSource() {
	Q_D(EditBase);
	return d->m_extraSource;
}

void
EditBase::setExtraSource(const QString& source) {
	Q_D(EditBase);
	d->m_extraSource = source;
}

void
EditBase::quickInfoTip() {
	Q_D(EditBase);
	d->requestCodeAssist(0, CodeAssistKind_QuickInfoTip);
}

void
EditBase::argumentTip() {
	Q_D(EditBase);
	d->requestCodeAssist(0, CodeAssistKind_ArgumentTip);
}

void
EditBase::autoComplete() {
	Q_D(EditBase);
	d->requestCodeAssist(0, CodeAssistKind_AutoComplete);
}

void
EditBase::gotoDefinition() {
	Q_D(EditBase);
	d->requestCodeAssist(0, CodeAssistKind_GotoDefinition);
}

void
EditBase::setActiveCodeAssist(
	CodeAssistKind codeAssistKind,
	int position
) {
	Q_D(EditBase);
	d->m_activeCodeAssistKind = codeAssistKind;
	d->m_activeCodeAssistPosition = position;
}

CodeAssistKind
EditBase::activeCodeAssistKind() {
	Q_D(EditBase);
	return d->m_activeCodeAssistKind;
}

int
EditBase::activeCodeAssistPosition() {
	Q_D(EditBase);
	return d->m_activeCodeAssistPosition;
}

QTextCursor
EditBase::activeCodeAssistCursor() {
	Q_D(EditBase);
	return d->activeCodeAssistCursor();
}

QRect
EditBase::activeCodeAssistCursorRect() {
	Q_D(EditBase);
	return d->activeCodeAssistCursorRect();
}

QPoint
EditBase::activeCodeTipPoint(bool isBelowCurrentCursor) {
	Q_D(EditBase);
	return d->activeCodeTipPoint(isBelowCurrentCursor);
}

QCompleter*
EditBase::ensureCompleter() {
	Q_D(EditBase);
	return d->ensureCompleter();
}

void
EditBase::updateCompleter(bool isForced) {
	Q_D(EditBase);
	d->updateCompleter(isForced);
}

QIcon
EditBase::completerIcon(CompleterIcon icon) {
	Q_D(EditBase);
	return (size_t)icon < countof(d->m_iconTable) ? d->m_iconTable[icon] : QIcon();
}

CodeTipBase*
EditBase::ensureCodeTip() {
	Q_D(EditBase);
	return d->ensureCodeTip();
}

QTextCursor
EditBase::cursorFromOffset(size_t offset) {
	Q_D(EditBase);
	return d->cursorFromOffset(offset);
}

QTextCursor
EditBase::cursorFromLineCol(
	int line,
	int col
) {
	Q_D(EditBase);
	return d->cursorFromLineCol(line, col);
}

void
EditBase::setTextCursorLineCol(
	int line,
	int col
) {
	Q_D(EditBase);
	setTextCursor(d->cursorFromLineCol(line, col));
}

void
EditBase::highlightLineTemp(
	int line,
	const QColor& backColor,
	const QColor& textColor
) {
	Q_D(EditBase);

	QTextEdit::ExtraSelection selection;
	selection.cursor = d->cursorFromLineCol(line, 0);
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
EditBase::hideCodeAssist() {
	Q_D(EditBase);
	return d->hideCodeAssist();
}

void
EditBase::addFile(
	QStandardItemModel* model,
	const QString& fileName
) {
	Q_D(EditBase);

	QStandardItem* qtItem = new QStandardItem;
	qtItem->setText(fileName);
	qtItem->setData(fileName.toLower(), EditBase::CaseInsensitiveSortRole);
	qtItem->setIcon(d->m_iconTable[FileIcon]);

	model->appendRow(qtItem);
}

void
EditBase::autoIndent(
	QTextCursor* cursor,
	const QString& baseIndent,
	const QString& tailWord
) {
	cursor->insertText(QChar('\n'));
	cursor->insertText(baseIndent);
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

	if (!d->isCompleterVisible())
		switch (key) {
		case Qt::Key_Escape:
			d->hideCodeAssist();
			QPlainTextEdit::keyPressEvent(e);
			break;

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

		case Qt::Key_Up:
		case Qt::Key_Down:
			if (!d->isCodeTipVisible() || !d->m_codeTip->isMultiTip())
				QPlainTextEdit::keyPressEvent(e);
			else if (key == Qt::Key_Up)
				d->m_codeTip->prevTip();
			else
				d->m_codeTip->nextTip();

			break;

		case Qt::Key_Space:
			if (e->modifiers() & QT_CONTROL_MODIFIER) {
				d->keyPressControlSpace(e);
				break;
			}

			// fall through

		default:
			if (ch.isPrint())
				keyPressPrintChar(e);
			else
				QPlainTextEdit::keyPressEvent(e);
		}
	else
		switch (key) {
		case Qt::Key_Escape:
		case Qt::Key_Enter:
		case Qt::Key_Return:
		case Qt::Key_Tab:
		case Qt::Key_Backtab:
		case Qt::Key_Up:
		case Qt::Key_Down:
			e->ignore(); // let the completer do the default processing
			break;

		case Qt::Key_Space:
			if (e->modifiers() & QT_CONTROL_MODIFIER) {
				d->keyPressControlSpace(e);
				break;
			}

			// fall through

		default:
			if (!ch.isPrint() || ch.isLetterOrNumber() || ch == '_') {
				QPlainTextEdit::keyPressEvent(e);
				break;
			}

			d->applyCompleter();
			keyPressEvent(e); // re-run
		}
}

void
EditBase::keyPressPrintChar(QKeyEvent* e) {
	Q_D(EditBase);

	QString text = e->text();
	QChar ch = text.isEmpty() ? QChar() : text.at(0);
	int c = ch.toLatin1();

	QTextCursor cursor = textCursor();

	switch (c) {
	case ')':
	case ']':
	case '}':
		if (cursor.hasSelection() || getCursorNextChar(cursor) != c || hasCursorHighlightColor(cursor)) {
			QPlainTextEdit::keyPressEvent(e);
			break;
		}

		cursor.movePosition(QTextCursor::NextCharacter);
		setTextCursor(cursor);
		break;

	case '(':
	case '[':
	case '{':
		QPlainTextEdit::keyPressEvent(e);

		if (hasCursorHighlightColor(cursor))
			break;

		if (isBraceAutoComplete(getCursorNextChar(cursor))) {
			cursor = textCursor();
			cursor.insertText(getRightBrace(c));
			cursor.movePosition(QTextCursor::PreviousCharacter);
			setTextCursor(cursor);
		}

		break;

	default:
		QPlainTextEdit::keyPressEvent(e);
	}
}

void
EditBase::mousePressEvent(QMouseEvent* e) {
	Q_D(EditBase);

	// check for triggers first

	QPlainTextEdit::mousePressEvent(e);
}

void
EditBase::mouseMoveEvent(QMouseEvent* e) {
	Q_D(EditBase);

	QPlainTextEdit::mouseMoveEvent(e);

	if (!d->isCompleterVisible() &&
		(d->m_codeAssistTriggers & QuickInfoTipOnMouseOverIdentifier)
	)
		d->requestQuickInfoTip(EditBasePrivate::CodeAssistDelay_QuickInfoTip, e->pos());
}

void
EditBase::enterEvent(QEvent* e) {
	Q_D(EditBase);

	QPlainTextEdit::enterEvent(e);

	if (!d->isCompleterVisible() &&
		(d->m_codeAssistTriggers & QuickInfoTipOnMouseOverIdentifier)
	) {
		QPoint pos = mapFromGlobal(QCursor::pos());
		d->requestQuickInfoTip(EditBasePrivate::CodeAssistDelay_QuickInfoTip, pos);
	}
}

void
EditBase::activateCompleter(const QModelIndex& index) {
	Q_D(EditBase);

	QModelIndex nameIndex = index.sibling(index.row(), NameColumn); // user could have clicked on synopsis
	QString completion = d->m_completer->popup()->model()->data(nameIndex, Qt::DisplayRole).toString();

	QTextCursor cursor = textCursor();
	cursor.setPosition(d->m_activeCodeAssistPosition);

	QChar c = getCursorNextChar(cursor);
	if (c.isLetterOrNumber() || c == '_')
		cursor.select(QTextCursor::WordUnderCursor);

	cursor.insertText(completion);
	setTextCursor(cursor);
}

//..............................................................................

EditBasePrivate::EditBasePrivate() {
	q_ptr = NULL;
	m_syntaxHighlighter = NULL;
	m_lineNumberMargin = NULL;
	m_tabWidth = 4;
	m_isCurrentLineHighlightingEnabled = false;
	m_isExtraSelectionUpdateRequired = false;
	m_isTabsToSpacesEnabled = false;
	m_highlighTable[HighlightKind_CurrentLine].format.setProperty(QTextFormat::FullWidthSelection, true);
	m_thread = NULL;
	m_codeTip = NULL;
	m_completer = NULL;
	m_activeCodeAssistKind = CodeAssistKind_None;
	m_activeCodeAssistPosition = -1;
	m_pendingCodeAssistKind = CodeAssistKind_None;
	m_pendingCodeAssistPosition = -1;

	m_codeAssistTriggers =
		EditBase::QuickInfoTipOnMouseOverIdentifier |
		EditBase::ArgumentTipOnCtrlShiftSpace |
		EditBase::ArgumentTipOnTypeLeftParenthesis |
		EditBase::ArgumentTipOnTypeComma |
		EditBase::AutoCompleteOnCtrlSpace |
		EditBase::AutoCompleteOnTypeDot |
		EditBase::AutoCompleteOnTypeIdentifier |
		EditBase::ImportAutoCompleteOnTypeQuotationMark |
		EditBase::GotoDefinitionOnCtrlClick;

	static const size_t iconIdxTable[] = {
		0,  // FileIcon = 0
		0,  // ObjectIcon
		1,  // NamespaceIcon
		2,  // EventIcon
		4,  // FunctionIcon
		6,  // PropertyIcon
		7,  // VariableIcon
		12, // FieldIcon
		11, // ConstIcon
		10, // TypeIcon
		9,  // TypedefIcon
	};

	QPixmap imageList(":/Images/ObjectIcons");
	int iconSize = imageList.height();

	for (size_t i = 1; i < countof(m_iconTable); i++)
		m_iconTable[i] = imageList.copy(iconIdxTable[i] * iconSize, 0, iconSize, iconSize);

	QFileIconProvider fip;
	m_iconTable[EditBase::FileIcon] = fip.icon(QFileIconProvider::File);
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
EditBasePrivate::cursorFromLineCol(
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
EditBasePrivate::cursorFromOffset(size_t offset) {
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
	int anchor = cursor.anchor();

	cursor.beginEditBlock();
	cursor.setPosition(start);
	cursor.movePosition(QTextCursor::StartOfLine);

	QTextCursor endCursor = cursor;
	endCursor.setPosition(end);

	QString indent = m_isTabsToSpacesEnabled ? QString(m_tabWidth, QChar(' ')) : QString('\t');
	for (; cursor < endCursor; moveCursorWithLimit(&cursor, endCursor, QTextCursor::Down))
		cursor.insertText(indent);

	cursor.endEditBlock();

	int newEnd = q->textCursor().selectionEnd();

	if (anchor == start) {
		cursor.setPosition(start);
		cursor.setPosition(newEnd, QTextCursor::KeepAnchor);
	} else {
		cursor.setPosition(newEnd);
		cursor.setPosition(start, QTextCursor::KeepAnchor);
	}

	q->setTextCursor(cursor);
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
	else if (!m_isTabsToSpacesEnabled)
		q->QPlainTextEdit::keyPressEvent(e);
	else {
		e->accept();
		int col = cursor.positionInBlock();
		cursor.beginEditBlock();
		cursor.insertText(QString(m_tabWidth - col % m_tabWidth, QChar(' ')));
		cursor.endEditBlock();
		q->setTextCursor(cursor);
	}
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
	cursor.beginEditBlock();
	cursor.removeSelectedText();

	int position0 = cursor.position();
	QTextCursor solCursor = cursor;
	solCursor.movePosition(QTextCursor::StartOfLine);
	int solPosition = solCursor.position();
	bool isSol = position0 == solPosition;
	if (isSol) {
		cursor.insertText(QChar('\n')); // shortcut -- no auto-indent needed
		cursor.endEditBlock();
		q->setTextCursor(cursor);
		return;
	}

	QString baseIndent;
	if (isCursorOnIndent(solCursor)) {
		solCursor.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
		if (position0 <= solCursor.position()) { // another shortcut
			int delta = position0 - solPosition;
			cursor.setPosition(solPosition);
			cursor.insertText(QChar('\n'));
			cursor.setPosition(cursor.position() + delta);
			cursor.endEditBlock();
			q->setTextCursor(cursor);
			return;
		}

		baseIndent = solCursor.selectedText();
	}

	cursor.movePosition(QTextCursor::PreviousWord);
	cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
	int eolPosition = cursor.position();
	if (eolPosition > position0) // do not overshoot
		cursor.setPosition(position0, QTextCursor::KeepAnchor);

	QString tailWord = cursor.selectedText();
	cursor.clearSelection();

	if (eolPosition < position0) { // trim trailing whitespace
		cursor.setPosition(eolPosition);
		cursor.setPosition(position0, QTextCursor::KeepAnchor);
		cursor.removeSelectedText();
	}

	q->autoIndent(&cursor, baseIndent, tailWord);
	cursor.endEditBlock();
	q->setTextCursor(cursor);
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
EditBasePrivate::keyPressControlSpace(QKeyEvent* e) {
	Q_Q(EditBase);

	if (e->modifiers() & Qt::ShiftModifier) {
		if (m_codeAssistTriggers & EditBase::ArgumentTipOnCtrlShiftSpace)
			requestCodeAssist(0, CodeAssistKind_ArgumentTip);
	} else {
		if (m_codeAssistTriggers & EditBase::AutoCompleteOnCtrlSpace)
			requestCodeAssist(0, CodeAssistKind_AutoComplete);
	}
}

void
EditBasePrivate::timerEvent(QTimerEvent* e) {
	Q_Q(EditBase);

	if (e->timerId() != m_codeAssistTimer.timerId())
		return;

	m_codeAssistTimer.stop();
	startCodeAssistThread(m_pendingCodeAssistKind, m_pendingCodeAssistPosition);
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
EditBasePrivate::requestCodeAssist(
	int delay,
	CodeAssistKind kind
) {
	Q_Q(EditBase);

	QTextCursor cursor = q->textCursor();
	requestCodeAssist(delay, kind, cursor.position());
}

void
EditBasePrivate::requestCodeAssist(
	int delay,
	CodeAssistKind kind,
	int position
) {
	Q_Q(EditBase);

	if (m_thread) {
		m_thread->cancel();
		m_thread = NULL;
	}

	if (!delay) {
		m_codeAssistTimer.stop();
		startCodeAssistThread(kind, position);
	} else {
		m_pendingCodeAssistKind = kind;
		m_pendingCodeAssistPosition = position;
		m_codeAssistTimer.start(delay, this);
	}
}

void
EditBasePrivate::requestQuickInfoTip(
	int delay,
	const QPoint& pos
) {
	Q_Q(EditBase);

	QTextCursor cursor = q->cursorForPosition(pos);
	requestCodeAssist(delay, CodeAssistKind_QuickInfoTip, cursor.position());
}

void
EditBasePrivate::startCodeAssistThread(
	CodeAssistKind kind,
	int position
) {
	Q_Q(EditBase);

	if (m_thread)
		m_thread->cancel();

	m_thread = q->createCodeAssistThread();
	m_thread->m_importDirList = m_importDirList;
	m_thread->m_importList = m_importList;

	if (!m_extraSource.isEmpty()) {
		QByteArray source = m_extraSource.toUtf8();
		m_thread->m_extraSource = sl::String(source.constData(), source.length());
	}

	QObject::connect(
		m_thread, SIGNAL(ready()),
		this, SLOT(onCodeAssistThreadReady())
	);

	QObject::connect(
		m_thread, SIGNAL(finished()),
		this, SLOT(onCodeAssistThreadFinished())
	);

	m_thread->request(
		!m_fileName.isEmpty() ? m_fileName : QStringLiteral("untitled-source"),
		kind,
		position,
		q->toPlainText()
	);
}

CodeTipBase*
EditBasePrivate::ensureCodeTip() {
	if (m_codeTip)
		return m_codeTip;

	Q_Q(EditBase);
	m_codeTip = q->createCodeTip();
	return m_codeTip;
}

QCompleter*
EditBasePrivate::ensureCompleter() {
	if (m_completer)
		return m_completer;

	Q_Q(EditBase);

	QTreeView* popup = new QTreeView;
	CompleterItemDelegate* itemDelegate = new CompleterItemDelegate(popup, &m_theme);
	popup->setHeaderHidden(true);
	popup->setRootIsDecorated(false);
	popup->setSelectionBehavior(QAbstractItemView::SelectRows);
	popup->setFont(q->font());
	popup->setPalette(m_theme.completerPalette());

	popup->setItemDelegateForColumn(EditBase::NameColumn, itemDelegate);
	popup->setItemDelegateForColumn(EditBase::DetailColumn, itemDelegate);

	m_completer = new QCompleter(q);
	m_completer->setWidget(q);
	m_completer->setCompletionMode(QCompleter::PopupCompletion);
	m_completer->setMaxVisibleItems(Limit_MaxVisibleItemCount);
	m_completer->setPopup(popup);

	QObject::connect(
		m_completer, SIGNAL(activated(const QModelIndex&)),
		this, SLOT(onCompleterActivated(const QModelIndex&))
	);

	return m_completer;
}

void
EditBasePrivate::updateCompleter(bool isForced) {
	ASSERT(m_completer);

	Q_Q(EditBase);

	QTextCursor cursor = q->textCursor();
	int position = cursor.position();
	if (position < m_activeCodeAssistPosition) {
		hideCodeAssist();
		return;
	}

	cursor.setPosition(position, QTextCursor::MoveAnchor);
	cursor.setPosition(m_activeCodeAssistPosition, QTextCursor::KeepAnchor);
	QString prefix = cursor.selectedText();

	if (m_activeCodeAssistKind == CodeAssistKind_ImportAutoComplete)
		prefix.remove(0, 1); // opening quotation mark

	if (isForced)
		m_completerRect = activeCodeAssistCursorRect();
	else if (prefix == m_completer->completionPrefix())
		return;

	QAbstractItemView* popup = (QTreeView*)m_completer->popup();
	QTreeView* treeView = (QTreeView*)popup;
	m_completer->setCompletionPrefix(prefix);
	popup->setCurrentIndex(m_completer->model()->index(0, 0));

	QMargins margins = treeView->contentsMargins();
	int marginWidth = margins.left() + margins.right();
	int scrollWidth = popup->verticalScrollBar()->sizeHint().width();
	int nameWidth = popup->sizeHintForColumn(EditBase::NameColumn);
	int synopsisWidth = popup->sizeHintForColumn(EditBase::DetailColumn);

	if (nameWidth > Limit_MaxNameWidth)
		nameWidth = Limit_MaxNameWidth;

	if (synopsisWidth > Limit_MaxSynopsisWidth)
		synopsisWidth = Limit_MaxSynopsisWidth;

	treeView->setColumnWidth(EditBase::NameColumn, nameWidth);
	treeView->setColumnWidth(EditBase::DetailColumn, synopsisWidth);

	int fullWidth = nameWidth + synopsisWidth + scrollWidth + marginWidth;
	m_completerRect.setWidth(fullWidth);
	m_completer->complete(m_completerRect);
}

void
EditBasePrivate::applyCompleter() {
	Q_Q(EditBase);

	ASSERT(m_completer);

	QModelIndex index = m_completer->popup()->currentIndex();
	if (index.isValid())
		onCompleterActivated(index);

	hideCodeAssist();
}

QTextCursor
EditBasePrivate::activeCodeAssistCursor() {
	Q_Q(EditBase);

	QTextCursor cursor = q->textCursor();
	cursor.setPosition(m_activeCodeAssistPosition);
	return cursor;
}

QRect
EditBasePrivate::activeCodeAssistCursorRect() {
	Q_Q(EditBase);

	QTextCursor cursor = activeCodeAssistCursor();
	QRect rect = q->cursorRect(cursor);

#if (QT_VERSION >= 0x050500)
	rect.translate(q->viewportMargins().left(), q->viewportMargins().top());
#else
	rect.translate(m_lineNumberMargin ? m_lineNumberMargin->width() : 0, 0);
#endif

	return rect;
}

QPoint
EditBasePrivate::activeCodeTipPoint(bool isBelowCurrentCursor) {
	Q_Q(EditBase);

	QRect rect = activeCodeAssistCursorRect();

	if (isBelowCurrentCursor)
		rect.moveTop(q->cursorRect().top());

	return q->mapToGlobal(rect.bottomLeft());
}

	void setActiveCodeAssist(
		CodeAssistKind codeAssistKind,
		int position
	);

void
EditBasePrivate::hideCodeAssist() {
	Q_Q(EditBase);

	if (m_completer)
		m_completer->popup()->hide();

	if (m_codeTip)
		m_codeTip->close();

	m_thread = NULL; // prevent any pending code assist to pop
	m_activeCodeAssistKind = CodeAssistKind_None;
	m_activeCodeAssistPosition = -1;

	q->releaseCodeAssist();
}

void
EditBasePrivate::onCursorPositionChanged() {
	switch (m_activeCodeAssistKind) {
	case CodeAssistKind_QuickInfoTip:
		hideCodeAssist();
		break;

	case CodeAssistKind_ArgumentTip:
		requestCodeAssist(CodeAssistDelay_ArgumentTipPos, CodeAssistKind_ArgumentTip);
		break;

	case CodeAssistKind_AutoComplete:
	case CodeAssistKind_ImportAutoComplete:
		if (isCompleterVisible())
			updateCompleter();
		break;
	}

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

void
EditBasePrivate::onCompleterActivated(const QModelIndex& index) {
	Q_Q(EditBase);
	q->activateCompleter(index);
}

void
EditBasePrivate::onCodeAssistThreadReady() {
	Q_Q(EditBase);

	CodeAssistThreadBase* thread = qobject_cast<CodeAssistThreadBase*>(sender());
	ASSERT(thread);

	if (thread == m_thread)
		q->showCodeAssist(thread);
}

void
EditBasePrivate::onCodeAssistThreadFinished() {
	CodeAssistThreadBase* thread = qobject_cast<CodeAssistThreadBase*>(sender());
	ASSERT(thread);

	if (thread == m_thread)
		m_thread = NULL;

	thread->deleteLater();
}

//..............................................................................

void
CompleterItemDelegate::paint(
	QPainter* painter,
	const QStyleOptionViewItem& option,
	const QModelIndex& index
) const {
	if (index.column() != EditBase::DetailColumn) {
		QStyledItemDelegate::paint(painter, option, index);
		return;
	}

	QColor color = m_theme->color(EditTheme::CompleterSynopsisColumn);
	QStyleOptionViewItem altOption = option;
	altOption.palette.setColor(QPalette::Text, color);
	altOption.palette.setColor(QPalette::WindowText, color);
	QStyledItemDelegate::paint(painter, altOption, index);
}

//..............................................................................

} // namespace jnc
