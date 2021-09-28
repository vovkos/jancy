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
#include "modulepane.h"
#include "mdichild.h"
#include "moc_modulepane.cpp"

ModulePane::ModulePane(QWidget *parent)
	: QTreeWidget(parent) {
	m_document = NULL;

	setColumnCount(1);
	header()->hide();

	QObject::connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
		this, SLOT(onItemDoubleClicked(QTreeWidgetItem *, int)));
}

bool ModulePane::build(jnc::Module* module, MdiChild* document) {
	clear();
	jnc::GlobalNamespace* globalNamespace = module->getGlobalNamespace();
	addNamespace(0, globalNamespace);
	m_document = document;
	return true;
}

void ModulePane::clear() {
	QTreeWidget::clear();
	m_document = NULL;
}

void ModulePane::onItemDoubleClicked(QTreeWidgetItem *treeItem, int column) {
	QVariant variant = treeItem->data(0, Qt::UserRole);
	if(variant.isNull())
		return;

	jnc::ModuleItemDecl* decl = (jnc::ModuleItemDecl*)variant.value<void*> ();
	if (!decl)
		return;

	m_document->setTextCursorLineCol(decl->getLine(), decl->getCol());
	m_document->setFocus();
}

QTreeWidgetItem *ModulePane::insertItem(const QString &text, QTreeWidgetItem *parent) {
	QTreeWidgetItem *item = new QTreeWidgetItem;
	item->setText(0, text);

	if (parent)
		parent->addChild(item);
	else
		addTopLevelItem(item);

	return item;
}

bool ModulePane::addItemAttributes(QTreeWidgetItem *parent, jnc::ModuleItemDecl *decl) {
	jnc::AttributeBlock *attributeBlock = decl->getAttributeBlock();
	if (!attributeBlock)
		return false;

	QTreeWidgetItem *attributes = insertItem("attributes", parent);

	size_t count = attributeBlock->getAttributeCount();
	for (size_t i = 0; i < count; i++) {
		jnc::Attribute* attribute = attributeBlock->getAttribute(i);
		jnc::ModuleItemDecl* decl = attribute->getDecl();

		QTreeWidgetItem *item = insertItem(decl->getName(), attributes);
		item->setData(0, Qt::UserRole, qVariantFromValue((void*)decl));
	}

	expandItem(attributes);
	return true;
}

void ModulePane::addNamespace(QTreeWidgetItem *parent,
	jnc::GlobalNamespace *globalNamespace) {
	jnc::ModuleItemKind itemKind = globalNamespace->getItemKind();

	QTreeWidgetItem *treeItem = 0;

	if (itemKind == jnc::ModuleItemKind_Scope) {
		treeItem = insertItem("scope", parent);
	} else if (!globalNamespace->getDecl()->getParentNamespace()) {
		treeItem = insertItem("global", parent);
	} else {
		QString text;
		text.sprintf("namespace %s",
			(const char *)globalNamespace->getDecl()->getName());

		treeItem = insertItem(text, parent);
	}

	treeItem->setData(0, Qt::UserRole, qVariantFromValue((void*)globalNamespace->getDecl()));

	jnc::Namespace* nspace = globalNamespace->getNamespace();
	size_t count = nspace->getItemCount();
	for (size_t i = 0; i < count; i++) {
		jnc::ModuleItem *child = nspace->getItem(i);
		addItem(treeItem, child);
	}

	expandItem(treeItem);
}

void ModulePane::addItem(QTreeWidgetItem *parent, jnc::ModuleItem *item) {
	jnc::ModuleItemKind itemKind = item->getItemKind();

	QString name;
	QTreeWidgetItem* treeItem;

	switch (itemKind) {
	case jnc::ModuleItemKind_Namespace:
		addNamespace(parent, (jnc::GlobalNamespace*)item);
		break;

	case jnc::ModuleItemKind_Type:
		addType(parent, (jnc::Type*)item);
		break;

	case jnc::ModuleItemKind_Typedef:
		addTypedef(parent, (jnc::Typedef*)item);
		break;

	case jnc::ModuleItemKind_Variable:
		addVariable(parent, (jnc::Variable*)item);
		break;

	case jnc::ModuleItemKind_Function:
		addFunction(parent, (jnc::Function*)item);
		break;

	case jnc::ModuleItemKind_Property:
		addProperty(parent, (jnc::Property*)item);
		break;

	case jnc::ModuleItemKind_EnumConst:
		addEnumConst(parent, (jnc::EnumConst*)item);
		break;

	case jnc::ModuleItemKind_Field:
		addField(parent, (jnc::Field*)item);
		break;

	case jnc::ModuleItemKind_Alias:
		addAlias(parent, (jnc::Alias*)item);
		break;

	default:
		name.sprintf("item %p of kind %d", item, itemKind);

		treeItem = insertItem(name, parent);
		treeItem->setText(0, name);
	}
}

void ModulePane::addType(QTreeWidgetItem *parent, jnc::Type *type) {
	QString itemName = (const char*)type->getTypeString();

	QTreeWidgetItem *item = insertItem(itemName, parent);

	if (type->getFlags() & jnc::ModuleItemFlag_LayoutReady) {
		QString toolTip = QString("%1 (sizeof = %2)").arg (type->getTypeString()).arg (type->getSize ());
		item->setToolTip(0, toolTip);
	}

	jnc::ModuleItemDecl* decl = NULL;
	jnc::TypeKind typeKind = type->getTypeKind();
	switch (typeKind) {
	case jnc::TypeKind_Enum:
		decl = ((jnc::EnumType *)type)->getDecl();
		addItemAttributes(item, decl);
		addEnumTypeMembers(item, (jnc::EnumType *)type);
		break;

	case jnc::TypeKind_Struct:
	case jnc::TypeKind_Union:
	case jnc::TypeKind_Class:
		decl = ((jnc::DerivableType *)type)->getDecl();
		addItemAttributes(item, decl);
		addDerivableTypeMembers(item, (jnc::DerivableType *)type);
		break;
	}

	item->setData(0, Qt::UserRole, qVariantFromValue((void*)decl));
}

