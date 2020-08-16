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
#include "moc_jnc_Edit.cpp"
#include "moc_jnc_Edit_p.cpp"

namespace jnc {

//..............................................................................

Edit::Edit(QWidget *parent):
	QPlainTextEdit(parent),
	d_ptr(new EditPrivate)
{
	Q_D(Edit);

	d->q_ptr = this;
	d->init();
}

Edit::~Edit()
{
}

bool
Edit::isLineNumberMarginEnabled()
{
	Q_D(Edit);
	return d->m_lineNumberMargin != NULL;
}

void
Edit::enableLineNumberMargin(bool isEnabled)
{
	Q_D(Edit);
	d->enableLineNumberMargin(isEnabled);
}

bool
Edit::isCurrentLineHighlightingEnabled()
{
	Q_D(Edit);
	return d->m_isCurrentLineHighlightingEnabled;
}

void
Edit::enableCurrentLineHighlighting(bool isEnabled)
{
	Q_D(Edit);
	return d->enableCurrentLineHighlighting(isEnabled);
}

bool
Edit::isSyntaxHighlightingEnabled()
{
	Q_D(Edit);
	return d->m_syntaxHighlighter != NULL;
}

void
Edit::enableSyntaxHighlighting(bool isEnabled)
{
	Q_D(Edit);
	return d->enableSyntaxHighlighting(isEnabled);
}

Edit::CodeAssistTriggers
Edit::codeAssistTriggers()
{
	Q_D(Edit);
	return d->m_codeAssistTriggers;
}

void
Edit::setCodeAssistTriggers(CodeAssistTriggers triggers)
{
	Q_D(Edit);
	d->m_codeAssistTriggers = triggers;
}

QStringList
Edit::importDirList()
{
	Q_D(Edit);
	return d->m_importDirList;
}

void
Edit::setImportDirList(const QStringList& dirList)
{
	Q_D(Edit);
	d->m_importDirList = dirList;
}

void
Edit::setTextCursorLineCol(
	int line,
	int col
	)
{
	QTextCursor cursor = textCursor();
	cursor.setPosition(0);
	cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line);
	cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, col);
	setTextCursor(cursor);
}

void
Edit::quickInfoTip()
{
	Q_D(Edit);
	d->requestCodeAssist(CodeAssistKind_QuickInfoTip, true);
}

void
Edit::argumentTip()
{
	Q_D(Edit);
	d->requestCodeAssist(CodeAssistKind_ArgumentTip, true);
}

void
Edit::autoComplete()
{
	Q_D(Edit);
	d->requestCodeAssist(CodeAssistKind_AutoComplete, true);
}

void
Edit::autoCompleteList()
{
	Q_D(Edit);
	d->requestCodeAssist(CodeAssistKind_AutoCompleteList, true);
}

void
Edit::gotoDefinition()
{
	Q_D(Edit);
	d->requestCodeAssist(CodeAssistKind_GotoDefinition, true);
}

void
Edit::resizeEvent(QResizeEvent *e)
{
	Q_D(Edit);

	QPlainTextEdit::resizeEvent(e);

	if (d->m_lineNumberMargin)
		d->updateLineNumberMarginGeometry();
}

