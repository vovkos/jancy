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
	d->setupEditor();
	d->enableSyntaxHighlighting(true);
	d->enableLineNumberMargin(true);
	d->enableCurrentLineHighlighting(true);
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
	return d->enableCurrentLineHighlighting(true);
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
	return d->enableSyntaxHighlighting(true);
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
		EditPrivate::CodeAssistKind_QuickInfoTip,
		textCursor().position(),
		true
		);
}

void
Edit::argumentTip()
{
	Q_D(Edit);

	d->requestCodeAssist(
		EditPrivate::CodeAssistKind_ArgumentTip,
		textCursor().position(),
		true
		);
}

void
Edit::autoComplete()
{
	Q_D(Edit);

	d->requestCodeAssist(
		EditPrivate::CodeAssistKind_AutoComplete,
		textCursor().position(),
		true
		);
}

void
Edit::autoCompleteList()
{
	Q_D(Edit);

	d->requestCodeAssist(
		EditPrivate::CodeAssistKind_AutoCompleteList,
		textCursor().position(),
		true
		);
}

void
Edit::gotoDefinition()
{
	Q_D(Edit);

	d->requestCodeAssist(
		EditPrivate::CodeAssistKind_GotoDefinition,
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

	if (e->key() == Qt::Key_Space && (e->modifiers() & Qt::ControlModifier))
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
		return;
	}

	QPlainTextEdit::keyPressEvent(e);
}

void
Edit::keyReleaseEvent(QKeyEvent* e)
{
	Q_D(Edit);

	QPlainTextEdit::keyReleaseEvent(e); // default processing first

	QByteArray text = e->text().toLatin1();
	if (text.isEmpty())
		return;

	char c = text[0];
	switch (c)
	{
	case '.':
		if (d->m_codeAssistTriggers & AutoCompleteListOnTypeDot)
			d->requestCodeAssist(EditPrivate::CodeAssistKind_AutoCompleteList, textCursor().position());
		break;

	case '(':
		if (d->m_codeAssistTriggers & ArgumentTipOnTypeLeftParenthesis)
			d->requestCodeAssist(EditPrivate::CodeAssistKind_ArgumentTip, textCursor().position());
		break;

	case ',':
		if (d->m_codeAssistTriggers & ArgumentTipOnTypeComma)
			d->requestCodeAssist(EditPrivate::CodeAssistKind_ArgumentTip, textCursor().position());
		break;
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
EditPrivate::setupEditor()
{
	Q_Q(Edit);

#if (_JNC_OS_DARWIN)
	QFont font("Menlo", 11);
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
	CodeAssistKind codeAssistKind,
	int position,
	bool isSync
	)
{
	Q_Q(Edit);

	static const char* codeAssistKindString[] =
	{
		"QuickInfoTip",
		"ArgumentTip",
		"AutoComplete",
		"AutoCompleteList",
		"GotoDefinition",
	};

	printf("requestCodeAssist(%s, %d, %d)\n", codeAssistKindString[codeAssistKind], position, isSync);
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
EditPrivate::onCodeAssistReady()
{
	Q_Q(Edit);
}

//..............................................................................

} // namespace jnc
