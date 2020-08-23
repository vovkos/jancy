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
#include "qrc_res.cpp"

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
	Q_D(Edit);
	setTextCursor(d->getCursorFromLineCol(lex::LineCol(line, col)));
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

		case Qt::Key_Space:
			if (e->modifiers() & Qt::ControlModifier)
			{
				d->keyPressCtrlSpace(e);
				return;
			}
		}

		if (ch.isPrint() && !ch.isLetterOrNumber() && ch != '_')
			d->applyCompleter();

		QPlainTextEdit::keyPressEvent(e);
	}
	else switch (key)
	{
		char c;

	case Qt::Key_Escape:
		QPlainTextEdit::keyPressEvent(e);
		d->hideCodeAssist();
		break;

	case Qt::Key_Space:
		if (e->modifiers() & Qt::ControlModifier)
			d->keyPressCtrlSpace(e);
		else
			QPlainTextEdit::keyPressEvent(e);

		break;

	default:
		QPlainTextEdit::keyPressEvent(e);

		c = ch.toLatin1();
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

	if (!d->isCompleterVisible() &&
		(d->m_codeAssistTriggers & QuickInfoTipOnMouseOverIdentifier))
		d->requestQuickInfoTip(e->pos());
}

void
Edit::enterEvent(QEvent* e)
{
	Q_D(Edit);

	QPlainTextEdit::enterEvent(e);

	if (!d->isCompleterVisible() &&
		d->m_lastCodeAssistKind == CodeAssistKind_QuickInfoTip &&
		(d->m_codeAssistTriggers & QuickInfoTipOnMouseOverIdentifier))
	{
		QPoint pos = mapFromGlobal(QCursor::pos());
		d->requestQuickInfoTip(pos);
	}
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
	m_lastCodeAssistKind = CodeAssistKind_Undefined;
	m_lastCodeAssistPosition = -1;
	m_pendingCodeAssistPosition = -1;

	m_codeAssistTriggers =
		Edit::QuickInfoTipOnMouseOverIdentifier |
		Edit::ArgumentTipOnCtrlShiftSpace |
		Edit::ArgumentTipOnTypeLeftParenthesis |
		Edit::ArgumentTipOnTypeComma |
		Edit::AutoCompleteOnCtrlSpace |
		Edit::AutoCompleteListOnTypeDot |
		Edit::AutoCompleteListOnTypeIdentifier |
		Edit::GotoDefinitionOnCtrlClick;
}

QImage createSubImage(QImage* image, const QRect & rect) {
    size_t offset = rect.x() * image->depth() / 8
                    + rect.y() * image->bytesPerLine();
    return QImage(image->bits() + offset, rect.width(), rect.height(),
                  image->bytesPerLine(), image->format());
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
	q->setMouseTracking(true);

	enableSyntaxHighlighting(true);
	enableLineNumberMargin(true);
	enableCurrentLineHighlighting(true);

	QObject::connect(
		q, SIGNAL(cursorPositionChanged()),
		this, SLOT(onCursorPositionChanged())
		);

	static const size_t iconIdxTable[] =
	{
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

	if (isEnabled == m_isCurrentLineHighlightingEnabled)
		return;

	if (isEnabled)
		highlightCurrentLine();
	else
		q->setExtraSelections(QList<QTextEdit::ExtraSelection>());

	m_isCurrentLineHighlightingEnabled = isEnabled;
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
	requestCodeAssist(kind, cursor.position(), isSync);
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
	m_thread->m_importDirList = m_importDirList;

	QObject::connect(
		m_thread, SIGNAL(ready()),
		this, SLOT(onCodeAssistReady())
		);

	QObject::connect(
		m_thread, SIGNAL(finished()),
		this, SLOT(onThreadFinished())
		);

	m_thread->request(kind, ref::g_nullPtr, position, q->toPlainText());
}

void
EditPrivate::requestQuickInfoTip(const QPoint& pos)
{
	Q_Q(Edit);

	QTextCursor cursor = q->cursorForPosition(pos);
	m_pendingCodeAssistPosition = cursor.position();
	m_quickInfoTipTimer.start(Timeout_QuickInfo, this);
}

void
EditPrivate::hideCodeAssist()
{
	if (m_completer)
		m_completer->popup()->hide();

	QToolTip::hideText();

	m_lastCodeAssistKind = CodeAssistKind_Undefined;
	m_lastCodeAssistPosition = -1;
	m_thread = NULL;
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
	lex::LineCol pos = getLineColFromCursor(cursor);
	int position = cursor.position();
	int anchorPosition = getLastCodeAssistPosition();
	if (position < anchorPosition)
	{
		hideCodeAssist();
		return;
	}

	cursor.setPosition(position, QTextCursor::MoveAnchor);
	cursor.setPosition(anchorPosition, QTextCursor::KeepAnchor);
	QString prefix = cursor.selectedText();

	if (m_lastCodeAssistKind == CodeAssistKind_ImportAutoCompleteList)
		prefix.remove(0, 1);

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
	Q_Q(Edit);

	ASSERT(m_completer);

	QModelIndex index = m_completer->popup()->currentIndex();
	QString completion = index.isValid() ? m_completer->popup()->model()->data(index).toString() : QString();
	if (!completion.isEmpty())
		onCompleterActivated(completion);

	hideCodeAssist();
}

QTextCursor
EditPrivate::getCursorFromLineCol(const lex::LineCol& pos)
{
	Q_Q(Edit);

	QTextCursor cursor = q->textCursor();
	cursor.setPosition(0);
	cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, pos.m_line);
	cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos.m_col);
	return cursor;
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