void
Edit::keyPressEvent(QKeyEvent* e)
{
	Q_D(Edit);

	int key = e->key();
	QString text = e->text();
	QChar ch = text.isEmpty() ? QChar() : text.at(0);

	if (d->isCompleterVisible())
	{
		switch (key)
		{
		case Qt::Key_Enter:
		case Qt::Key_Return:
		case Qt::Key_Escape:
		case Qt::Key_Tab:
		case Qt::Key_Backtab:
			e->ignore();
			return; // let the completer do default behavior
		}

		if (key == Qt::Key_Backspace || ch.isNull() || ch.isLetterOrNumber())
		{
			QPlainTextEdit::keyPressEvent(e);
			d->updateCompleter();
		}
		else
		{
			d->applyCompleter();
			QPlainTextEdit::keyPressEvent(e);
		}
	}
	else if (key == Qt::Key_Space && (e->modifiers() & Qt::ControlModifier))
	{
		if (e->modifiers() & Qt::ShiftModifier)
		{
			if (d->m_codeAssistTriggers & ArgumentTipOnCtrlShiftSpace)
				argumentTip();
		}
		else
		{
			if (d->m_codeAssistTriggers & AutoCompleteOnCtrlSpace)
				autoComplete();
		}

		e->accept();
	}
	else
	{
		QPlainTextEdit::keyPressEvent(e);

		char c = ch.toLatin1();
		switch (c)
		{
		case '.':
			if (d->m_codeAssistTriggers & AutoCompleteListOnTypeDot)
				d->requestCodeAssist(CodeAssistKind_AutoCompleteList);
			break;

		case '(':
			if (d->m_codeAssistTriggers & ArgumentTipOnTypeLeftParenthesis)
				d->requestCodeAssist(CodeAssistKind_ArgumentTip);
			break;

		case ',':
			if (d->m_codeAssistTriggers & ArgumentTipOnTypeComma)
				d->requestCodeAssist(CodeAssistKind_ArgumentTip);
			break;
		}
	}
}


void
Edit::mousePressEvent(QMouseEvent* e)
{
	Q_D(Edit);

	// check for triggers first

	QPlainTextEdit::mousePressEvent(e);
}

void
Edit::mouseMoveEvent(QMouseEvent* e)
{
	Q_D(Edit);

	QPlainTextEdit::mouseMoveEvent(e);

	// check for triggers
}

//..............................................................................

EditPrivate::EditPrivate()
{
    q_ptr = NULL;
	m_syntaxHighlighter = NULL;
	m_lineNumberMargin = NULL;
	m_isCurrentLineHighlightingEnabled = false;
	m_thread = NULL;
	m_completer = NULL;

	m_codeAssistTriggers =
		Edit::QuickInfoTipOnHoverOverIdentifier |
		Edit::ArgumentTipOnCtrlShiftSpace |
		Edit::ArgumentTipOnTypeLeftParenthesis |
		Edit::ArgumentTipOnTypeComma |
		Edit::AutoCompleteOnCtrlSpace |
		Edit::AutoCompleteListOnTypeDot |
		Edit::AutoCompleteListOnTypeIdentifier |
		Edit::GotoDefinitionOnCtrlClick;
}

void
EditPrivate::init()
{
	Q_Q(Edit);

#if (_JNC_OS_DARWIN)
	QFont font("Menlo", 11);
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
	q->setTabStopWidth(q_ptr->fontMetrics().width(' ') * 4);
	q->setWordWrapMode(QTextOption::NoWrap);

	enableSyntaxHighlighting(true);
	enableLineNumberMargin(true);
	enableCurrentLineHighlighting(true);
}

void
EditPrivate::enableSyntaxHighlighting(bool isEnabled)
{
	Q_Q(Edit);

	if (isEnabled)
	{
		if (!m_syntaxHighlighter)
			m_syntaxHighlighter = new JancyHighlighter(q->document());
	}
	else if (m_syntaxHighlighter)
	{
		m_syntaxHighlighter->setDocument(NULL);
		delete m_syntaxHighlighter;
		m_syntaxHighlighter = NULL;
	}
}

