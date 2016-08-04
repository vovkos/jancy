#include "pch.h"
#include "modulepane.h"
#include "mdichild.h"
#include "moc_modulepane.cpp"

#include "axl_g_WarningSuppression.h" // gcc loses warning suppresion from pch

ModulePane::ModulePane(QWidget *parent)
	: QTreeWidget(parent)
{
	document = 0;

	setColumnCount(1);
	header()->hide();

	QObject::connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
		this, SLOT(onItemDoubleClicked(QTreeWidgetItem *, int)));
}

bool ModulePane::build (jnc::Module* module, MdiChild* document)
{
	clear();

	jnc::GlobalNamespace *globalNamespace =	module->getGlobalNamespace();
	addNamespace(0, globalNamespace);

	this->document = document;

	return true;
}

void ModulePane::clear()
{
	QTreeWidget::clear();
	document = 0;
}

void ModulePane::onItemDoubleClicked(QTreeWidgetItem *treeItem, int column)
{
	QVariant variant = treeItem->data(0, Qt::UserRole);
	if(variant.isNull())
		return;

	jnc::ModuleItemDecl* decl = (jnc::ModuleItemDecl*) variant.value<void *>();
	if (!decl)
		return;

	document->selectLine(decl->getLine (), true);
	document->setFocus();
}

QTreeWidgetItem *ModulePane::insertItem(const QString &text,
	QTreeWidgetItem *parent)
{
	QTreeWidgetItem *item = new QTreeWidgetItem;
	item->setText(0, text);

	if (parent)
		parent->addChild(item);
	else
		addTopLevelItem(item);

	return item;
}

bool ModulePane::addItemAttributes(QTreeWidgetItem *parent,
	jnc::ModuleItemDecl *decl)
{
	jnc::AttributeBlock *attributeBlock = decl->getAttributeBlock();
	if (!attributeBlock)
		return false;

	QTreeWidgetItem *attributes = insertItem("attributes", parent);

	size_t count = attributeBlock->getAttributeCount ();
	for (size_t i = 0; i < count; i++)
	{
		jnc::Attribute* attribute = attributeBlock->getAttribute (i);
		jnc::ModuleItemDecl* decl = attribute->getDecl ();

		QTreeWidgetItem *item = insertItem (decl->getName (), attributes);
		item->setData(0, Qt::UserRole, qVariantFromValue((void*) decl));
	}

	expandItem(attributes);
	return true;
}

void ModulePane::addNamespace(QTreeWidgetItem *parent,
	jnc::GlobalNamespace *globalNamespace)
{
	jnc::ModuleItemKind itemKind = globalNamespace->getItemKind();

	QTreeWidgetItem *treeItem = 0;

	if (itemKind == jnc::ModuleItemKind_Scope)
	{
		treeItem = insertItem("scope", parent);
	}
	else if (!globalNamespace->getDecl ()->getParentNamespace())
	{
		treeItem = insertItem("global", parent);
	}
	else
	{
		QString text;
		text.sprintf("namespace %s",
			(const char *)globalNamespace->getDecl ()->getName());

		treeItem = insertItem(text, parent);
	}

	treeItem->setData(0, Qt::UserRole,
		qVariantFromValue((void *)globalNamespace->getDecl ()));

	jnc::Namespace* nspace = globalNamespace->getNamespace ();
	size_t count = nspace->getItemCount();
	for (size_t i = 0; i < count; i++) {
		jnc::ModuleItem *child = nspace->getItem (i);
		addItem(treeItem, child);
	}

	expandItem(treeItem);
}

void ModulePane::addItem(QTreeWidgetItem *parent, jnc::ModuleItem *item)
{
	jnc::ModuleItemKind itemKind = item->getItemKind();

	QString name;
	QTreeWidgetItem* treeItem;

	switch (itemKind)
	{
	case jnc::ModuleItemKind_Namespace:
		addNamespace (parent, (jnc::GlobalNamespace*) item);
		break;

	case jnc::ModuleItemKind_Type:
		addType (parent, (jnc::Type*) item);
		break;

	case jnc::ModuleItemKind_Typedef:
		addTypedef (parent, (jnc::Typedef*) item);
		break;

	case jnc::ModuleItemKind_Variable:
		addVariable (parent, (jnc::Variable*) item);
		break;

	case jnc::ModuleItemKind_Function:
		addFunction (parent, (jnc::Function*) item);
		break;

	case jnc::ModuleItemKind_Property:
		addProperty (parent, (jnc::Property*) item);
		break;

	case jnc::ModuleItemKind_EnumConst:
		addEnumConst (parent, (jnc::EnumConst*) item);
		break;

	case jnc::ModuleItemKind_StructField:
		addStructField (parent, (jnc::StructField*) item);
		break;

	case jnc::ModuleItemKind_Lazy:
		// don't display lazy items
		break;

	default:
		name.sprintf("item %p of kind %d", item, itemKind);

		treeItem = insertItem(name, parent);
		treeItem->setText(0, name);
	}
}

