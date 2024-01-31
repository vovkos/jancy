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

#pragma once

class MdiChild;

class ModulePane : public QTreeWidget {
	Q_OBJECT

public:
	ModulePane(QWidget *parent);

	QSize sizeHint() const { return QSize(300, 50); }

	bool build(jnc::Module* module, MdiChild* document);
	void clear();

private slots:
	void onItemDoubleClicked(QTreeWidgetItem* treeItem, int column);

private:
	QTreeWidgetItem *insertItem(const QString& text, QTreeWidgetItem* parent = 0);

	bool addItemAttributes(QTreeWidgetItem* parent, jnc::ModuleItemDecl* decl);
	void addNamespace(QTreeWidgetItem* parent, jnc::GlobalNamespace* globalNamespace);
	void addItem(QTreeWidgetItem* parent, jnc::ModuleItem* item);
	void addType(QTreeWidgetItem* parent, jnc::Type* type);
	void addTypedef(QTreeWidgetItem* parent, jnc::Typedef* typed);
	void addEnumConst(QTreeWidgetItem* parent, jnc::EnumConst* member);
	void addValue(QTreeWidgetItem* parent, const QString& name, jnc::Type* type, uint_t ptrTypeFlags, jnc::ModuleItem* item);
	void addFunction(QTreeWidgetItem* parent, jnc::Function* function);
	void addFunctionOverload(QTreeWidgetItem* parent, jnc::FunctionOverload* overload);
	void addOverloadableFunction(QTreeWidgetItem* parent, jnc::OverloadableFunction function);
	void addProperty(QTreeWidgetItem* parent, jnc::Property* prop);
	void addAlias(QTreeWidgetItem* parent, jnc::Alias* alias);
	void addEnumTypeMembers(QTreeWidgetItem* parent, jnc::EnumType* type);
	void addDerivableTypeMembers(QTreeWidgetItem* parent, jnc::DerivableType* type);

	void addVariable(QTreeWidgetItem* parent, jnc::Variable* variable) {
		addValue(parent, variable->getDecl()->getName(), variable->getType(), variable->getPtrTypeFlags(), variable);
	}

	void addField(QTreeWidgetItem* parent, jnc::Field* field) {
		addValue(parent, field->getDecl()->getName(), field->getType(), field->getPtrTypeFlags(), field);
	}

protected:
	MdiChild* m_document;
};
