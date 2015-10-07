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

bool ModulePane::build(jnc::ct::Module *module, MdiChild *document)
{
	clear();

	jnc::ct::GlobalNamespace *globalNamespace =
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

	jnc::ct::ModuleItem *item = (jnc::ct::ModuleItem *)variant.value<void *>();
	if(!item)
		return;

	jnc::ct::ModuleItemDecl* decl = item->getItemDecl ();
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
	jnc::ct::ModuleItem *item)
{
	jnc::ct::ModuleItemDecl* decl = item->getItemDecl ();
	if (!decl)
		return false;

	jnc::ct::AttributeBlock *attributeBlock = decl->getAttributeBlock();
	if (!attributeBlock)
		return false;

	QTreeWidgetItem *attributes = insertItem("attributes", parent);

	sl::Iterator<jnc::ct::Attribute> attribute = attributeBlock->getAttributeList().getHead();
	for (; attribute; attribute++)
	{
		QTreeWidgetItem *treeItem = insertItem ((const char*) attribute->getName (), attributes);
		treeItem->setData(0, Qt::UserRole, qVariantFromValue ((void*) *attribute));
	}

	expandItem(attributes);
	return true;
}

void ModulePane::addNamespace(QTreeWidgetItem *parent,
	jnc::ct::GlobalNamespace *globalNamespace)
{
	jnc::ct::ModuleItemKind itemKind = globalNamespace->getItemKind();

	QTreeWidgetItem *treeItem = 0;

	if (itemKind == jnc::ct::ModuleItemKind_Scope)
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
		jnc::ct::ModuleItem *child = globalNamespace->getItem(i);
		addItem(treeItem, child);
	}

	expandItem(treeItem);
}

void ModulePane::addItem(QTreeWidgetItem *parent, jnc::ct::ModuleItem *item)
{
	jnc::ct::ModuleItemKind itemKind = item->getItemKind();

	QString name;
	QTreeWidgetItem* treeItem;

	switch (itemKind)
	{
	case jnc::ct::ModuleItemKind_Namespace:
		addNamespace (parent, (jnc::ct::GlobalNamespace*) item);
		break;

	case jnc::ct::ModuleItemKind_Type:
		addType (parent, (jnc::ct::Type*) item);
		break;

	case jnc::ct::ModuleItemKind_Typedef:
		addTypedef (parent, (jnc::ct::Typedef*) item);
		break;

	case jnc::ct::ModuleItemKind_Variable:
		addVariable (parent, (jnc::ct::Variable*) item);
		break;

	case jnc::ct::ModuleItemKind_Function:
		addFunction (parent, (jnc::ct::Function*) item);
		break;

	case jnc::ct::ModuleItemKind_Property:
		addProperty (parent, (jnc::ct::Property*) item);
		break;

	case jnc::ct::ModuleItemKind_EnumConst:
		addEnumConst (parent, (jnc::ct::EnumConst*) item);
		break;

	case jnc::ct::ModuleItemKind_StructField:
		addStructField (parent, (jnc::ct::StructField*) item);
		break;

	case jnc::ct::ModuleItemKind_Lazy:
		// don't display lazy items
		break;

	default:
		name.sprintf("item %p of kind %d", item, itemKind);

		treeItem = insertItem(name, parent);
		treeItem->setText(0, name);
		treeItem->setData(0, Qt::UserRole, qVariantFromValue((void*) item));
	}
}

void ModulePane::addType(QTreeWidgetItem *parent, jnc::ct::Type *type)
{
	QString itemName = (const char *)type->getTypeString();

	QTreeWidgetItem *item = insertItem(itemName, parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void *)type));

	QString toolTip = QString ("%1 (sizeof = %2)").arg (type->getTypeString ().cc ()).arg (type->getSize ());
	item->setToolTip (0, toolTip);

	addItemAttributes(item, type);

	jnc::ct::TypeKind typeKind = type->getTypeKind();
	switch (typeKind)
	{
	case jnc::ct::TypeKind_Enum:
		addEnumTypeMembers(item, (jnc::ct::EnumType *)type);
		break;

	case jnc::ct::TypeKind_Struct:
	case jnc::ct::TypeKind_Union:
		addDerivableTypeMembers(item, (jnc::ct::StructType *)type);
		break;

	case jnc::ct::TypeKind_Class:
		addClassTypeMembers(item, (jnc::ct::ClassType *)type);
		break;
	}
}

void ModulePane::addTypedef (QTreeWidgetItem *parent, jnc::ct::Typedef* typed)
{
	QString name;
	name.sprintf (
		"typedef %s %s",
		typed->getType ()->getTypeString ().cc (), // thanks a lot gcc
		typed->getName ().cc ()
		);

	QTreeWidgetItem *item = insertItem (name, parent);
	item->setData (0, Qt::UserRole, qVariantFromValue((void *) (jnc::ct::ModuleItem*) typed));
}