int
EditPrivate::calcLastCodeAssistPosition()
{
	ASSERT(m_lastCodeAssistKind && m_lastCodeAssistPosition == -1);

	QTextCursor cursor = getCursorFromLineCol(m_lastCodeAssistLineCol);
	m_lastCodeAssistPosition = cursor.position();
	return m_lastCodeAssistPosition;
}

QRect
EditPrivate::getCursorRectFromLineCol(const lex::LineCol& pos)
{
	Q_Q(Edit);

	QTextCursor cursor = getCursorFromLineCol(pos);
	QRect rect = q->cursorRect(cursor);

#if (QT_VERSION >= 0x050500)
	rect.translate(q->viewportMargins().left(), q->viewportMargins().top());
#else
	rect.translate(m_lineNumberMargin ? m_lineNumberMargin->width() : 0, 0);
#endif

	return rect;
}

QPoint
EditPrivate::getToolTipPointFromLineCol(
	const lex::LineCol& pos,
	bool isBelowCurrentCursor
	)
{
	Q_Q(Edit);

	enum
	{
		QtToolTipOffset_X = 2,
		QtToolTipOffset_Y = 16,
	};

	QRect rect = getCursorRectFromLineCol(pos);

	if (isBelowCurrentCursor)
		rect.moveTop(q->cursorRect().top());

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
	ModuleItemDecl* decl = item->getDecl();
	Type* type = item->getType();
	QString prefix;
	QString suffix;
	QString text;

	ModuleItemKind itemKind = item->getItemKind();
	switch (itemKind)
	{
	case ModuleItemKind_Typedef:
		prefix = "typedef ";
		break;

	case ModuleItemKind_Namespace:
		text = QString("namespace ") + decl->getQualifiedName();
		break;

	case ModuleItemKind_Field:
	case ModuleItemKind_Variable:
	case ModuleItemKind_Function:
	case ModuleItemKind_Property:
		type = item->getType();
		break;

	case ModuleItemKind_Type:
		text = type->getTypeString();
		break;
	}

	if (text.isEmpty())
	{
		if (type)
		{
			prefix += type->getTypeStringPrefix();
			suffix = type->getTypeStringSuffix();
		}

		text = prefix + ' ' + decl->getQualifiedName() + suffix;
	}

	QToolTip::showText(point, text, q);
}

void
EditPrivate::createArgumentTip(
	const lex::LineCol& pos,
	FunctionType* functionType,
	size_t argumentIdx
	)
{
	Q_Q(Edit);

	#define ML_ARG_INDENT "&nbsp;&nbsp;&nbsp;&nbsp;"

	Type* returnType = functionType->getReturnType();
	size_t argCount = functionType->getArgCount();
	size_t lastArgIdx = argCount - 1;

	QString text = returnType->getTypeString();
	if (argCount >= 2)
		text += " (<br>" ML_ARG_INDENT;
	else
		text += " (";

	for (size_t i = 0; i < argCount; i++)
	{
		FunctionArg* arg = functionType->getArg(i);
		Type* argType = arg->getType();

		if (i == argumentIdx)
			text += "<b>";

		text += argType->getTypeStringPrefix();
		text += ' ';
		text += arg->getDecl()->getName();
		text += argType->getTypeStringSuffix();

		if (i == argumentIdx)
			text += "</b>";

		if (i != lastArgIdx)
			text += ",<br>" ML_ARG_INDENT;
	}

	if (argCount >= 2)
		text += "<br>" ML_ARG_INDENT ")";
	else
		text += ")";

	QPoint point = getToolTipPointFromLineCol(pos, true);
	if (point != m_lastToolTipPoint)
	{
		QToolTip::showText(point, " ", q); // ensure tipChanged
		m_lastToolTipPoint = point;
	}

	QToolTip::showText(point, text, q);
}

