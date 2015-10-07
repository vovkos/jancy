#ifndef _MODULEPANE_H
#define _MODULEPANE_H

class MdiChild;

class ModulePane : public QTreeWidget
{
	Q_OBJECT

public:
	ModulePane(QWidget *parent);
	
	QSize sizeHint() const { return QSize(300, 50); }

	bool build(jnc::ct::Module *module, MdiChild *document);
	void clear();

private slots:
	void onItemDoubleClicked(QTreeWidgetItem *treeItem, int column);

private:
	QTreeWidgetItem *insertItem(const QString &text,
		QTreeWidgetItem *parent = 0, void *data = 0);

	bool addItemAttributes(QTreeWidgetItem *parent, jnc::ct::ModuleItem *item);

	void addNamespace(QTreeWidgetItem *parent, jnc::ct::GlobalNamespace *globalNamespace);
	void addItem(QTreeWidgetItem *parent, jnc::ct::ModuleItem *item);
	void addType(QTreeWidgetItem *parent, jnc::ct::Type *type);
	void addTypedef(QTreeWidgetItem *parent, jnc::ct::Typedef *typed);
	void addVariable(QTreeWidgetItem *parent, jnc::ct::Variable *variable);
	void addEnumConst(QTreeWidgetItem *parent, jnc::ct::EnumConst *member);
	void addValue(QTreeWidgetItem *parent, const char* name, jnc::ct::Type *type, jnc::ct::ModuleItem *item);
	void addFunction(QTreeWidgetItem *parent, jnc::ct::Function *function);
	void addFunctionImpl(QTreeWidgetItem *parent, jnc::ct::Function *function);
	void addProperty(QTreeWidgetItem *parent, jnc::ct::Property *prop);
	void addEnumTypeMembers(QTreeWidgetItem *parent, jnc::ct::EnumType *type);
	void addClassTypeMembers(QTreeWidgetItem *parent, jnc::ct::ClassType *type);
	void addDerivableTypeMembers(QTreeWidgetItem *parent, jnc::ct::DerivableType *type);

	void addStructField(QTreeWidgetItem *parent, jnc::ct::StructField *field)
	{
		addValue (parent, field->getName (), field->getType (), field);
	}

	MdiChild *document;
};

#endif