void
EditPrivate::enableLineNumberMargin(bool isEnabled)
{
	Q_Q(Edit);

	if (isEnabled)
	{
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
	}
	else
	{
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
EditPrivate::enableCurrentLineHighlighting(bool isEnabled)
{
	Q_Q(Edit);

	if (isEnabled)
	{
		QObject::connect(
			q, SIGNAL(cursorPositionChanged()),
			this, SLOT(highlightCurrentLine())
			);

		highlightCurrentLine();
	}
	else
	{
		QObject::disconnect(
			q, SIGNAL(cursorPositionChanged()),
			this, SLOT(highlightCurrentLine())
			);

		q->setExtraSelections(QList<QTextEdit::ExtraSelection>());
	}
}

void
EditPrivate::updateLineNumberMarginGeometry()
{
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
EditPrivate::requestCodeAssist(
	CodeAssistKind kind,
	bool isSync
	)
{
	Q_Q(Edit);

	QTextCursor cursor = q->textCursor();
	requestCodeAssist(kind, getLineColFromCursor(cursor), isSync);
}

void
EditPrivate::requestCodeAssist(
	CodeAssistKind kind,
	const lex::LineCol& pos,
	bool isSync
	)
{
	Q_Q(Edit);

	if (m_thread)
		m_thread->cancel();

	m_thread = new CodeAssistThread(this);

	QObject::connect(
		m_thread, SIGNAL(ready()),
		this, SLOT(onCodeAssistReady())
		);

	QObject::connect(
		m_thread, SIGNAL(finished()),
		this, SLOT(onThreadFinished())
		);

	m_thread->request(kind, q->toPlainText(), pos);
}

void
EditPrivate::updateLineNumberMargin(
	const QRect& rect,
	int dy
	)
{
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
EditPrivate::highlightCurrentLine()
{
	Q_Q(Edit);

	QTextEdit::ExtraSelection selection;
	selection.cursor = q->textCursor();
	selection.cursor.clearSelection();
	selection.format.setBackground(QColor(Color_CurrentLineBack));
	selection.format.setProperty(QTextFormat::FullWidthSelection, true);

	QList<QTextEdit::ExtraSelection> extraSelections;
	extraSelections.append(selection);
	q->setExtraSelections(extraSelections);
}

void
EditPrivate::ensureCompleter()
{
	if (m_completer)
		return;

	Q_Q(Edit);

	m_completer = new QCompleter(q);
	m_completer->setWidget(q);
	m_completer->setCompletionMode(QCompleter::PopupCompletion);
	m_completer->setMaxVisibleItems(10);
	m_completer->popup()->setFont(q->font());

    QObject::connect(
		m_completer, SIGNAL(activated(const QString&)),
		this, SLOT(onCompleterActivated(const QString&))
		);
}

void
EditPrivate::updateCompleter(bool isForced)
{
	ASSERT(m_completer);

	Q_Q(Edit);

	QTextCursor cursor = q->textCursor();
	cursor.select(QTextCursor::WordUnderCursor);

	QString prefix = cursor.selectedText();
	if (!isForced && prefix == m_completer->completionPrefix())
		return;

	QAbstractItemView* popup = m_completer->popup();
	m_completer->setCompletionPrefix(prefix);
	popup->setCurrentIndex(m_completer->completionModel()->index(0, 0));

    m_completerRect.setWidth(
		popup->sizeHintForColumn(0) +
		popup->verticalScrollBar()->sizeHint().width()
		);

	m_completer->complete(m_completerRect);
}

void
EditPrivate::applyCompleter()
{
	ASSERT(m_completer);

	QModelIndex index = m_completer->popup()->currentIndex();
	QString completion = index.isValid() ? m_completer->popup()->model()->data(index).toString() : QString();
	if (!completion.isEmpty())
		onCompleterActivated(completion);

	hideCompleter();
}

lex::LineCol
EditPrivate::getLineColFromCursor(const QTextCursor& cursor0)
{
	QTextCursor cursor = cursor0;
	cursor.movePosition(QTextCursor::StartOfLine);

	int line = 0;
	while (cursor.positionInBlock() > 0)
	{
		line++;
		cursor.movePosition(QTextCursor::Up);
	}

	QTextBlock block = cursor.block().previous();
	while (block.isValid())
	{
		line += block.lineCount();
		block = block.previous();
	}

	return lex::LineCol(line, cursor0.columnNumber());
}

QRect
EditPrivate::getCursorRectFromLineCol(const lex::LineCol& pos)
{
	Q_Q(Edit);

	QTextCursor cursor = q->textCursor();
	cursor.setPosition(0);
	cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, pos.m_line);
	cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos.m_col);

	QRect rect = q->cursorRect(cursor);
	rect.translate(q->viewportMargins().left(), q->viewportMargins().top());
	return rect;
}

QPoint
EditPrivate::getToolTipPointFromLineCol(const lex::LineCol& pos)
{
	Q_Q(Edit);

	enum
	{
		QtToolTipOffset_X = 2,
		QtToolTipOffset_Y = 16,
	};

	QRect rect = getCursorRectFromLineCol(pos);
	QPoint point = q->mapToGlobal(rect.bottomLeft());
	point -= QPoint(QtToolTipOffset_X, QtToolTipOffset_Y);
	return point;
}

void
EditPrivate::createQuickInfoTip(
	const lex::LineCol& pos,
	ModuleItem* item
	)
{
	Q_Q(Edit);

	QPoint point = getToolTipPointFromLineCol(pos);
	QToolTip::showText(point, item->getDecl()->getName(), q);
}

void
EditPrivate::createArgumentTip(
	const lex::LineCol& pos,
	FunctionType* functionType,
	size_t argumentIdx
	)
{
	Q_Q(Edit);

	QString argTypeString = "(";

	size_t argCount = functionType->getArgCount();
	size_t lastArgIdx = argCount - 1;
	for (size_t i = 0; i < argCount; i++)
	{
		FunctionArg* arg = functionType->getArg(i);
		Type* argType = arg->getType();

		if (i == argumentIdx)
			argTypeString += "<b>";

		argTypeString += argType->getTypeStringPrefix();
		argTypeString += ' ';
		argTypeString += arg->getDecl()->getName();
		argTypeString += argType->getTypeStringSuffix();

		if (i == argumentIdx)
			argTypeString += "</b>";

		if (i != lastArgIdx)
			argTypeString += ", ";
	}

	argTypeString += ")";

	functionType->getTypeStringSuffix();
	QPoint point = getToolTipPointFromLineCol(pos);
	QToolTip::showText(point, argTypeString, q);
}

void
EditPrivate::createAutoCompleteList(
	const lex::LineCol& pos,
	Namespace* nspace,
	uint_t flags
	)
{
	Q_Q(Edit);

	QStringList list;

	size_t count = nspace->getItemCount();
	for (size_t i = 0; i < count; i++)
	{
		ModuleItem* item = nspace->getItem(i);
		const char* name = item->getDecl()->getName();
		printf("[%d]\t%s\n", i, name);
		list.append(name);
	}

	ensureCompleter();

	list.sort(Qt::CaseInsensitive);

	m_completer->setModel(new QStringListModel(list, m_completer));
	m_completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_completer->setWrapAround(false);
	m_completer->setCompletionPrefix(QString());

	m_completerRect = getCursorRectFromLineCol(pos);
	updateCompleter(true);
}

void
EditPrivate::onCompleterActivated(const QString &completion)
{
	Q_Q(Edit);

	if (m_completer->widget() != q)
        return;

    QTextCursor cursor = q->textCursor();
    int extra = completion.length() - m_completer->completionPrefix().length();
    cursor.movePosition(QTextCursor::Left);
    cursor.movePosition(QTextCursor::EndOfWord);
    cursor.insertText(completion.right(extra));
    q->setTextCursor(cursor);
}

void
EditPrivate::onCodeAssistReady()
{
	Q_Q(Edit);

	CodeAssistThread* thread = (CodeAssistThread*)sender();
	ASSERT(thread);

	CodeAssist* codeAssist = thread->getCodeAssist();
	if (!codeAssist)
		return;

	CodeAssistKind kind = codeAssist->getCodeAssistKind();
	lex::LineCol pos(codeAssist->getLine(), codeAssist->getCol());

	switch (kind)
	{
	case CodeAssistKind_QuickInfoTip:
		createQuickInfoTip(pos, codeAssist->getModuleItem());
		break;

	case CodeAssistKind_ArgumentTip:
		createArgumentTip(pos, codeAssist->getFunctionType(), codeAssist->getArgumentIdx());
		break;

	case CodeAssistKind_AutoComplete:
		break;

	case CodeAssistKind_AutoCompleteList:
		createAutoCompleteList(pos, codeAssist->getNamespace(), codeAssist->getNamespaceFlags());
		break;

	case CodeAssistKind_GotoDefinition:
		break;
	}
}

void
EditPrivate::onThreadFinished()
{
	CodeAssistThread* thread = (CodeAssistThread*)sender();
	ASSERT(thread);

	if (thread == m_thread)
		m_thread = NULL;

	thread->deleteLater();
}

//..............................................................................

} // namespace jnc