size_t
EditPrivate::getItemIconIdx(ModuleItem* item)
{
	static const size_t iconIdxTable[ModuleItemKind__Count] =
	{
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
	)
{
	size_t count = nspace->getItemCount();
	for (size_t i = 0; i < count; i++)
	{
		ModuleItem* item = nspace->getItem(i);
		QString name = item->getDecl()->getName();

		QStandardItem* qtItem = new QStandardItem;
		qtItem->setText(name);
		qtItem->setData(name.toLower(), Role_CaseInsensitiveSort);

		size_t iconIdx = getItemIconIdx(item);
		if (iconIdx != -1)
			qtItem->setIcon(m_iconTable[iconIdx]);

		model->appendRow(qtItem);
	}
}

void
EditPrivate::createAutoCompleteList(
	const lex::LineCol& pos,
	Namespace* nspace,
	uint_t flags
	)
{
	Q_Q(Edit);

	QStandardItemModel* model = new QStandardItemModel(m_completer);
	addAutoCompleteNamespace(model, nspace);

	NamespaceKind namespaceKind = nspace->getNamespaceKind();
	if (namespaceKind == NamespaceKind_Type)
	{
		NamedType* namedType = (NamedType*)nspace->getParentItem();
		if (namedType->getTypeKindFlags() & TypeKindFlag_Derivable)
		{
			DerivableType* derivableType = (DerivableType*)namedType;
			size_t count = derivableType->getBaseTypeCount();
			for (size_t i = 0; i < count; i++)
			{
				BaseTypeSlot* slot = derivableType->getBaseType(i);
				addAutoCompleteNamespace(model, slot->getType()->getNamespace());
			}
		}
	}

	if (flags & CodeAssistNamespaceFlag_IncludeParentNamespace)
	{
		nspace = nspace->getParentNamespace();

		while (nspace)
		{
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

	m_completerRect = getCursorRectFromLineCol(pos);
	updateCompleter(true);
}

void
EditPrivate::createImportAutoCompleteList(
	const lex::LineCol& pos,
	Module* module
	)
{
	Q_Q(Edit);

	QStandardItemModel* model = new QStandardItemModel(m_completer);

	handle_t it = module->getImportDirIterator();
	while (it)
	{
		const char* dir = module->getNextImportDir(&it);
		QStandardItem* item = new QStandardItem(dir);
		model->appendRow(item);
	}

	it = module->getExtensionSourceFileIterator();
	while (it)
	{
		const char* fileName = module->getNextExtensionSourceFile(&it);
		QStandardItem* item = new QStandardItem(fileName);
		model->appendRow(item);
	}

	ensureCompleter();

	model->setSortRole(Role_CaseInsensitiveSort);
	model->sort(0);

	m_completer->setModel(model);
	m_completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_completer->setWrapAround(false);
	m_completer->setCompletionPrefix(QString());

	m_completerRect = getCursorRectFromLineCol(pos);
	updateCompleter(true);
}

void
EditPrivate::keyPressCtrlSpace(QKeyEvent* e)
{
	if (e->modifiers() & Qt::ShiftModifier)
	{
		if (m_codeAssistTriggers & Edit::ArgumentTipOnCtrlShiftSpace)
			requestCodeAssist(CodeAssistKind_ArgumentTip, true);
	}
	else
	{
		if (m_codeAssistTriggers & Edit::AutoCompleteOnCtrlSpace)
			requestCodeAssist(CodeAssistKind_AutoComplete, true);
	}

	e->accept();
}

void
EditPrivate::timerEvent(QTimerEvent* e)
{
	if (e->timerId() != m_quickInfoTipTimer.timerId())
		return;

	m_quickInfoTipTimer.stop();
	requestCodeAssist(CodeAssistKind_QuickInfoTip, m_pendingCodeAssistPosition);
}

void
EditPrivate::onCursorPositionChanged()
{
	if (m_isCurrentLineHighlightingEnabled)
		highlightCurrentLine();

	switch (m_lastCodeAssistKind)
	{
	case CodeAssistKind_QuickInfoTip:
		hideCodeAssist();
		break;

	case CodeAssistKind_ArgumentTip:
		requestCodeAssist(CodeAssistKind_ArgumentTip);
		break;

	case CodeAssistKind_AutoCompleteList:
		if (isCompleterVisible())
			updateCompleter();
		break;
	}
}

void
EditPrivate::onCompleterActivated(const QString &completion)
{
	Q_Q(Edit);

    QTextCursor cursor = q->textCursor();
	cursor.select(QTextCursor::WordUnderCursor);

	int basePosition = getLastCodeAssistPosition();
	int position = cursor.position();
	int anchor = cursor.anchor();

	if (anchor < basePosition)
		cursor.setPosition(basePosition, QTextCursor::MoveAnchor);

	if (position < basePosition)
		cursor.setPosition(basePosition, QTextCursor::KeepAnchor);

    cursor.insertText(completion);
    q->setTextCursor(cursor);
}

void
EditPrivate::onCodeAssistReady()
{
	Q_Q(Edit);

	CodeAssistThread* thread = (CodeAssistThread*)sender();
	ASSERT(thread);

	if (thread != m_thread)
		return;

	CodeAssist* codeAssist = thread->getCodeAssist();
	if (!codeAssist)
	{
		if (thread->getCodeAssistKind() != CodeAssistKind_QuickInfoTip ||
			m_lastCodeAssistKind == CodeAssistKind_QuickInfoTip) // don't let failed quick-info to ruin existing code-assist
			hideCodeAssist();

		return;
	}

	CodeAssistKind kind = codeAssist->getCodeAssistKind();
	lex::LineCol pos(codeAssist->getLine(), codeAssist->getCol());

	m_lastCodeAssistKind = kind;
	m_lastCodeAssistLineCol = pos;
	m_lastCodeAssistPosition = -1;

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

	case CodeAssistKind_ImportAutoCompleteList:
		createImportAutoCompleteList(pos, codeAssist->getModule());
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
