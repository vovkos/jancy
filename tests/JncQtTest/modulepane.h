#ifndef _MODULEPANE_H
#define _MODULEPANE_H

class MdiChild;

class ModulePane : public QTreeWidget
{
	Q_OBJECT

public:
	ModulePane(QWidget *parent);
	
	QSize sizeHint() const { return QSize(300, 50); }

	bool build(jnc::CModule *module, MdiChild *document);
	void clear();

private slots:
	void onItemDoubleClicked(QTreeWidgetItem *treeItem, int column);

private:
	QTreeWidgetItem *insertItem(const QString &text,
		QTreeWidgetItem *parent = 0, void *data = 0);

	bool addItemAttributes(QTreeWidgetItem *parent, jnc::CModuleItem *item);

	void addNamespace(QTreeWidgetItem *parent, jnc::CGlobalNamespace *globalNamespace);
	void addItem(QTreeWidgetItem *parent, jnc::CModuleItem *item);
	void addType(QTreeWidgetItem *parent, jnc::CType *type);
	void addTypedef(QTreeWidgetItem *parent, jnc::CTypedef *typed);
	void addVariable(QTreeWidgetItem *parent, jnc::CVariable *variable);
	void addEnumConst(QTreeWidgetItem *parent, jnc::CEnumConst *member);
	void addValue(QTreeWidgetItem *parent, const char* name, jnc::CType *type, jnc::CModuleItem *item);
	void addFunction(QTreeWidgetItem *parent, jnc::CFunction *function);
	void addFunctionImpl(QTreeWidgetItem *parent, jnc::CFunction *function);
	void addProperty(QTreeWidgetItem *parent, jnc::CProperty *prop);
	void addEnumTypeMembers(QTreeWidgetItem *parent, jnc::CEnumType *type);
	void addStructTypeMembers(QTreeWidgetItem *parent, jnc::CStructType *type);
	
	void addStructField(QTreeWidgetItem *parent, jnc::CStructField *field)
	{
		addValue (parent, field->GetName (), field->GetType (), field);
	}

	void addUnionTypeMembers(QTreeWidgetItem *parent, jnc::CUnionType *type);
	void addClassTypeMembers(QTreeWidgetItem *parent, jnc::CClassType *type);

	MdiChild *document;
};

#endif