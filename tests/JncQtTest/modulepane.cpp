#include "pch.h"
#include "modulepane.h"
#include "mdichild.h"
#include "moc_modulepane.cpp"

ModulePane::ModulePane(QWidget *parent)
	: QTreeWidget(parent)
{
	document = 0;

	setColumnCount(1);
	header()->hide();

	QObject::connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
		this, SLOT(onItemDoubleClicked(QTreeWidgetItem *, int)));
}

bool ModulePane::build(jnc::CModule *module, MdiChild *document)
{
	clear();

	jnc::CGlobalNamespace *globalNamespace =
		module->m_NamespaceMgr.GetGlobalNamespace();
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

	jnc::CModuleItem *item = (jnc::CModuleItem *)variant.value<void *>();
	if(!item)
		return;

	jnc::CModuleItemDecl* decl = item->GetItemDecl ();
	if (!decl)
		return;

	document->selectLine(decl->GetPos ()->m_Line, true);
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
	jnc::CModuleItem *item)
{
	jnc::CModuleItemDecl* decl = item->GetItemDecl ();
	if (!decl)
		return false;

	jnc::CAttributeBlock *attributeBlock = decl->GetAttributeBlock();
	if (!attributeBlock)
		return false;

	QTreeWidgetItem *attributes = insertItem("attributes", parent);

	QString itemName;

	rtl::CIteratorT<jnc::CAttribute> attribute = attributeBlock->GetAttributeList().GetHead();
	for (; attribute; attribute++)
	{
		jnc::CValue* pValue = attribute->GetValue();

		if (pValue)
			itemName.sprintf("%s = %s");
		else
			itemName = (const char *)attribute->GetName();

		QTreeWidgetItem *treeItem = insertItem(itemName, attributes);
		treeItem->setData(0, Qt::UserRole, qVariantFromValue((void *)*attribute));
	}

	expandItem(attributes);
	return true;
}

void ModulePane::addNamespace(QTreeWidgetItem *parent,
	jnc::CGlobalNamespace *globalNamespace)
{
	jnc::EModuleItem itemKind = globalNamespace->GetItemKind();

	QTreeWidgetItem *treeItem = 0;

	if (itemKind == jnc::EModuleItem_Scope)
	{
		treeItem = insertItem("scope", parent);
	}
	else if (!globalNamespace->GetParentNamespace())
	{
		treeItem = insertItem("global", parent);
	}
	else
	{
		QString text;
		text.sprintf("namespace %s",
			(const char *)globalNamespace->GetName());

		treeItem = insertItem(text, parent);
	}

	treeItem->setData(0, Qt::UserRole,
		qVariantFromValue((void *)globalNamespace));

	size_t count = globalNamespace->GetItemCount();
	for (size_t i = 0; i < count; i++) {
		jnc::CModuleItem *child = globalNamespace->GetItem(i);
		addItem(treeItem, child);
	}

	expandItem(treeItem);
}

void ModulePane::addItem(QTreeWidgetItem *parent, jnc::CModuleItem *item)
{
	jnc::EModuleItem itemKind = item->GetItemKind();

	QString name;
	QTreeWidgetItem* treeItem;

	switch (itemKind)
	{
	case jnc::EModuleItem_Namespace:
		addNamespace (parent, (jnc::CGlobalNamespace*) item);
		break;

	case jnc::EModuleItem_Type:
		addType (parent, (jnc::CType*) item);
		break;

	case jnc::EModuleItem_Typedef:
		addTypedef (parent, (jnc::CTypedef*) item);
		break;

	case jnc::EModuleItem_Variable:
		addVariable (parent, (jnc::CVariable*) item);
		break;

	case jnc::EModuleItem_Function:
		addFunction (parent, (jnc::CFunction*) item);
		break;

	case jnc::EModuleItem_Property:
		addProperty (parent, (jnc::CProperty*) item);
		break;

	case jnc::EModuleItem_EnumConst:
		addEnumConst (parent, (jnc::CEnumConst*) item);
		break;

	case jnc::EModuleItem_StructField:
		addStructField (parent, (jnc::CStructField*) item);
		break;

	case jnc::EModuleItem_Lazy:
		name.sprintf("lazy: %s", ((jnc::CLazyModuleItem*) item)->GetName ().cc ());

		treeItem = insertItem(name, parent);
		treeItem->setText(0, name);
		treeItem->setData(0, Qt::UserRole, qVariantFromValue((void*) item));
		break;

	default:
		name.sprintf("item %x of kind %d", item, itemKind);

		treeItem = insertItem(name, parent);
		treeItem->setText(0, name);
		treeItem->setData(0, Qt::UserRole, qVariantFromValue((void*) item));
	}
}

void ModulePane::addType(QTreeWidgetItem *parent, jnc::CType *type)
{
	QString itemName = (const char *)type->GetTypeString();

	QTreeWidgetItem *item = insertItem(itemName, parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void *)type));

	addItemAttributes(item, type);

	jnc::EType typeKind = type->GetTypeKind();
	switch (typeKind)
	{
	case jnc::EType_Enum:
		addEnumTypeMembers(item, (jnc::CEnumType *)type);
		break;

	case jnc::EType_Struct:
		addStructTypeMembers(item, (jnc::CStructType *)type);
		break;

	case jnc::EType_Union:
		addUnionTypeMembers(item, (jnc::CUnionType *)type);
		break;

	case jnc::EType_Class:
		addClassTypeMembers(item, (jnc::CClassType *)type);
		break;
	}
}

