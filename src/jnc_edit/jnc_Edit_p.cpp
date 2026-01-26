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
#include "jnc_CursorUtils.h"
#include "qrc_res.cpp"

namespace jnc {

//..............................................................................

Edit::Edit(QWidget *parent):
	EditBase(parent, new EditPrivate) {
	Q_D(Edit);
	d->q_ptr = this;
	d->m_syntaxHighlighter = createSyntaxHighlighter();
	d->init();
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
Edit::quickInfoTip() {
	Q_D(Edit);
	d->requestCodeAssist(0, CodeAssistKind_QuickInfoTip);
}

void
Edit::argumentTip() {
	Q_D(Edit);
	d->requestCodeAssist(0, CodeAssistKind_ArgumentTip);
}

void
Edit::autoComplete() {
	Q_D(Edit);
	d->requestCodeAssist(0, CodeAssistKind_AutoComplete);
}

void
Edit::gotoDefinition() {
	Q_D(Edit);
	d->requestCodeAssist(0, CodeAssistKind_GotoDefinition);
}

HighlighterBase*
Edit::createSyntaxHighlighter() {
 	Q_D(Edit);
	return new JancyHighlighter(document(), &d->m_theme);
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
		d->requestQuickInfoTip(EditPrivate::CodeAssistDelay_QuickInfoTip, e->pos());
}

void
Edit::enterEvent(QEvent* e) {
	Q_D(Edit);

	QPlainTextEdit::enterEvent(e);

	if (!d->isCompleterVisible() &&
		d->m_lastCodeAssistKind == CodeAssistKind_QuickInfoTip &&
		(d->m_codeAssistTriggers & QuickInfoTipOnMouseOverIdentifier)) {
		QPoint pos = mapFromGlobal(QCursor::pos());
		d->requestQuickInfoTip(EditPrivate::CodeAssistDelay_QuickInfoTip, pos);
	}
}


//..............................................................................

EditPrivate::EditPrivate() {
	m_thread = NULL;
	m_codeTip = NULL;
	m_completer = NULL;
	m_lastCodeAssistKind = CodeAssistKind_Undefined;
	m_lastCodeAssistOffset = -1;
	m_lastCodeAssistPosition = -1;
	m_pendingCodeAssistKind = CodeAssistKind_Undefined;
	m_pendingCodeAssistPosition = -1;

	m_codeAssistTriggers =
		Edit::QuickInfoTipOnMouseOverIdentifier |
		Edit::ArgumentTipOnCtrlShiftSpace |
		Edit::ArgumentTipOnTypeLeftParenthesis |
		Edit::ArgumentTipOnTypeComma |
		Edit::AutoCompleteOnCtrlSpace |
		Edit::AutoCompleteOnTypeDot |
		Edit::AutoCompleteOnTypeIdentifier |
		Edit::ImportAutoCompleteOnTypeQuotationMark |
		Edit::GotoDefinitionOnCtrlClick;
}

void
EditPrivate::init() {
	EditBasePrivate::init();

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
}

void
EditPrivate::requestCodeAssist(
	int delay,
	CodeAssistKind kind
) {
	Q_Q(Edit);

	QTextCursor cursor = q->textCursor();
	requestCodeAssist(delay, kind, cursor.position());
}

void
EditPrivate::requestCodeAssist(
	int delay,
	CodeAssistKind kind,
	int position
) {
	Q_Q(Edit);

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
EditPrivate::requestQuickInfoTip(
	int delay,
	const QPoint& pos
) {
	Q_Q(Edit);

	QTextCursor cursor = q->cursorForPosition(pos);
	requestCodeAssist(delay, CodeAssistKind_QuickInfoTip, cursor.position());
}

void
EditPrivate::startCodeAssistThread(
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

	ASSERT(!m_fileName.isEmpty());
	m_thread->request(m_fileName, kind, position, q->toPlainText());
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
	popup->setPalette(m_theme.completerPalette());

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
	popup->setCurrentIndex(m_completer->model()->index(0, 0));

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
	ModuleItemKind itemKind = item->getItemKind();
	if (itemKind == ModuleItemKind_Template) {
		Template* templ = (Template*)item;
		return
			templ->getDerivableTypeKind() != TypeKind_Void ||
			templ->getDecl()->getStorageKind() == StorageKind_Typedef ?
				Icon_Type :
				Icon_Function;
	}

	static const size_t iconTable[ModuleItemKind__Count] = {
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
	};

	ASSERT((size_t)itemKind < countof(iconTable));
	return iconTable[itemKind];
}

void
EditPrivate::addAutoCompleteNamespace(
	QStandardItemModel* model,
	Namespace* nspace
) {
	NamespaceKind namespaceKind = nspace->getNamespaceKind();
	if (namespaceKind == NamespaceKind_Type) {
		NamedType* namedType = (NamedType*)nspace->getDeclItem();
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
		QString name = item->getDecl()->getName();
		if (name.startsWith('!')) // internal item
			continue;

		ModuleItemKind itemKind = item->getItemKind();
		Type* type = item->getType();
		QString synopsis = item->getItemString(ModuleItemStringKind_Synopsis);
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
		if (hasCursorHighlightColor(cursor) || // not within keywords/literals/comments/etc...
			!(flags & CodeAssistFlag_QualifiedName) && getCursorPrevChar(cursor) == '.' // ...and not after member operators
		) {
			hideCodeAssist();
			return;
		}
	}

	ensureCompleter();
	QStandardItemModel* model = new QStandardItemModel(m_completer);
	addAutoCompleteNamespace(model, nspace);

	if (flags & CodeAssistFlag_IncludeParentNamespace) {
		nspace = nspace->getParentNamespace();

		while (nspace) {
			addAutoCompleteNamespace(model, nspace);
			nspace = nspace->getParentNamespace();
		}
	}

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
EditPrivate::keyPressControlSpace(QKeyEvent* e) {
	Q_Q(Edit);

	if (e->modifiers() & Qt::ShiftModifier) {
		if (m_codeAssistTriggers & Edit::ArgumentTipOnCtrlShiftSpace)
			requestCodeAssist(0, CodeAssistKind_ArgumentTip);
	} else {
		if (m_codeAssistTriggers & Edit::AutoCompleteOnCtrlSpace)
			requestCodeAssist(0, CodeAssistKind_AutoComplete);
	}
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
	case '(':
	case '[':
	case '{':
		EditBasePrivate::keyPressPrintChar(e);

		if ((m_codeAssistTriggers & Edit::ArgumentTipOnTypeLeftParenthesis) && c == '(')
			requestCodeAssist(CodeAssistDelay_ArgumentTipInitial, CodeAssistKind_ArgumentTip);

		break;

	case '.':
		EditBasePrivate::keyPressPrintChar(e);

		if ((m_codeAssistTriggers & Edit::AutoCompleteOnTypeDot) && !hasCursorHighlightColor(cursor))
			requestCodeAssist(CodeAssistDelay_AutoComplete, CodeAssistKind_AutoComplete);

		break;

	case ',':
		EditBasePrivate::keyPressPrintChar(e);

		if ((m_codeAssistTriggers & Edit::ArgumentTipOnTypeComma) && !hasCursorHighlightColor(cursor))
			requestCodeAssist(CodeAssistDelay_ArgumentTipComma, CodeAssistKind_ArgumentTip);

		break;

	case '"':
		isImportAutoComplete =
			((m_codeAssistTriggers & Edit::ImportAutoCompleteOnTypeQuotationMark) &&
			getCursorLinePrefix(cursor).trimmed() == "import");

		EditBasePrivate::keyPressPrintChar(e);

		if (isImportAutoComplete)
			requestCodeAssist(CodeAssistDelay_AutoComplete, CodeAssistKind_AutoComplete);

		break;

	default:
		EditBasePrivate::keyPressPrintChar(e);
	}
}

void
EditPrivate::timerEvent(QTimerEvent* e) {
	Q_Q(Edit);

	if (e->timerId() != m_codeAssistTimer.timerId())
		return;

	m_codeAssistTimer.stop();
	startCodeAssistThread(m_pendingCodeAssistKind, m_pendingCodeAssistPosition);
}

void
EditPrivate::onCursorPositionChanged() {
	switch (m_lastCodeAssistKind) {
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
	text += function->getItemString(ModuleItemStringKind_QualifiedName);
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
			m_lastCodeAssistKind == CodeAssistKind_QuickInfoTip
		) // don't let failed quick-info to ruin other code-assists
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
		hideCodeAssist();
		// not yet...
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
