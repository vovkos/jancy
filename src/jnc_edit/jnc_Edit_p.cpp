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
#include "jnc_Edit_p.h"
#include "jnc_CodeTip.h"
#include "moc_jnc_Edit.cpp"
#include "moc_jnc_Edit_p.cpp"
#include "qrc_res.cpp"

namespace jnc {

//..............................................................................

static class Init {
public:
	Init() {
		g::getModule()->setTag("jnc_edit");
		g_defaultDarkTheme.setDefaultDarkTheme();
	}
} g_init;

//..............................................................................

lex::LineCol
getCursorLineCol(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	cursor.movePosition(QTextCursor::StartOfLine);

	int line = 0;
	while (cursor.positionInBlock() > 0) {
		line++;
		cursor.movePosition(QTextCursor::Up);
	}

	QTextBlock block = cursor.block().previous();
	while (block.isValid()) {
		line += block.lineCount();
		block = block.previous();
	}

	return lex::LineCol(line, cursor0.columnNumber());
}

bool
isCursorMultiLineSelection(const QTextCursor& cursor0) {
	if (!cursor0.hasSelection())
		return false;

	QTextCursor cursor = cursor0;
	int start = cursor.anchor();
	int end = cursor.position();

	if (start > end) {
		int t = start;
		start = end;
		end = t;
	}

	cursor.setPosition(start);
	cursor.movePosition(QTextCursor::StartOfLine);
	cursor.movePosition(QTextCursor::Down);
	return cursor.position() <= end;
}

bool
isCursorAtStartOfLine(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	int position = cursor.position();
	cursor.movePosition(QTextCursor::StartOfLine);
	return cursor.position() == position;
}

inline
bool
isCursorAtEndOfLineIgnoreSpace(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	cursor.setPosition(cursor.position());
	cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
	return cursor.selectedText().trimmed().isEmpty();
}

inline
int
getCursorStartOfLinePosition(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	cursor.movePosition(QTextCursor::StartOfLine);
	return cursor.position();
}

inline
int
getCursorEndOfLinePosition(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	cursor.movePosition(QTextCursor::EndOfLine);
	return cursor.position();
}

QString
getCursorLinePrefix(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	int position = cursor.position();
	cursor.setPosition(position);
	cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
	return cursor.selectedText();
}

QString
getCursorLineSuffix(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	int position = cursor.position();
	cursor.setPosition(position);
	cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
	return cursor.selectedText();
}

bool
isCursorLineEmpty(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	cursor.select(QTextCursor::LineUnderCursor);
	return cursor.selectedText().trimmed().isEmpty();
}

bool
isCursorNextLineEmpty(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	cursor.movePosition(QTextCursor::Down);
	return isCursorLineEmpty(cursor);
}

QString
getCursorPrevWord(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
	return cursor.selectedText();
}

QChar
getCursorPrevChar(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	int position = cursor.position();
	cursor.setPosition(position);

	int sol = getCursorStartOfLinePosition(cursor);
	if (position <= sol)
		return QChar();

	cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
	QString selection = cursor.selectedText();
	return !selection.isEmpty() ? selection.at(0) : QChar();
}

QChar
getCursorNextChar(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	int position = cursor.position();
	cursor.setPosition(position);

	int eol = getCursorEndOfLinePosition(cursor);
	if (position >= eol)
		return QChar();

	cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
	QString selection = cursor.selectedText();
	return !selection.isEmpty() ? selection.at(0) : QChar();
}

bool
isCursorOnIndent(const QTextCursor& cursor0) {
	QTextCursor cursor = cursor0;
	int position = cursor.position();
	cursor.movePosition(QTextCursor::StartOfLine);

	if (cursor.position() == position) // already was at start-of-line
		return getCursorNextChar(cursor).isSpace();

	cursor.setPosition(position, QTextCursor::KeepAnchor);
	QString selection = cursor.selectedText();
	return !selection.isEmpty() && selection.at(0).isSpace() && selection.trimmed().isEmpty();
}

bool
hasCursorHighlightColor(const QTextCursor& cursor) {
	if (cursor.atBlockEnd() && cursor.block().userState() != 0)
		return true;

#if (QT_VERSION >= 0x050600)
	QVector<QTextLayout::FormatRange> formats = cursor.block().layout()->formats();
#else
	QList<QTextLayout::FormatRange> formats = cursor.block().layout()->additionalFormats();
#endif

	// binary search

	int pos = cursor.positionInBlock();
	int left = 0;
	int right = formats.count();

	while (left < right) {
		int i = (left + right) / 2;

		QTextLayout::FormatRange range = formats[i];
		int end = range.start + range.length;

		if (pos < range.start)
			right = i;
		else if (pos >= end)
			left = i + 1;
		else
			return true;
	}

	return false;
}

void
moveCursorWithLimit(
	QTextCursor* cursor,
	const QTextCursor& limitCursor,
	QTextCursor::MoveOperation op,
	QTextCursor::MoveMode mode = QTextCursor::MoveAnchor,
	int n = 1
) {
	int pos = cursor->position();
	cursor->movePosition(op, mode, n);

	if (cursor->position() == pos) // didn't change, use limit
		*cursor = limitCursor;
}

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

Edit::Edit(QWidget *parent):
	QPlainTextEdit(parent),
	d_ptr(new EditPrivate) {
	Q_D(Edit);

	d->q_ptr = this;
	d->init();
}

Edit::~Edit() {}

bool
Edit::isLineNumberMarginEnabled() {
	Q_D(Edit);
	return d->m_lineNumberMargin != NULL;
}

void
Edit::enableLineNumberMargin(bool isEnabled) {
	Q_D(Edit);
	d->enableLineNumberMargin(isEnabled);
}

int Edit::lineNumberMarginWidth() {
	Q_D(Edit);
	return d->m_lineNumberMargin ? d->m_lineNumberMargin->width() : 0;
}

bool
Edit::isCurrentLineHighlightingEnabled() {
	Q_D(Edit);
	return d->m_isCurrentLineHighlightingEnabled;
}

void
Edit::enableCurrentLineHighlighting(bool isEnabled) {
	Q_D(Edit);
	d->enableCurrentLineHighlighting(isEnabled);
}

bool
Edit::isSyntaxHighlightingEnabled() {
	Q_D(Edit);
	return d->m_syntaxHighlighter != NULL;
}

void
Edit::enableSyntaxHighlighting(bool isEnabled) {
	Q_D(Edit);
	d->enableSyntaxHighlighting(isEnabled);
}

int
Edit::tabWidth() {
	Q_D(Edit);
	return d->m_tabWidth;
}

void
Edit::setTabWidth(int width) {
	Q_D(Edit);

	d->m_tabWidth = width;
	setTabStopWidth(fontMetrics().width(' ') * width);
}

const EditTheme*
Edit::theme() {
	Q_D(Edit);
	return &d->m_theme;
}

void
Edit::setTheme(const EditTheme* theme) {
	Q_D(Edit);

	d->m_theme = *theme;
	d->applyTheme();
	viewport()->update();
}

Edit::CodeAssistTriggers
Edit::codeAssistTriggers() {
	Q_D(Edit);
	return d->m_codeAssistTriggers;
}

void
Edit::setCodeAssistTriggers(CodeAssistTriggers triggers) {
	Q_D(Edit);
	d->m_codeAssistTriggers = triggers;
}

QStringList
Edit::importDirList() {
	Q_D(Edit);
	return d->m_importDirList;
}

void
Edit::setImportDirList(const QStringList& dirList) {
	Q_D(Edit);
	d->m_importDirList = dirList;
}

QStringList
Edit::importList() {
	Q_D(Edit);
	return d->m_importList;
}

void
Edit::setImportList(const QStringList& importList) {
	Q_D(Edit);
	d->m_importList = importList;
}

QString
Edit::extraSource() {
	Q_D(Edit);
	return d->m_extraSource;
}

void
Edit::setExtraSource(const QString& source) {
	Q_D(Edit);
	d->m_extraSource = source;
}

void
Edit::setTextCursorLineCol(
	int line,
	int col
) {
	Q_D(Edit);
	setTextCursor(d->getCursorFromLineCol(line, col));
}

void
Edit::highlightLineTemp(
	int line,
	const QColor& backColor,
	const QColor& textColor
) {
	Q_D(Edit);

	QTextEdit::ExtraSelection selection;
	selection.cursor = d->getCursorFromLineCol(line, 0);
	selection.format.setProperty(QTextFormat::FullWidthSelection, true);

	if (backColor.isValid())
		selection.format.setBackground(backColor);

	if (textColor.isValid())
		selection.format.setForeground(textColor);

	d->m_highlighTable[EditPrivate::HighlightKind_Temp] = selection;
	d->updateExtraSelections();
}

void
Edit::quickInfoTip() {
	Q_D(Edit);
	d->requestCodeAssist(CodeAssistKind_QuickInfoTip);
}

void
Edit::argumentTip() {
	Q_D(Edit);
	d->requestCodeAssist(CodeAssistKind_ArgumentTip);
}

void
Edit::autoComplete() {
	Q_D(Edit);
	d->requestCodeAssist(CodeAssistKind_AutoComplete);
}

void
Edit::gotoDefinition() {
	Q_D(Edit);
	d->requestCodeAssist(CodeAssistKind_GotoDefinition);
}

void
Edit::indentSelection() {
	Q_D(Edit);
	d->indentSelection();
}

void
Edit::unindentSelection() {
	Q_D(Edit);
	d->unindentSelection();
}

void
Edit::changeEvent(QEvent* e) {
	Q_D(Edit);

	QPlainTextEdit::changeEvent(e);

	if (e->type() == QEvent::FontChange)
		d->updateFont();
}

void
Edit::resizeEvent(QResizeEvent *e) {
	Q_D(Edit);

	QPlainTextEdit::resizeEvent(e);

	if (d->m_lineNumberMargin)
		d->updateLineNumberMarginGeometry();
}

void
Edit::keyPressEvent(QKeyEvent* e) {
	Q_D(Edit);

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
			if (!d->isCodeTipVisible() || !d->m_codeTip->isFunctionTypeOverload())
				QPlainTextEdit::keyPressEvent(e);
			else if (key == Qt::Key_Up)
				d->m_codeTip->prevFunctionTypeOverload();
			else
				d->m_codeTip->nextFunctionTypeOverload();

			break;

		case Qt::Key_Space:
			if (e->modifiers() & QT_CONTROL_MODIFIER) {
				d->keyPressControlSpace(e);
				break;
			}

			// fall through

		default:
			if (ch.isPrint())
				d->keyPressPrintChar(e);
			else
				QPlainTextEdit::keyPressEvent(e);
		} else
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

