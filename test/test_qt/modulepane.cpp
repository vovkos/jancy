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

bool ModulePane::build(jnc::Module *module, MdiChild *document)
{
	clear();

	jnc::GlobalNamespace *globalNamespace =
		module->m_namespaceMgr.getGlobalNamespace();
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

	jnc::ModuleItem *item = (jnc::ModuleItem *)variant.value<void *>();
	if(!item)
		return;

	jnc::ModuleItemDecl* decl = item->getItemDecl ();
	if (!decl)
		return;

	document->selectLine(decl->getPos ()->m_line, true);
	document->setFocus();
}

QTreeWidgetItem *ModulePane::insertItem(const QString &text,
	QTreeWidgetItem *parent, void *data)
{
	QTreeWidgetItem *item = new QTreeWidgetItem;
	item->setText(0, text);

	if (parent)
		parent->addChild(item);
	else
		addTopLevelItem(item);

	if (data)
		item->setData(0, Qt::UserRole, qVariantFromValue(data));

	return item;
}

bool ModulePane::addItemAttributes(QTreeWidgetItem *parent,
	jnc::ModuleItem *item)
{
	jnc::ModuleItemDecl* decl = item->getItemDecl ();
	if (!decl)
		return false;

	jnc::AttributeBlock *attributeBlock = decl->getAttributeBlock();
	if (!attributeBlock)
		return false;

	QTreeWidgetItem *attributes = insertItem("attributes", parent);

	rtl::Iterator<jnc::Attribute> attribute = attributeBlock->getAttributeList().getHead();
	for (; attribute; attribute++)
	{
		QTreeWidgetItem *treeItem = insertItem ((const char*) attribute->getName (), attributes);
		treeItem->setData(0, Qt::UserRole, qVariantFromValue ((void*) *attribute));
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
	else if (!globalNamespace->getParentNamespace())
	{
		treeItem = insertItem("global", parent);
	}
	else
	{
		QString text;
		text.sprintf("namespace %s",
			(const char *)globalNamespace->getName());

		treeItem = insertItem(text, parent);
	}

	treeItem->setData(0, Qt::UserRole,
		qVariantFromValue((void *)globalNamespace));

	size_t count = globalNamespace->getItemCount();
	for (size_t i = 0; i < count; i++) {
		jnc::ModuleItem *child = globalNamespace->getItem(i);
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
		name.sprintf("lazy: %s", ((jnc::LazyModuleItem*) item)->getName ().cc ());

		treeItem = insertItem(name, parent);
		treeItem->setText(0, name);
		treeItem->setData(0, Qt::UserRole, qVariantFromValue((void*) item));
		break;

	default:
		name.sprintf("item %p of kind %d", item, itemKind);

		treeItem = insertItem(name, parent);
		treeItem->setText(0, name);
		treeItem->setData(0, Qt::UserRole, qVariantFromValue((void*) item));
	}
}

void ModulePane::addType(QTreeWidgetItem *parent, jnc::Type *type)
{
	QString itemName = (const char *)type->getTypeString();

	QTreeWidgetItem *item = insertItem(itemName, parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void *)type));

	QString toolTip = QString ("%1 (sizeof = %2)").arg (type->getTypeString ().cc ()).arg (type->getSize ());
	item->setToolTip (0, toolTip);

	addItemAttributes(item, type);

	jnc::TypeKind typeKind = type->getTypeKind();
	switch (typeKind)
	{
	case jnc::TypeKind_Enum:
		addEnumTypeMembers(item, (jnc::EnumType *)type);
		break;

	case jnc::TypeKind_Struct:
	case jnc::TypeKind_Union:
		addDerivableTypeMembers(item, (jnc::StructType *)type);
		break;

	case jnc::TypeKind_Class:
		addClassTypeMembers(item, (jnc::ClassType *)type);
		break;
	}
}

void ModulePane::addTypedef (QTreeWidgetItem *parent, jnc::Typedef* typed)
{
	QString name;
	name.sprintf (
		"typedef %s %s",
		typed->getType ()->getTypeString ().cc (), // thanks a lot gcc
		typed->getName ().cc ()
		);

	QTreeWidgetItem *item = insertItem (name, parent);
	item->setData (0, Qt::UserRole, qVariantFromValue((void *) (jnc::ModuleItem*) typed));
}

void ModulePane::addVariable(QTreeWidgetItem *parent, jnc::Variable *variable)
{
	addValue (parent, variable->getName().cc (), variable->getType(), variable);
}

