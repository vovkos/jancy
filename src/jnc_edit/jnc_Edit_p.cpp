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
#include "jnc_CodeAssistThread.h"
#include "jnc_Highlighter.h"
#include "jnc_CursorUtils.h"
#include "qrc_res.cpp"

namespace jnc {

//..............................................................................

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

//..............................................................................

Edit::Edit(QWidget *parent):
	EditBase(parent, new EditPrivate) {
	Q_D(Edit);
	d->q_ptr = this;
	d->m_syntaxHighlighter = createSyntaxHighlighter();
	d->init();
}

Edit::~Edit() {}

HighlighterBase*
Edit::createSyntaxHighlighter() {
	Q_D(Edit);
	return new JancyHighlighter(document(), &d->m_theme);
}

CodeAssistThreadBase*
Edit::createCodeAssistThread() {
	return new CodeAssistThread(this);
}

CodeTipBase*
Edit::createCodeTip() {
	Q_D(Edit);
	return new CodeTip(this);
}

void
Edit::autoIndent(
	QTextCursor* cursor,
	const QString& baseIndent,
	const QString& tailWord
) {
	if (tailWord == QChar('{')) {
		QChar next = getCursorNextChar(*cursor);
		cursor->insertText(QChar('\n'));
		cursor->insertText(baseIndent);
		cursor->insertText(QChar('\t'));
		int finalPosition = cursor->position();

		if (next == QChar('}')) {
			cursor->insertText(QChar('\n'));
			cursor->insertText(baseIndent);
		}

		cursor->setPosition(finalPosition);
	} else {
		QTextCursor solCursor = *cursor;
		solCursor.movePosition(QTextCursor::StartOfLine);
		solCursor.movePosition(QTextCursor::NextWord);
		solCursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
		QString keyword = solCursor.selectedText();

		cursor->insertText(QChar('\n'));
		cursor->insertText(baseIndent);

		static const QRegExp indentKeywordRegExp("^(if|else|while|do|for)$");
		if (indentKeywordRegExp.exactMatch(keyword))
			cursor->insertText(QChar('\t'));
	}
}

void
Edit::activateCompleter(const QModelIndex& index) {
	Q_D(Edit);

	QTextCursor cursor = textCursor();

	Function* function = d->getPrototypeFunction(index);
	if (function && getCursorLineSuffix(cursor).trimmed().isEmpty()) {
		bool isNextLineEmpty = isCursorNextLineEmpty(cursor);
		QString completion = getPrototypeDeclString(function, isNextLineEmpty);
		cursor.select(QTextCursor::LineUnderCursor);
		cursor.insertText(completion);

		int delta = isNextLineEmpty ? 2 : 3; // inside body after \t
		cursor.setPosition(cursor.position() - delta);
		setTextCursor(cursor);
	} else if (d->m_activeCodeAssistKind == CodeAssistKind_ImportAutoComplete) {
		QModelIndex nameIndex = index.sibling(index.row(), NameColumn); // user could have clicked on synopsis
		QString completion = d->m_completer->popup()->model()->data(nameIndex, Qt::DisplayRole).toString();
		int basePosition = d->m_activeCodeAssistPosition;
		QString quotedCompletion = '"' + completion + '"';
		cursor.setPosition(basePosition);
		cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
		cursor.insertText(quotedCompletion);
	} else
		EditBase::activateCompleter(index);
}

void
Edit::showCodeAssist(CodeAssistThreadBase* thread0) {
	Q_D(Edit);

	CodeAssistThread* thread = qobject_cast<CodeAssistThread*>(thread0);
	ASSERT(thread);

	CodeAssist* codeAssist = thread->getModule()->getCodeAssist();
	if (!codeAssist) {
		if (thread->codeAssistKind() != CodeAssistKind_QuickInfoTip ||
			d->m_activeCodeAssistKind == CodeAssistKind_QuickInfoTip
		) // don't let failed quick-info ruin other code-assists
			d->hideCodeAssist();

		return;
	}

	d->createCodeAssist(thread->getModule(), codeAssist);
}

void
Edit::releaseCodeAssist() {
	Q_D(Edit);
	EditBase::releaseCodeAssist();
	d->m_activeCodeAssistModule = rc::g_nullPtr; // drop cache
}

void
Edit::keyPressPrintChar(QKeyEvent* e) {
	Q_D(Edit);

	QString text = e->text();
	QChar ch = text.isEmpty() ? QChar() : text.at(0);
	int c = ch.toLatin1();

	QTextCursor cursor = textCursor();
	bool isImportAutoComplete;

	switch (c) {
	case '(':
	case '<':
		EditBase::keyPressPrintChar(e);

		if (d->m_codeAssistTriggers & ArgumentTipOnTypeLeftParenthesis)
			d->requestCodeAssist(ArgumentTipInitialDelay, CodeAssistKind_ArgumentTip);

		break;

	case '>':
		if (getCursorPrevChar(cursor) != '-') {
			EditBase::keyPressPrintChar(e);
			break;
		}

		// fall through on operator ->

	case '.':
		EditBase::keyPressPrintChar(e);

		if ((d->m_codeAssistTriggers & AutoCompleteOnTypeDot) && !hasCursorHighlightColor(cursor))
			d->requestCodeAssist(AutoCompleteDelay, CodeAssistKind_AutoComplete);

		break;

	case ',':
		EditBase::keyPressPrintChar(e);

		if ((d->m_codeAssistTriggers & ArgumentTipOnTypeComma) && !hasCursorHighlightColor(cursor))
			d->requestCodeAssist(ArgumentTipCommaDelay, CodeAssistKind_ArgumentTip);

		break;

	case '"':
		isImportAutoComplete =
			((d->m_codeAssistTriggers & ImportAutoCompleteOnTypeQuotationMark) &&
			getCursorLinePrefix(cursor).trimmed() == "import");

		EditBase::keyPressPrintChar(e);

		if (isImportAutoComplete)
			d->requestCodeAssist(AutoCompleteDelay, CodeAssistKind_ImportAutoComplete);

		break;

	default:
		EditBase::keyPressPrintChar(e);
	}
}

//..............................................................................

void
EditPrivate::createCodeAssist(
	const rc::Ptr<Module>& module,
	CodeAssist* codeAssist
) {
	m_activeCodeAssistModule = module; // cache
	m_activeCodeAssistKind = codeAssist->getCodeAssistKind();
	m_activeCodeAssistPosition = cursorFromOffset(codeAssist->getOffset()).position();

	switch (m_activeCodeAssistKind) {
	case CodeAssistKind_QuickInfoTip:
		createQuickInfoTip(codeAssist->getModuleItem());
		break;

	case CodeAssistKind_ArgumentTip: {
		Template* templ = codeAssist->getTemplate();
		if (templ)
			createArgumentTip(templ, codeAssist->getArgumentIdx());
		else
			createArgumentTip(codeAssist->getFunctionTypeOverload(), codeAssist->getArgumentIdx());
		break;
		}

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
EditPrivate::createQuickInfoTip(ModuleItem* item) {
	Q_Q(Edit);

	CodeTip* codeTip = qobject_cast<CodeTip*>(ensureCodeTip());
	ASSERT(codeTip);

	QPoint point = activeCodeTipPoint();
	codeTip->showQuickInfoTip(point, item);
}

void
EditPrivate::createArgumentTip(
	FunctionTypeOverload* typeOverload,
	size_t argumentIdx
) {
	Q_Q(Edit);

	CodeTip* codeTip = qobject_cast<CodeTip*>(ensureCodeTip());
	ASSERT(codeTip);

	QPoint point = activeCodeTipPoint();
	codeTip->showArgumentTip(point, typeOverload, argumentIdx);
}

void
EditPrivate::createArgumentTip(
	Template* templ,
	size_t argumentIdx
) {
	Q_Q(Edit);

	CodeTip* codeTip = qobject_cast<CodeTip*>(ensureCodeTip());
	ASSERT(codeTip);

	QPoint point = activeCodeTipPoint();
	codeTip->showArgumentTip(point, templ, argumentIdx);
}

size_t
EditPrivate::getItemIconIdx(ModuleItem* item) {
	ModuleItemKind itemKind = item->getItemKind();
	if (itemKind == ModuleItemKind_Template) {
		Template* templ = (Template*)item;
		return
			templ->getDerivableTypeKind() != TypeKind_Void ||
			templ->getDecl()->getStorageKind() == StorageKind_Typedef ?
				Edit::TypeIcon :
				Edit::FunctionIcon;
	}

	static const size_t iconTable[ModuleItemKind__Count] = {
		Edit::ObjectIcon,    // ModuleItemKind_Undefined
		Edit::NamespaceIcon, // ModuleItemKind_Namespace
		Edit::ObjectIcon,    // ModuleItemKind_Attribute
		Edit::ObjectIcon,    // ModuleItemKind_AttributeBlock
		Edit::ObjectIcon,    // ModuleItemKind_Scope
		Edit::TypeIcon,      // ModuleItemKind_Type
		Edit::TypedefIcon,   // ModuleItemKind_Typedef
		Edit::ObjectIcon,    // ModuleItemKind_Alias
		Edit::ConstIcon,     // ModuleItemKind_Const
		Edit::VariableIcon,  // ModuleItemKind_Variable
		Edit::FunctionIcon,  // ModuleItemKind_Function
		Edit::VariableIcon,  // ModuleItemKind_FunctionArg
		Edit::FunctionIcon,  // ModuleItemKind_FunctionOverload
		Edit::PropertyIcon,  // ModuleItemKind_Property
		Edit::PropertyIcon,  // ModuleItemKind_PropertyTemplate
		Edit::ConstIcon,     // ModuleItemKind_EnumConst
		Edit::VariableIcon,  // ModuleItemKind_Field
		Edit::TypeIcon,      // ModuleItemKind_BaseTypeSlot
		Edit::FunctionIcon,  // ModuleItemKind_Orphan
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

		if (name.isEmpty()) {
			Module* module = item->getModule();
			ASSERT(
				item == module->getGlobalNamespace() &&
				nspace->getDeclItem() == module->getStdNamespace(StdNamespace_Jnc)
			);

			name = "global";
		}

		ModuleItemKind itemKind = item->getItemKind();
		Type* type = item->getType();
		QString synopsis = item->getItemString(ModuleItemStringKind_Synopsis);
		size_t iconIdx = getItemIconIdx(item);

		QStandardItem* nameItem = new QStandardItem;
		nameItem->setText(name);
		nameItem->setData(name.toLower(), Edit::CaseInsensitiveSortRole);
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
		QTextCursor cursor = activeCodeAssistCursor();
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

	model->setSortRole(Edit::CaseInsensitiveSortRole);
	model->sort(0);

	m_completer->setModel(model);
	m_completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_completer->setWrapAround(false);
	m_completer->setCompletionPrefix(QString());

	updateCompleter(true);
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
			q->addFile(model, dirIt.fileName());
		}
	}

	it = module->getExtensionSourceFileIterator();
	while (it) {
		const char* fileName = module->getNextExtensionSourceFile(&it);
		q->addFile(model, fileName);
	}

	ensureCompleter();

	model->setSortRole(Edit::CaseInsensitiveSortRole);
	model->sort(0);

	m_completer->setModel(model);
	m_completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_completer->setWrapAround(false);
	m_completer->setCompletionPrefix(QString());

	updateCompleter(true);
}

Function*
EditPrivate::getPrototypeFunction(const QModelIndex& index) {
	ModuleItem* item = (ModuleItem*)m_completer->popup()->model()->data(index, Role_ModuleItem).value<void*>();
	if (!item || item->getItemKind() != ModuleItemKind_Function)
		return NULL;

	ModuleItemDecl* decl = item->getDecl();
	if (decl->getParentNamespace() != m_activeCodeAssistModule->getCodeAssist()->getNamespace())
		return NULL;

	AttributeBlock* block = decl->getAttributeBlock();
	return block && block->findAttribute("prototype") ? (Function*)item : NULL;
}

//..............................................................................

} // namespace jnc