void ModulePane::addVariable(QTreeWidgetItem *parent, jnc::ct::Variable *variable)
{
	addValue (parent, variable->getName().cc (), variable->getType(), variable);
}

void ModulePane::addEnumConst(QTreeWidgetItem *parent, jnc::ct::EnumConst *member)
{
	QTreeWidgetItem *item = insertItem((const char *)member->getName(), parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void *)member));
}

void ModulePane::addValue (QTreeWidgetItem *parent, const char* name, jnc::ct::Type* type, jnc::ct::ModuleItem* moduleItem)
{
	QString itemName;
	itemName.sprintf ("%s %s", type->getTypeString ().cc (), name);
	QTreeWidgetItem *item = insertItem(itemName, parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void *) moduleItem));

	if (jnc::ct::isClassType (type, jnc::ct::ClassTypeKind_Reactor))
		addClassTypeMembers (item, (jnc::ct::ClassType*) type);
}

void ModulePane::addEnumTypeMembers (QTreeWidgetItem *parent, jnc::ct::EnumType* type)
{
	sl::Iterator <jnc::ct::EnumConst> Member = type->getConstList ().getHead ();
	for (; Member; Member++)
		addEnumConst (parent, *Member);

	expandItem (parent);
}

void ModulePane::addDerivableTypeMembers(QTreeWidgetItem *parent, jnc::ct::DerivableType *pType)
{
	sl::Array <jnc::ct::StructField*> FieldArray = pType->getMemberFieldArray ();
	size_t Count = FieldArray.getCount ();
	for (size_t i = 0; i < Count; i++)
	{
		jnc::ct::StructField* pField = FieldArray [i];
		addStructField (parent, pField);
	}

	if (pType->getStaticConstructor ())
		addItem (parent, pType->getStaticConstructor ());

	if (pType->getStaticDestructor ())
		addItem (parent, pType->getStaticDestructor ());

	if (pType->getPreConstructor ())
		addItem (parent, pType->getPreConstructor ());

	if (pType->getConstructor ())
		addItem (parent, pType->getConstructor ());

	sl::Array <jnc::ct::Property*> PropertyArray = pType->getMemberPropertyArray ();
	Count = PropertyArray.getCount ();
	for (size_t i = 0; i < Count; i++)
	{
		jnc::ct::Property* pProp = PropertyArray [i];
		addProperty (parent, pProp);
	}

	sl::Array <jnc::ct::Function*> FunctionArray = pType->getMemberMethodArray ();
	Count = FunctionArray.getCount ();
	for (size_t i = 0; i < Count; i++)
	{
		jnc::ct::Function* pFunction = FunctionArray [i];
		addFunction (parent, pFunction);
	}

	expandItem (parent);
}

void ModulePane::addClassTypeMembers (QTreeWidgetItem *parent, jnc::ct::ClassType* pType)
{
	if (pType->getDestructor ())
		addItem (parent, pType->getDestructor ());

	addDerivableTypeMembers (parent, pType);
}

void ModulePane::addFunction (QTreeWidgetItem *parent, jnc::ct::Function* pFunction)
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
			jnc::ct::Function* pOverload = pFunction->getOverload (i);
			addFunctionImpl (item, pOverload);
		}

		expandItem (item);
	}
}

void ModulePane::addFunctionImpl (QTreeWidgetItem *parent, jnc::ct::Function* pFunction)
{
	jnc::ct::FunctionType* pType = pFunction->getType ();

	sl::String Name = pFunction->getFunctionKind () == jnc::ct::FunctionKind_Named ?
		pFunction->getName () :
		sl::String (jnc::ct::getFunctionKindString (pFunction->getFunctionKind ()));

	QString itemName;
	itemName.sprintf (
		"%s%s %s %s",
		pType->getReturnType ()->getTypeString ().cc (),
		pType->getTypeModifierString ().cc (),
		Name.cc (),
		pFunction->getType ()->getArgString ().cc ()
		);

	QTreeWidgetItem *item = insertItem (itemName, parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void *)(jnc::ct::ModuleItem*) pFunction));
}

void ModulePane::addProperty (QTreeWidgetItem *parent, jnc::ct::Property* pProp)
{
	QTreeWidgetItem *item = insertItem (pProp->m_tag.cc (), parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void *)(jnc::ct::ModuleItem*) pProp));

	jnc::ct::Function* pGetter = pProp->getGetter ();
	jnc::ct::Function* pSetter = pProp->getSetter ();

	addFunction (item, pGetter);

	if (pSetter)
		addFunction (item, pSetter);

	expandItem (item);
}