void ModulePane::addEnumConst(QTreeWidgetItem *parent, jnc::EnumConst *member)
{
	QTreeWidgetItem *item = insertItem((const char *)member->getName(), parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void *)member));
}

void ModulePane::addValue (QTreeWidgetItem *parent, const char* name, jnc::Type* type, jnc::ModuleItem* moduleItem)
{
	QString itemName;
	itemName.sprintf ("%s %s", type->getTypeString ().cc (), name);
	QTreeWidgetItem *item = insertItem(itemName, parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void *) moduleItem));

	if (jnc::isClassType (type, jnc::ClassTypeKind_Reactor))
		addClassTypeMembers (item, (jnc::ClassType*) type);
}

void ModulePane::addEnumTypeMembers (QTreeWidgetItem *parent, jnc::EnumType* type)
{
	rtl::Iterator <jnc::EnumConst> Member = type->getConstList ().getHead ();
	for (; Member; Member++)
		addEnumConst (parent, *Member);

	expandItem (parent);
}

void ModulePane::addDerivableTypeMembers(QTreeWidgetItem *parent, jnc::DerivableType *pType)
{
	rtl::Array <jnc::StructField*> FieldArray = pType->getMemberFieldArray ();
	size_t Count = FieldArray.getCount ();
	for (size_t i = 0; i < Count; i++)
	{
		jnc::StructField* pField = FieldArray [i];
		addStructField (parent, pField);
	}

	if (pType->getStaticConstructor ())
		addItem (parent, pType->getStaticConstructor ());

	if (pType->getStaticDestructor ())
		addItem (parent, pType->getStaticDestructor ());

	if (pType->getPreconstructor ())
		addItem (parent, pType->getPreconstructor ());

	if (pType->getConstructor ())
		addItem (parent, pType->getConstructor ());

	rtl::Array <jnc::Property*> PropertyArray = pType->getMemberPropertyArray ();
	Count = PropertyArray.getCount ();
	for (size_t i = 0; i < Count; i++)
	{
		jnc::Property* pProp = PropertyArray [i];
		addProperty (parent, pProp);
	}

	rtl::Array <jnc::Function*> FunctionArray = pType->getMemberMethodArray ();
	Count = FunctionArray.getCount ();
	for (size_t i = 0; i < Count; i++)
	{
		jnc::Function* pFunction = FunctionArray [i];
		addFunction (parent, pFunction);
	}

	expandItem (parent);
}

void ModulePane::addClassTypeMembers (QTreeWidgetItem *parent, jnc::ClassType* pType)
{
	if (pType->getDestructor ())
		addItem (parent, pType->getDestructor ());

	addDerivableTypeMembers (parent, pType);
}

void ModulePane::addFunction (QTreeWidgetItem *parent, jnc::Function* pFunction)
{
	if (!pFunction->isOverloaded ())
	{
		addFunctionImpl (parent, pFunction);
	}
	else
	{
		size_t count = pFunction->getOverloadCount ();

		QString itemName;
		itemName.sprintf (
			"%s (%d overloads)",
			pFunction->m_tag.cc (),
			count
			);

		QTreeWidgetItem *item = insertItem (itemName, parent);
		for (size_t i = 0; i < count; i++)
		{
			jnc::Function* pOverload = pFunction->getOverload (i);
			addFunctionImpl (item, pOverload);
		}

		expandItem (item);
	}
}

void ModulePane::addFunctionImpl (QTreeWidgetItem *parent, jnc::Function* pFunction)
{
	jnc::FunctionType* pType = pFunction->getType ();

	rtl::String Name = pFunction->getFunctionKind () == jnc::FunctionKind_Named ?
		pFunction->getName () :
		rtl::String (jnc::getFunctionKindString (pFunction->getFunctionKind ()));

	QString itemName;
	itemName.sprintf (
		"%s%s %s %s",
		pType->getReturnType ()->getTypeString ().cc (),
		pType->getTypeModifierString ().cc (),
		Name.cc (),
		pFunction->getType ()->getArgString ().cc ()
		);

	QTreeWidgetItem *item = insertItem (itemName, parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void *)(jnc::ModuleItem*) pFunction));
}

void ModulePane::addProperty (QTreeWidgetItem *parent, jnc::Property* pProp)
{
	QTreeWidgetItem *item = insertItem (pProp->m_tag.cc (), parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void *)(jnc::ModuleItem*) pProp));

	jnc::Function* pGetter = pProp->getGetter ();
	jnc::Function* pSetter = pProp->getSetter ();

	addFunction (item, pGetter);

	if (pSetter)
		addFunction (item, pSetter);

	expandItem (item);
}