void ModulePane::addTypedef (QTreeWidgetItem *parent, jnc::CTypedef* typed)
{
	QString name;
	name.sprintf (
		"typedef %s %s",
		typed->GetType ()->GetTypeString ().cc (), // thanks a lot gcc
		typed->GetName ().cc ()
		);

	QTreeWidgetItem *item = insertItem (name, parent);
	item->setData (0, Qt::UserRole, qVariantFromValue((void *) (jnc::CModuleItem*) typed));
}

void ModulePane::addVariable(QTreeWidgetItem *parent, jnc::CVariable *variable)
{
	addValue (parent, variable->GetName().cc (), variable->GetType(), variable);
}

void ModulePane::addEnumConst(QTreeWidgetItem *parent, jnc::CEnumConst *member)
{
	QTreeWidgetItem *item = insertItem((const char *)member->GetName(), parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void *)member));
}

void ModulePane::addValue (QTreeWidgetItem *parent, const char* name, jnc::CType* type, jnc::CModuleItem* moduleItem)
{
	QString itemName;
	itemName.sprintf ("%s %s", type->GetTypeString ().cc (), name);
	QTreeWidgetItem *item = insertItem(itemName, parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void *) moduleItem));

	if (jnc::IsClassType (type, jnc::EClassType_Reactor))
		addClassTypeMembers (item, (jnc::CClassType*) type);
}

void ModulePane::addEnumTypeMembers (QTreeWidgetItem *parent, jnc::CEnumType* type)
{
	rtl::CIteratorT <jnc::CEnumConst> Member = type->GetConstList ().GetHead ();
	for (; Member; Member++)
		addEnumConst (parent, *Member);

	expandItem (parent);
}

void ModulePane::addStructTypeMembers (QTreeWidgetItem *parent, jnc::CStructType* pType)
{
	rtl::CIteratorT <jnc::CStructField> Member = pType->GetFieldList ().GetHead ();
	for (; Member; Member++)
		addStructField (parent, *Member);

	expandItem (parent);
}

void ModulePane::addUnionTypeMembers (QTreeWidgetItem *parent, jnc::CUnionType* pType)
{
	rtl::CIteratorT <jnc::CStructField> Member = pType->GetFieldList ().GetHead ();
	for (; Member; Member++)
		addStructField (parent, *Member);

	expandItem (parent);
}

void ModulePane::addClassTypeMembers (QTreeWidgetItem *parent, jnc::CClassType* pType)
{
	rtl::CIteratorT <jnc::CStructField> Member = pType->GetFieldList ().GetHead ();
	for (; Member; Member++)
		addStructField (parent, *Member);

	if (pType->GetPreConstructor ())
		addItem (parent, pType->GetPreConstructor ());

	if (pType->GetConstructor ())
		addItem (parent, pType->GetConstructor ());

	if (pType->GetDestructor ())
		addItem (parent, pType->GetDestructor ());

	rtl::CArrayT <jnc::CProperty*> PropertyArray = pType->GetMemberPropertyArray ();
	size_t Count = PropertyArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		jnc::CProperty* pProperty = PropertyArray [i];
		addProperty (parent, pProperty);
	}

	rtl::CArrayT <jnc::CFunction*> FunctionArray = pType->GetMemberMethodArray ();
	Count = FunctionArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		jnc::CFunction* pFunction = FunctionArray [i];
		addFunction (parent, pFunction);
	}

	expandItem (parent);
}

void ModulePane::addFunction (QTreeWidgetItem *parent, jnc::CFunction* pFunction)
{
	if (!pFunction->IsOverloaded ())
	{
		addFunctionImpl (parent, pFunction);
	}
	else
	{
		size_t count = pFunction->GetOverloadCount ();

		QString itemName;
		itemName.sprintf (
			"%s (%d overloads)",
			pFunction->m_Tag.cc (),
			count
			);

		QTreeWidgetItem *item = insertItem (itemName, parent);
		for (size_t i = 0; i < count; i++)
		{
			jnc::CFunction* pOverload = pFunction->GetOverload (i);
			addFunctionImpl (item, pOverload);
		}

		expandItem (item);
	}
}

void ModulePane::addFunctionImpl (QTreeWidgetItem *parent, jnc::CFunction* pFunction)
{
	jnc::CFunctionType* pType = pFunction->GetType ();

	rtl::CString Name = pFunction->GetFunctionKind () == jnc::EFunction_Named ?
		pFunction->GetName () :
		rtl::CString (jnc::GetFunctionKindString (pFunction->GetFunctionKind ()));

	QString itemName;
	itemName.sprintf (
		"%s%s %s %s",
		pType->GetTypeModifierString ().cc (),
		pType->GetReturnType ()->GetTypeString ().cc (),
		Name.cc (),
		pFunction->GetType ()->GetArgString ().cc ()
		);

	QTreeWidgetItem *item = insertItem (itemName, parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void *)(jnc::CModuleItem*) pFunction));
}

void ModulePane::addProperty (QTreeWidgetItem *parent, jnc::CProperty* pProperty)
{
	QTreeWidgetItem *item = insertItem (pProperty->m_Tag.cc (), parent);
	item->setData(0, Qt::UserRole, qVariantFromValue((void *)(jnc::CModuleItem*) pProperty));

	jnc::CFunction* pGetter = pProperty->GetGetter ();
	jnc::CFunction* pSetter = pProperty->GetSetter ();

	addFunction (item, pGetter);

	if (pSetter)
		addFunction (item, pSetter);

	expandItem (item);
}
