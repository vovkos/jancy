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

	d->requestCodeAssist(
		CodeAssistKind_QuickInfoTip,
		textCursor().position(),
		true
		);
}

void
Edit::argumentTip()
{
	Q_D(Edit);

	d->requestCodeAssist(
		CodeAssistKind_ArgumentTip,
		textCursor().position(),
		true
		);
}

void
Edit::autoComplete()
{
	Q_D(Edit);

	d->requestCodeAssist(
		CodeAssistKind_AutoComplete,
		textCursor().position(),
		true
		);
}

void
Edit::autoCompleteList()
{
	Q_D(Edit);

	d->requestCodeAssist(
		CodeAssistKind_AutoCompleteList,
		textCursor().position(),
		true
		);
}

void
Edit::gotoDefinition()
{
	Q_D(Edit);

	d->requestCodeAssist(
		CodeAssistKind_GotoDefinition,
		textCursor().position(),
		true
		);
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
				d->requestCodeAssist(CodeAssistKind_AutoCompleteList, textCursor().position());
			break;

		case '(':
			if (d->m_codeAssistTriggers & ArgumentTipOnTypeLeftParenthesis)
				d->requestCodeAssist(CodeAssistKind_ArgumentTip, textCursor().position());
			break;

		case ',':
			if (d->m_codeAssistTriggers & ArgumentTipOnTypeComma)
				d->requestCodeAssist(CodeAssistKind_ArgumentTip, textCursor().position());
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
	int position,
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

	m_thread->request(kind, q->toPlainText(), position);
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

void
EditPrivate::createQuickInfoTip(
	const lex::LineColOffset& pos,
	ModuleItem* item
	)
{
	Q_Q(Edit);

	QPoint point = q->cursorRect().topLeft();
	point = q->mapToGlobal(point);
	point += QPoint(q->viewportMargins().left(), 0);

	QToolTip::showText(point, item->getDecl()->getName(), q);
}

void
EditPrivate::createArgumentTip(
	const lex::LineColOffset& pos,
	Function* function,
	size_t argumentIdx
	)
{
	Q_Q(Edit);

	QPoint point = q->cursorRect().topLeft();
	point = q->mapToGlobal(point);
	point += QPoint(q->viewportMargins().left(), 0);

	QToolTip::showText(point, function->getDecl()->getName(), q);
}

void
EditPrivate::createAutoCompleteList(
	const lex::LineColOffset& pos,
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

	m_completerRect = q->cursorRect();
	m_completerRect.translate(q->viewportMargins().left(), 0);

	updateCompleter(true);
}

void
EditPrivate::onCompleterActivated(const QString &completion)
{
	Q_Q(Edit);

	if (m_completer->widget() != q)
        return;

    QTextCursor tc = q->textCursor();
    int extra = completion.length() - m_completer->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    q->setTextCursor(tc);
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

	lex::LineColOffset pos;
	pos.m_line = codeAssist->getLine();
	pos.m_col = codeAssist->getCol();
	pos.m_offset = codeAssist->getOffset();

	switch (kind)
	{
	case CodeAssistKind_QuickInfoTip:
		createQuickInfoTip(pos, codeAssist->getModuleItem());
		break;

	case CodeAssistKind_ArgumentTip:
		createArgumentTip(pos, codeAssist->getFunction(), codeAssist->getArgumentIdx());
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