void ModulePane::addTypedef(QTreeWidgetItem *parent, jnc::Typedef* tdef) {
	QString name;
	name.sprintf("typedef %s %s %s",
		tdef->getType()->getTypeStringPrefix(),
		tdef->getDecl()->getName(),
		tdef->getType()->getTypeStringSuffix()
	);

	QTreeWidgetItem *item = insertItem(name, parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void*)tdef->getDecl()));
}

void ModulePane::addVariable(QTreeWidgetItem *parent, jnc::Variable *variable) {
	addValue(parent, variable->getDecl()->getName(), variable->getType(), variable);
}

void ModulePane::addEnumConst(QTreeWidgetItem *parent, jnc::EnumConst *member) {
	QTreeWidgetItem *item = insertItem((const char*) member->getDecl()->getName(), parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void*)member->getDecl()));
}

void ModulePane::addValue(QTreeWidgetItem *parent, const QString& name, jnc::Type* type, jnc::ModuleItem* moduleItem) {
	QString itemName = QString("%1 %2 %3").arg (type->getTypeStringPrefix ()).arg (name).arg (type->getTypeStringSuffix ());
	QTreeWidgetItem *item = insertItem(itemName, parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void*)moduleItem->getDecl()));

	if (jnc::isClassType(type, jnc::ClassTypeKind_Reactor))
		addDerivableTypeMembers(item, (jnc::ClassType*)type);
}

void ModulePane::addEnumTypeMembers(QTreeWidgetItem *parent, jnc::EnumType* type) {
	size_t count = type->getConstCount();
	for (size_t i = 0; i < count; i++) {
		jnc::EnumConst* member = type->getConst(i);
		addEnumConst(parent, member);
	}

	expandItem(parent);
}

void ModulePane::addDerivableTypeMembers(QTreeWidgetItem *parent, jnc::DerivableType *type) {
	size_t count = type->getStaticVariableCount();
	for (size_t i = 0; i < count; i++)
		addVariable(parent, type->getStaticVariable(i));

	count = type->getFieldCount();
	for (size_t i = 0; i < count; i++)
		addField(parent, type->getField(i));

	if (type->getStaticConstructor())
		addItem(parent, type->getStaticConstructor());

	if (type->getConstructor())
		addOverloadableFunction(parent, type->getConstructor());

	if (type->getDestructor())
		addItem(parent, type->getDestructor());

	count = type->getPropertyCount();
	for (size_t i = 0; i < count; i++)
		addProperty(parent, type->getProperty(i));

	count = type->getMethodCount();
	for (size_t i = 0; i < count; i++)
		addFunction(parent, type->getMethod(i));

	expandItem(parent);
}

void ModulePane::addFunction(QTreeWidgetItem *parent, jnc::Function* function) {
	jnc::FunctionType* type = function->getType();

	const char* name = function->getFunctionKind() == jnc::FunctionKind_Normal ?
		function->getDecl()->getName() :
		jnc::getFunctionKindString(function->getFunctionKind());

	QString itemName = QString("%1 %2%3").arg(type->getTypeStringPrefix(), name, type->getTypeStringSuffix());
	QTreeWidgetItem *item = insertItem(itemName, parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void*)function->getDecl()));
}

void ModulePane::addFunctionOverload(QTreeWidgetItem *parent, jnc::FunctionOverload* overload) {
	size_t count = overload->getOverloadCount();

	const char* name = overload->getFunctionKind() == jnc::FunctionKind_Normal ?
		overload->getDecl()->getName() :
		jnc::getFunctionKindString(overload->getFunctionKind());

	QString itemName = QString("%1 (%2 overloads)").arg(name).arg(count);

	QTreeWidgetItem *item = insertItem(itemName, parent);
	for (size_t i = 0; i < count; i++) {
		jnc::Function* function = overload->getOverload(i);
		addFunction(item, function);
	}

	expandItem(item);
}

void ModulePane::addOverloadableFunction(QTreeWidgetItem* parent, jnc::OverloadableFunction function) {
	function->getItemKind() == jnc::ModuleItemKind_Function ?
		addFunction(parent, function.getFunction()) :
		addFunctionOverload(parent, function.getFunctionOverload());
}

void ModulePane::addProperty(QTreeWidgetItem *parent, jnc::Property* prop) {
	QTreeWidgetItem *item = insertItem(prop->getDecl()->getName(), parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void*)prop->getDecl()));

	jnc::Function* getter = prop->getGetter();
	addFunction(item, getter);

	jnc::OverloadableFunction setter = prop->getSetter();
	if (setter)
		addOverloadableFunction(item, setter);

	expandItem(item);
}

void ModulePane::addAlias(QTreeWidgetItem* parent, jnc::Alias* alias) {
	QString name;
	name.sprintf(
		"alias %s = %s",
		alias->getDecl()->getName(),
		alias->getInitializerString_v()
	);

	QTreeWidgetItem *item = insertItem(name, parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void*)alias->getDecl()));
}