void ModulePane::addType(QTreeWidgetItem *parent, jnc::Type *type)
{
	QString itemName = (const char *)type->getTypeString();

	QTreeWidgetItem *item = insertItem(itemName, parent);

	QString toolTip = QString ("%1 (sizeof = %2)").arg (type->getTypeString ()).arg (type->getSize ());
	item->setToolTip (0, toolTip);
	
	jnc::ModuleItemDecl* decl = NULL;
	jnc::TypeKind typeKind = type->getTypeKind();
	switch (typeKind)
	{
	case jnc::TypeKind_Enum:
		decl = ((jnc::EnumType *)type)->getDecl ();
		addItemAttributes (item, decl);
		addEnumTypeMembers(item, (jnc::EnumType *)type);
		break;

	case jnc::TypeKind_Struct:
	case jnc::TypeKind_Union:
	case jnc::TypeKind_Class:
		decl = ((jnc::DerivableType *)type)->getDecl ();
		addItemAttributes (item, decl);
		addDerivableTypeMembers(item, (jnc::DerivableType *)type);
		break;
	}

	item->setData(0, Qt::UserRole, qVariantFromValue((void *)decl));
}

void ModulePane::addTypedef (QTreeWidgetItem *parent, jnc::Typedef* tdef)
{
	QString name;
	name.sprintf ("typedef %s", tdef->getType ()->createDeclarationString_v (tdef->getDecl ()->getName ()));

	QTreeWidgetItem *item = insertItem (name, parent);
	item->setData (0, Qt::UserRole, qVariantFromValue((void *) (jnc::ModuleItem*) tdef));
}

void ModulePane::addVariable(QTreeWidgetItem *parent, jnc::Variable *variable)
{
	addValue (parent, variable->getDecl ()->getName(), variable->getType(), variable);
}

void ModulePane::addEnumConst(QTreeWidgetItem *parent, jnc::EnumConst *member)
{
	QTreeWidgetItem *item = insertItem((const char *)member->getDecl ()->getName(), parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void *)member));
}

void ModulePane::addValue (QTreeWidgetItem *parent, const char* name, jnc::Type* type, jnc::ModuleItem* moduleItem)
{
	QString itemName = type->createDeclarationString_v (name);
	QTreeWidgetItem *item = insertItem(itemName, parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void *) moduleItem));

	if (jnc::isClassType (type, jnc::ClassTypeKind_Reactor))
		addDerivableTypeMembers (item, (jnc::ClassType*) type);
}

void ModulePane::addEnumTypeMembers (QTreeWidgetItem *parent, jnc::EnumType* type)
{
	size_t count = type->getConstCount ();
	for (size_t i = 0; i < count; i++)
	{
		jnc::EnumConst* member = type->getConst (i);
		addEnumConst (parent, member);
	}

	expandItem (parent);
}

void ModulePane::addDerivableTypeMembers(QTreeWidgetItem *parent, jnc::DerivableType *type)
{
	size_t count = type->getMemberFieldCount ();
	for (size_t i = 0; i < count; i++)
	{
		jnc::StructField* field = type->getMemberField (i);
		addStructField (parent, field);
	}

	if (type->getStaticConstructor ())
		addItem (parent, type->getStaticConstructor ());

	if (type->getStaticDestructor ())
		addItem (parent, type->getStaticDestructor ());

	if (type->getPreConstructor ())
		addItem (parent, type->getPreConstructor ());

	if (type->getConstructor ())
		addItem (parent, type->getConstructor ());

	if (type->getDestructor ())
		addItem (parent, type->getDestructor ());

	count = type->getMemberPropertyCount ();
	for (size_t i = 0; i < count; i++)
	{
		jnc::Property* prop = type->getMemberProperty (i);
		addProperty (parent, prop);
	}

	count = type->getMemberMethodCount ();
	for (size_t i = 0; i < count; i++)
	{
		jnc::Function* function = type->getMemberMethod (i);
		addFunction (parent, function);
	}

	expandItem (parent);
}

void ModulePane::addFunction (QTreeWidgetItem *parent, jnc::Function* function)
{
	if (!function->isOverloaded ())
	{
		addFunctionImpl (parent, function);
	}
	else
	{
		size_t count = function->getOverloadCount ();

		QString itemName;
		itemName.sprintf (
			"%s (%d overloads)",
			function->getDecl ()->getName (),
			count
			);

		QTreeWidgetItem *item = insertItem (itemName, parent);
		for (size_t i = 0; i < count; i++)
		{
			jnc::Function* pOverload = function->getOverload (i);
			addFunctionImpl (item, pOverload);
		}

		expandItem (item);
	}
}

void ModulePane::addFunctionImpl (QTreeWidgetItem *parent, jnc::Function* function)
{
	jnc::FunctionType* type = function->getType ();

	sl::String Name = function->getFunctionKind () == jnc::FunctionKind_Named ?
		function->getDecl ()->getName () :
		sl::String (jnc::getFunctionKindString (function->getFunctionKind ()));

	QString itemName = type->createDeclarationString_v (Name);
	QTreeWidgetItem *item = insertItem (itemName, parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void *)(jnc::ModuleItem*) function));
}

void ModulePane::addProperty (QTreeWidgetItem *parent, jnc::Property* prop)
{
	QTreeWidgetItem *item = insertItem (prop->getDecl ()->getName (), parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void *)(jnc::ModuleItem*) prop));

	jnc::Function* getter = prop->getGetter ();
	jnc::Function* setter = prop->getSetter ();

	addFunction (item, getter);

	if (setter)
		addFunction (item, setter);

	expandItem (item);
}