		case Qt::Key_Home:
			d->keyPressHome(e);
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
Edit::mousePressEvent(QMouseEvent* e) {
	Q_D(Edit);

	// check for triggers first

	QPlainTextEdit::mousePressEvent(e);
}

void
Edit::mouseMoveEvent(QMouseEvent* e) {
	Q_D(Edit);

	QPlainTextEdit::mouseMoveEvent(e);

	if (!d->isCompleterVisible() &&
		(d->m_codeAssistTriggers & QuickInfoTipOnMouseOverIdentifier))
		d->requestQuickInfoTip(e->pos());
}

void
Edit::enterEvent(QEvent* e) {
	Q_D(Edit);

	QPlainTextEdit::enterEvent(e);

	if (!d->isCompleterVisible() &&
		d->m_lastCodeAssistKind == CodeAssistKind_QuickInfoTip &&
		(d->m_codeAssistTriggers & QuickInfoTipOnMouseOverIdentifier)) {
		QPoint pos = mapFromGlobal(QCursor::pos());
		d->requestQuickInfoTip(pos);
	}
}

//..............................................................................

EditPrivate::EditPrivate() {
    q_ptr = NULL;
	m_syntaxHighlighter = NULL;
	m_lineNumberMargin = NULL;
	m_isCurrentLineHighlightingEnabled = false;
	m_isExtraSelectionUpdateRequired = false;
	m_tabWidth = 4;
	m_thread = NULL;
	m_codeTip = NULL;
	m_completer = NULL;
	m_lastCodeAssistKind = CodeAssistKind_Undefined;
	m_lastCodeAssistOffset = -1;
	m_lastCodeAssistPosition = -1;
	m_pendingCodeAssistPosition = -1;

	m_codeAssistTriggers =
//		Edit::QuickInfoTipOnMouseOverIdentifier | // doesn't work quite well yet
		Edit::ArgumentTipOnCtrlShiftSpace |
		Edit::ArgumentTipOnTypeLeftParenthesis |
		Edit::ArgumentTipOnTypeComma |
		Edit::AutoCompleteOnCtrlSpace |
		Edit::AutoCompleteOnTypeDot |
		Edit::AutoCompleteOnTypeIdentifier |
		Edit::ImportAutoCompleteOnTypeQuotationMark |
		Edit::GotoDefinitionOnCtrlClick;

	m_highlighTable[HighlightKind_CurrentLine].format.setProperty(QTextFormat::FullWidthSelection, true);
}

void
EditPrivate::init() {
	Q_Q(Edit);

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

	enableSyntaxHighlighting(true);
	enableLineNumberMargin(true);
	enableCurrentLineHighlighting(true);

	QObject::connect(
		q, SIGNAL(cursorPositionChanged()),
		this, SLOT(onCursorPositionChanged())
	);

	static const size_t iconIdxTable[] = {
		0,  // Icon_Object
		1,  // Icon_Namespace
		2,  // Icon_Event
		4,  // Icon_Function
		6,  // Icon_Property
		7,  // Icon_Variable
		12, // Icon_Field
		11, // Icon_Const
		10, // Icon_Type
		9,  // Icon_Typedef
 	};

	QPixmap imageList(":/Images/ObjectIcons");
	int iconSize = imageList.height();

	for (size_t i = 0; i < countof(m_iconTable); i++)
		m_iconTable[i] = imageList.copy(iconIdxTable[i] * iconSize, 0, iconSize, iconSize);

	applyTheme();
}

void
EditPrivate::applyTheme() {
	Q_Q(Edit);

	q->setPalette(m_theme.palette());

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
		m_syntaxHighlighter->m_theme = &m_theme;

	if (m_completer)
		m_completer->popup()->setPalette(m_theme.palette());

	if (m_lineNumberMargin)
		m_lineNumberMargin->update();
}

void
EditPrivate::enableSyntaxHighlighting(bool isEnabled) {
	Q_Q(Edit);

	if (isEnabled) {
		if (!m_syntaxHighlighter)
			m_syntaxHighlighter = new JancyHighlighter(q->document(), &m_theme);
	} else if (m_syntaxHighlighter) {
		m_syntaxHighlighter->setDocument(NULL);
		delete m_syntaxHighlighter;
		m_syntaxHighlighter = NULL;
	}
}

void
EditPrivate::enableLineNumberMargin(bool isEnabled) {
	Q_Q(Edit);

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
EditPrivate::enableCurrentLineHighlighting(bool isEnabled) {
	Q_Q(Edit);

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
EditPrivate::updateExtraSelections() {
	Q_Q(Edit);

	QList<QTextEdit::ExtraSelection> list;

	for (size_t i = 0; i < countof(m_highlighTable); i++)
		if (!m_highlighTable[i].cursor.isNull())
			list.append(m_highlighTable[i]);

	q->setExtraSelections(list);
	m_isExtraSelectionUpdateRequired = false;
}

void
EditPrivate::updateFont() {
	Q_Q(Edit);

	q->setTabStopWidth(q->fontMetrics().width(' ') * m_tabWidth);

	if (m_codeTip)
		m_codeTip->setFont(q->font());

	if (m_lineNumberMargin) {
		m_lineNumberMargin->updateFontMetrics();
		q->setViewportMargins(m_lineNumberMargin->width(), 0, 0, 0);
	}
}

void
EditPrivate::updateLineNumberMarginGeometry() {
	ASSERT(m_lineNumberMargin);

	Q_Q(Edit);

	QRect rect = q->contentsRect();

	m_lineNumberMargin->setGeometry(
		rect.left(),
		rect.top(),
		m_lineNumberMargin->width(),
		rect.height()
	);
}

void
EditPrivate::requestCodeAssist(CodeAssistKind kind) {
	Q_Q(Edit);

	QTextCursor cursor = q->textCursor();
	requestCodeAssist(kind, cursor.position());
}

void
EditPrivate::requestCodeAssist(
	CodeAssistKind kind,
	int position
) {
	Q_Q(Edit);

	if (m_thread)
		m_thread->cancel();


	m_thread = new CodeAssistThread(this);
	m_thread->m_importDirList = m_importDirList;
	m_thread->m_importList = m_importList;

	if (!m_extraSource.isEmpty()) {
		QByteArray source = m_extraSource.toUtf8();
		m_thread->m_extraSource = sl::String(source.constData(), source.length());
	}

	QObject::connect(
		m_thread, SIGNAL(ready()),
		this, SLOT(onCodeAssistReady())
	);

	QObject::connect(
		m_thread, SIGNAL(finished()),
		this, SLOT(onThreadFinished())
	);

	m_thread->request(kind, rc::g_nullPtr, position, q->toPlainText());
}

void
EditPrivate::requestQuickInfoTip(const QPoint& pos) {
	Q_Q(Edit);

	QTextCursor cursor = q->cursorForPosition(pos);
	m_pendingCodeAssistPosition = cursor.position();
	m_quickInfoTipTimer.start(Timeout_QuickInfo, this);
}

void
EditPrivate::hideCodeAssist() {
	if (m_completer)
		m_completer->popup()->hide();

	if (m_codeTip)
		m_codeTip->close();

	m_lastCodeAssistModule = rc::g_nullPtr;
	m_lastCodeAssistKind = CodeAssistKind_Undefined;
	m_lastCodeAssistPosition = -1;
	m_thread = NULL;
}

void
EditPrivate::updateLineNumberMargin(
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
EditPrivate::highlightCurrentLine() {
	Q_Q(Edit);

	QTextCursor cursor = q->textCursor();
	cursor.clearSelection();
	m_highlighTable[HighlightKind_CurrentLine].cursor = cursor;

	m_isExtraSelectionUpdateRequired = true;
}

void
EditPrivate::ensureCodeTip() {
	if (m_codeTip)
		return;

	Q_Q(Edit);

	m_codeTip = new CodeTip(q, &m_theme);
	m_codeTip->setFont(q->font());
}

void
EditPrivate::ensureCompleter() {
	if (m_completer)
		return;

	Q_Q(Edit);

	QTreeView* popup = new QTreeView;
	CompleterItemDelegate* itemDelegate = new CompleterItemDelegate(popup, &m_theme);
	popup->setHeaderHidden(true);
	popup->setRootIsDecorated(false);
	popup->setSelectionBehavior(QAbstractItemView::SelectRows);
	popup->setFont(q->font());
	popup->setPalette(m_theme.palette());
	popup->setItemDelegateForColumn(Column_Name, itemDelegate);
	popup->setItemDelegateForColumn(Column_Synopsis, itemDelegate);

	m_completer = new QCompleter(q);
	m_completer->setWidget(q);
	m_completer->setCompletionMode(QCompleter::PopupCompletion);
	m_completer->setMaxVisibleItems(Limit_MaxVisibleItemCount);
	m_completer->setPopup(popup);

    QObject::connect(
		m_completer, SIGNAL(activated(const QModelIndex&)),
		this, SLOT(onCompleterActivated(const QModelIndex&))
	);
}

void
EditPrivate::updateCompleter(bool isForced) {
	ASSERT(m_completer);

	Q_Q(Edit);

	QTextCursor cursor = q->textCursor();
	int position = cursor.position();
	int anchorPosition = getLastCodeAssistPosition();
	if (position < anchorPosition) {
		hideCodeAssist();
		return;
	}

	cursor.setPosition(position, QTextCursor::MoveAnchor);
	cursor.setPosition(anchorPosition, QTextCursor::KeepAnchor);
	QString prefix = cursor.selectedText();

	if (m_lastCodeAssistKind == CodeAssistKind_ImportAutoComplete)
		prefix.remove(0, 1); // opening quotation mark

	if (!isForced && prefix == m_completer->completionPrefix())
		return;

	QAbstractItemView* popup = (QTreeView*)m_completer->popup();
	QTreeView* treeView = (QTreeView*)popup;
	m_completer->setCompletionPrefix(prefix);
	popup->setCurrentIndex(m_completer->completionModel()->index(0, 0));

	QMargins margins = treeView->contentsMargins();
	int marginWidth = margins.left() + margins.right();
	int scrollWidth = popup->verticalScrollBar()->sizeHint().width();
	int nameWidth = popup->sizeHintForColumn(Column_Name);
	int synopsisWidth = popup->sizeHintForColumn(Column_Synopsis);

	if (nameWidth > Limit_MaxNameWidth)
		nameWidth = Limit_MaxNameWidth;

	if (synopsisWidth > Limit_MaxSynopsisWidth)
		synopsisWidth = Limit_MaxSynopsisWidth;

	treeView->setColumnWidth(Column_Name, nameWidth);
	treeView->setColumnWidth(Column_Synopsis, synopsisWidth);

	int fullWidth = nameWidth + synopsisWidth + scrollWidth + marginWidth;
	m_completerRect.setWidth(fullWidth);
	m_completer->complete(m_completerRect);
}

void
EditPrivate::applyCompleter() {
	Q_Q(Edit);

	ASSERT(m_completer);

	QModelIndex index = m_completer->popup()->currentIndex();
	if (index.isValid())
		onCompleterActivated(index);

	hideCodeAssist();
}

QTextCursor
EditPrivate::getCursorFromLineCol(
	int line,
	int col
) {
	Q_Q(Edit);

	QTextCursor cursor = q->textCursor();
	cursor.setPosition(0);
	cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line);
	cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, col);
	return cursor;
}

QTextCursor
EditPrivate::getCursorFromOffset(size_t offset) {
	Q_Q(Edit);

	QString prefix = q->toPlainText().toUtf8().left(offset);
	int position = prefix.length();
	QTextCursor cursor = q->textCursor();
	cursor.setPosition(position);
	return cursor;
}

QTextCursor
EditPrivate::getLastCodeAssistCursor() {
	Q_Q(Edit);

	int position = getLastCodeAssistPosition();
	QTextCursor cursor = q->textCursor();
	cursor.setPosition(position);
	return cursor;
}

QRect
EditPrivate::getLastCodeAssistCursorRect() {
	Q_Q(Edit);

	QTextCursor cursor = getLastCodeAssistCursor();
	QRect rect = q->cursorRect(cursor);

#if (QT_VERSION >= 0x050500)
	rect.translate(q->viewportMargins().left(), q->viewportMargins().top());
#else
	rect.translate(m_lineNumberMargin ? m_lineNumberMargin->width() : 0, 0);
#endif

	return rect;
}

int
EditPrivate::calcLastCodeAssistPosition() {
	ASSERT(m_lastCodeAssistKind && m_lastCodeAssistPosition == -1);

	QTextCursor cursor = getCursorFromOffset(m_lastCodeAssistOffset);
	m_lastCodeAssistPosition = cursor.position();
	return m_lastCodeAssistPosition;
}

QPoint
EditPrivate::getLastCodeTipPoint(bool isBelowCurrentCursor) {
	Q_Q(Edit);

	QRect rect = getLastCodeAssistCursorRect();

	if (isBelowCurrentCursor)
		rect.moveTop(q->cursorRect().top());

	return q->mapToGlobal(rect.bottomLeft());
}

void
EditPrivate::createQuickInfoTip(ModuleItem* item) {
	Q_Q(Edit);

	QPoint point = getLastCodeTipPoint();

	ensureCodeTip();
	m_codeTip->showQuickInfoTip(point, item);
}

void
EditPrivate::createArgumentTip(
	FunctionTypeOverload* typeOverload,
	size_t argumentIdx
) {
	Q_Q(Edit);

	QPoint point = getLastCodeTipPoint();

	ensureCodeTip();
	m_codeTip->showArgumentTip(point, typeOverload, argumentIdx);
}

size_t
EditPrivate::getItemIconIdx(ModuleItem* item) {
	static const size_t iconIdxTable[ModuleItemKind__Count] = {
		Icon_Object,    // ModuleItemKind_Undefined
		Icon_Namespace, // ModuleItemKind_Namespace
		Icon_Object,    // ModuleItemKind_Attribute
		Icon_Object,    // ModuleItemKind_AttributeBlock
		Icon_Object,    // ModuleItemKind_Scope
		Icon_Type,      // ModuleItemKind_Type
		Icon_Typedef,   // ModuleItemKind_Typedef
		Icon_Object,    // ModuleItemKind_Alias
		Icon_Const,     // ModuleItemKind_Const
		Icon_Variable,  // ModuleItemKind_Variable
		Icon_Function,  // ModuleItemKind_Function
		Icon_Variable,  // ModuleItemKind_FunctionArg
		Icon_Function,  // ModuleItemKind_FunctionOverload
		Icon_Property,  // ModuleItemKind_Property
		Icon_Property,  // ModuleItemKind_PropertyTemplate
		Icon_Const,     // ModuleItemKind_EnumConst
		Icon_Variable,  // ModuleItemKind_Field
		Icon_Type,      // ModuleItemKind_BaseTypeSlot
		Icon_Function,  // ModuleItemKind_Orphan
		Icon_Object,    // ModuleItemKind_LazyImport
	};

	ModuleItemKind itemKind = item->getItemKind();
	return ((size_t)itemKind < countof(iconIdxTable)) ? iconIdxTable[itemKind] : Icon_Object;
}

void
EditPrivate::addAutoCompleteNamespace(
	QStandardItemModel* model,
	Namespace* nspace
) {
	NamespaceKind namespaceKind = nspace->getNamespaceKind();
	if (namespaceKind == NamespaceKind_Type) {
		NamedType* namedType = (NamedType*)nspace->getParentItem();
		if (namedType->getTypeKind() == TypeKind_Enum) {
			EnumType* enumType = (EnumType*)namedType;
			Type* baseType = enumType->getBaseType();
			if (baseType->getTypeKind() == TypeKind_Enum)
				addAutoCompleteNamespace(model, baseType->getNamespace());
		} else if (namedType->getTypeKindFlags() & TypeKindFlag_Derivable) {
			DerivableType* derivableType = (DerivableType*)namedType;
			size_t count = derivableType->getBaseTypeCount();
			for (size_t i = 0; i < count; i++) {
				BaseTypeSlot* slot = derivableType->getBaseType(i);
				DerivableType* baseType = slot->getType();
				if (!(baseType->getTypeKindFlags() & TypeKindFlag_Import)) // still unresolved
					addAutoCompleteNamespace(model, baseType->getNamespace());
			}
		}
	}

	size_t count = nspace->getItemCount();
	for (size_t i = 0; i < count; i++) {
		ModuleItem* item = nspace->getItem(i);
		ModuleItemKind itemKind = item->getItemKind();
		Type* type = item->getType();
		QString name = item->getDecl()->getName();
		QString synopsis = item->getSynopsis_v();
		size_t iconIdx = getItemIconIdx(item);

		QStandardItem* nameItem = new QStandardItem;
		nameItem->setText(name);
		nameItem->setData(name.toLower(), Role_CaseInsensitiveSort);
		nameItem->setData(QVariant::fromValue((void*)item), Role_ModuleItem);

		QStandardItem* synopsisItem = new QStandardItem;
		synopsisItem->setText(synopsis);

		if (iconIdx != -1)
			synopsisItem->setIcon(m_iconTable[iconIdx]);

		QList<QStandardItem*> row;
		row.append(nameItem);
		row.append(synopsisItem);

		model->appendRow(row);
	}
}

void
EditPrivate::createAutoComplete(
	Namespace* nspace,
	uint_t flags
) {
	Q_Q(Edit);

	if (flags & CodeAssistFlag_AutoCompleteFallback) {
		QTextCursor cursor = getLastCodeAssistCursor();
		if (hasCursorHighlightColor(cursor)) // not within keywords/literals/comments/etc
			return;

		if (!(flags & CodeAssistFlag_QualifiedName) && getCursorPrevChar(cursor) == '.') // not after the member operator
			return;
	}

	QStandardItemModel* model = new QStandardItemModel(m_completer);
	addAutoCompleteNamespace(model, nspace);

	if (flags & CodeAssistFlag_IncludeParentNamespace) {
		nspace = nspace->getParentNamespace();

		while (nspace) {
			addAutoCompleteNamespace(model, nspace);
			nspace = nspace->getParentNamespace();
		}
	}

	ensureCompleter();

	model->setSortRole(Role_CaseInsensitiveSort);
	model->sort(0);

	m_completer->setModel(model);
	m_completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_completer->setWrapAround(false);
	m_completer->setCompletionPrefix(QString());

	m_completerRect = getLastCodeAssistCursorRect();
	updateCompleter(true);
}

void
EditPrivate::addFile(
	QStandardItemModel* model,
	const QString& fileName
) {
	QStandardItem* qtItem = new QStandardItem;
	qtItem->setText(fileName);
	qtItem->setData(fileName.toLower(), Role_CaseInsensitiveSort);
	qtItem->setIcon(m_fileIconProvider.icon(QFileIconProvider::File));

	model->appendRow(qtItem);
}

void
EditPrivate::createImportAutoComplete(Module* module) {
	Q_Q(Edit);

	QStandardItemModel* model = new QStandardItemModel(m_completer);

	QStringList importDirFilter;
	importDirFilter.append("*.jnc");
	importDirFilter.append("*.jncx");

	handle_t it = module->getImportDirIterator();
	while (it) {
		const char* dir = module->getNextImportDir(&it);
		QDirIterator dirIt(dir, importDirFilter);

		while (dirIt.hasNext()) {
			dirIt.next();
			addFile(model, dirIt.fileName());
		}
	}

	it = module->getExtensionSourceFileIterator();
	while (it) {
		const char* fileName = module->getNextExtensionSourceFile(&it);
		addFile(model, fileName);
	}

	ensureCompleter();

	model->setSortRole(Role_CaseInsensitiveSort);
	model->sort(0);

	m_completer->setModel(model);
	m_completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_completer->setWrapAround(false);
	m_completer->setCompletionPrefix(QString());

	m_completerRect = getLastCodeAssistCursorRect();
	updateCompleter(true);
}

void
EditPrivate::indentSelection() {
	Q_Q(Edit);

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
EditPrivate::unindentSelection() {
	Q_Q(Edit);

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
EditPrivate::keyPressControlSpace(QKeyEvent* e) {
	Q_Q(Edit);

	if (e->modifiers() & Qt::ShiftModifier) {
		if (m_codeAssistTriggers & Edit::ArgumentTipOnCtrlShiftSpace)
			requestCodeAssist(CodeAssistKind_ArgumentTip);
	} else {
		if (m_codeAssistTriggers & Edit::AutoCompleteOnCtrlSpace)
			requestCodeAssist(CodeAssistKind_AutoComplete);
	}
}

void
EditPrivate::keyPressHome(QKeyEvent* e) {
	Q_Q(Edit);

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

	if (isCursorAtStartOfLine(cursor)) {
		isNextWord = isCursorOnIndent(cursor);
	} else {
		bool wasOnIndent = isCursorOnIndent(cursor);
		cursor.movePosition(QTextCursor::StartOfLine, moveMode);
		isNextWord = !wasOnIndent && isCursorOnIndent(cursor);
	}

	if (isNextWord)
		cursor.movePosition(QTextCursor::NextWord, moveMode);

	q->setTextCursor(cursor);
}

void
EditPrivate::keyPressTab(QKeyEvent* e) {
	Q_Q(Edit);

	if (e->modifiers() & Qt::ShiftModifier) // unindent
		return keyPressBacktab(e);

	QTextCursor cursor = q->textCursor();
	if (isCursorMultiLineSelection(cursor))
		indentSelection();
	else
		q->QPlainTextEdit::keyPressEvent(e);
}

void
EditPrivate::keyPressBacktab(QKeyEvent* e) {
	Q_Q(Edit);

	QTextCursor cursor = q->textCursor();
	if (isCursorMultiLineSelection(cursor) || isCursorOnIndent(cursor))
		unindentSelection();
}

void
EditPrivate::keyPressEnter(QKeyEvent* e) {
	Q_Q(Edit);

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
EditPrivate::keyPressBackspace(QKeyEvent* e) {
	Q_Q(Edit);

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
EditPrivate::keyPressPrintChar(QKeyEvent* e) {
	Q_Q(Edit);

	QString text = e->text();
	QChar ch = text.isEmpty() ? QChar() : text.at(0);
	int c = ch.toLatin1();

	QTextCursor cursor = q->textCursor();
	bool isImportAutoComplete;

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

		if ((m_codeAssistTriggers & Edit::ArgumentTipOnTypeLeftParenthesis) && c == '(')
			requestCodeAssist(CodeAssistKind_ArgumentTip);

		break;

	case '.':
		q->QPlainTextEdit::keyPressEvent(e);

		if ((m_codeAssistTriggers & Edit::AutoCompleteOnTypeDot) && !hasCursorHighlightColor(cursor))
			requestCodeAssist(CodeAssistKind_AutoComplete);

		break;

	case ',':
		q->QPlainTextEdit::keyPressEvent(e);

		if ((m_codeAssistTriggers & Edit::ArgumentTipOnTypeComma) && !hasCursorHighlightColor(cursor))
			requestCodeAssist(CodeAssistKind_ArgumentTip);

		break;

	case '"':
		isImportAutoComplete =
			((m_codeAssistTriggers & Edit::ImportAutoCompleteOnTypeQuotationMark) &&
			getCursorLinePrefix(cursor).trimmed() == "import");

		q->QPlainTextEdit::keyPressEvent(e);

		if (isImportAutoComplete)
			requestCodeAssist(CodeAssistKind_AutoComplete);

		break;

	default:
		q->QPlainTextEdit::keyPressEvent(e);
	}
}

void
EditPrivate::timerEvent(QTimerEvent* e) {
	if (e->timerId() != m_quickInfoTipTimer.timerId())
		return;

	m_quickInfoTipTimer.stop();
	requestCodeAssist(CodeAssistKind_QuickInfoTip, m_pendingCodeAssistPosition);
}

void
EditPrivate::matchBraces() {
	Q_Q(Edit);

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
EditPrivate::onCursorPositionChanged() {
	switch (m_lastCodeAssistKind) {
	case CodeAssistKind_QuickInfoTip:
		hideCodeAssist();
		break;

	case CodeAssistKind_ArgumentTip:
		requestCodeAssist(CodeAssistKind_ArgumentTip);
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

Function*
EditPrivate::getPrototypeFunction(const QModelIndex& index) {
	ModuleItem* item = (ModuleItem*)m_completer->popup()->model()->data(index, Role_ModuleItem).value<void*>();
	if (!item || item->getItemKind() != ModuleItemKind_Function)
		return NULL;

	ModuleItemDecl* decl = item->getDecl();
	if (decl->getParentNamespace() != m_lastCodeAssistModule->getCodeAssist()->getNamespace())
		return NULL;

	AttributeBlock* block = decl->getAttributeBlock();
	return block && block->findAttribute("prototype") ? (Function*)item : NULL;
}

QString
getPrototypeDeclString(
	Function* function,
	bool isNextLineEmpty
) {
	FunctionType* type = function->getType();
	Type* returnType = type->getReturnType();
	size_t argCount = type->getArgCount();
	size_t lastArgIdx = argCount - 1;

	bool isMl = argCount >= 2;

	QString text = returnType->getTypeString();
	text += ' ';
	text += function->getDecl()->getQualifiedName();
	text += isMl ? "(\n\t" : "(";

	for (size_t i = 0; i < argCount; i++) {
		FunctionArg* arg = type->getArg(i);
		Type* argType = arg->getType();

		text += argType->getTypeStringPrefix();
		text += ' ';
		text += arg->getDecl()->getName();
		text += argType->getTypeStringSuffix();

		if (i != lastArgIdx)
			text += ",\n\t";
	}

	if (type->getFlags() & FunctionTypeFlag_VarArg)
		text += isMl ? ",\n\t..." : ", ...";

	if (isMl)
		text += "\n";

	text += ") {\n\t\n}";

	if (!isNextLineEmpty)
		text += '\n';

	return text;
}

void
EditPrivate::onCompleterActivated(const QModelIndex& index) {
	Q_Q(Edit);

    QTextCursor cursor = q->textCursor();

	Function* function = getPrototypeFunction(index);
	if (function && getCursorLineSuffix(cursor).trimmed().isEmpty()) {
		bool isNextLineEmpty = isCursorNextLineEmpty(cursor);
		QString completion = getPrototypeDeclString(function, isNextLineEmpty);
		cursor.select(QTextCursor::LineUnderCursor);
		cursor.insertText(completion);

		int delta = isNextLineEmpty ? 2 : 3; // inside body after \t
		cursor.setPosition(cursor.position() - delta);
	    q->setTextCursor(cursor);
		return;
	}

	QString completion = m_completer->popup()->model()->data(index, Qt::DisplayRole).toString();
	int basePosition = getLastCodeAssistPosition();

	if (m_lastCodeAssistKind == CodeAssistKind_ImportAutoComplete) {
		QString quotedCompletion = '"' + completion + '"';
		cursor.setPosition(basePosition);
		cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
		cursor.insertText(quotedCompletion);
	} else {
		cursor.setPosition(basePosition);

		QChar c = getCursorNextChar(cursor);
		if (c.isLetterOrNumber() || c == '_')
			cursor.select(QTextCursor::WordUnderCursor);

		cursor.insertText(completion);
	}

    q->setTextCursor(cursor);
}

void
EditPrivate::onCodeAssistReady() {
	Q_Q(Edit);

	CodeAssistThread* thread = (CodeAssistThread*)sender();
	ASSERT(thread);

	if (thread != m_thread)
		return;

	CodeAssist* codeAssist = thread->getModule()->getCodeAssist();
	if (!codeAssist) {
		if (thread->getCodeAssistKind() != CodeAssistKind_QuickInfoTip ||
			m_lastCodeAssistKind == CodeAssistKind_QuickInfoTip) // don't let failed quick-info to ruin existing code-assist
			hideCodeAssist();

		return;
	}

	m_lastCodeAssistModule = thread->getModule(); // cache
	m_lastCodeAssistKind = codeAssist->getCodeAssistKind();
	m_lastCodeAssistOffset = codeAssist->getOffset();
	m_lastCodeAssistPosition = -1;

	switch (m_lastCodeAssistKind) {
	case CodeAssistKind_QuickInfoTip:
		createQuickInfoTip(codeAssist->getModuleItem());
		break;

	case CodeAssistKind_ArgumentTip:
		createArgumentTip(codeAssist->getFunctionTypeOverload(), codeAssist->getArgumentIdx());
		break;

	case CodeAssistKind_AutoComplete:
		createAutoComplete(codeAssist->getNamespace(), codeAssist->getFlags());
		break;

	case CodeAssistKind_ImportAutoComplete:
		createImportAutoComplete(codeAssist->getModule());
		break;

	case CodeAssistKind_GotoDefinition:
		break;

	default:
		hideCodeAssist();
	}
}

void
EditPrivate::onThreadFinished() {
	CodeAssistThread* thread = (CodeAssistThread*)sender();
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
	if (index.column() != EditPrivate::Column_Synopsis) {
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
