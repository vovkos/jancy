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
#include "jnc_ct_MemberBlock.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

MemberBlock::MemberBlock(ModuleItem* parent) {
	m_parent = parent;
	m_staticConstructorOnceFlagVariable = NULL;
	m_staticConstructor = NULL;
	m_destructor = NULL;
}

Namespace*
MemberBlock::getParentNamespaceImpl() {
	ASSERT(
		m_parent->getItemKind() == ModuleItemKind_Property ||
		m_parent->getItemKind() == ModuleItemKind_Type &&
		(((Type*)m_parent)->getTypeKindFlags() & TypeKindFlag_Derivable));

	return m_parent->getItemKind() == ModuleItemKind_Property ?
		(Namespace*)(Property*)m_parent :
		(Namespace*)(DerivableType*)m_parent;
}

Unit*
MemberBlock::getParentUnitImpl() {
	ASSERT(
		m_parent->getItemKind() == ModuleItemKind_Property ||
		m_parent->getItemKind() == ModuleItemKind_Type &&
		(((Type*)m_parent)->getTypeKindFlags() & TypeKindFlag_Derivable));

	return m_parent->getItemKind() == ModuleItemKind_Property ?
		((Property*)m_parent)->getParentUnit() :
		((DerivableType*)m_parent)->getParentUnit();
}

void
MemberBlock::scanStaticVariables() {
	size_t count = m_staticVariableArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Variable* variable = m_staticVariableArray[i];

		if (variable->getType()->getTypeKind() == TypeKind_Class)
			m_staticVariablePrimeArray.append(variable);

		if (variable->hasInitializer() ||
			isConstructibleType(variable->getType()))
			m_staticVariableInitializeArray.append(variable);
	}
}

void
MemberBlock::scanPropertyCtorDtors() {
	size_t count = m_propertyArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Property* prop = m_propertyArray[i];

		if (prop->getStaticConstructor())
			m_propertyStaticConstructArray.append(prop);

		if (prop->getConstructor())
			m_propertyConstructArray.append(prop);

		if (prop->getDestructor())
			m_propertyDestructArray.append(prop);
	}
}

bool
MemberBlock::callStaticConstructor() {
	if (!m_staticConstructor)
		return true;

	Module* module = m_parent->getModule();

	if (!m_staticConstructorOnceFlagVariable)
		m_staticConstructorOnceFlagVariable = module->m_variableMgr.createOnceFlagVariable();

	OnceStmt stmt;
	lex::LineCol pos = module->m_namespaceMgr.getSourcePos();

	module->m_controlFlowMgr.onceStmt_Create(&stmt, m_staticConstructorOnceFlagVariable);
	module->m_controlFlowMgr.onceStmt_PreBody(&stmt, pos);

	bool result = module->m_operatorMgr.callOperator(m_staticConstructor);
	if (!result)
		return false;

	module->m_controlFlowMgr.onceStmt_PostBody(&stmt);
	return true;
}

void
MemberBlock::primeStaticVariables() {
	Module* module = m_parent->getModule();

	size_t count = m_staticVariablePrimeArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Variable* variable = m_staticVariablePrimeArray[i];
		ASSERT(variable->getStorageKind() == StorageKind_Static && variable->getType()->getTypeKind() == TypeKind_Class);
		module->m_variableMgr.primeStaticClassVariable(variable);
	}
}

bool
MemberBlock::initializeStaticVariables() {
	bool result;

	Module* module = m_parent->getModule();

	Unit* unit = getParentUnitImpl();
	if (unit)
		module->m_unitMgr.setCurrentUnit(unit);

	size_t count = m_staticVariableInitializeArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Variable* variable = m_staticVariableInitializeArray[i];
		if (variable->m_flags & ModuleItemFlag_Constructed) {
			variable->m_flags &= ~ModuleItemFlag_Constructed;
			continue;
		}

		result = module->m_variableMgr.initializeVariable(variable);
		if (!result)
			return false;
	}

	return true;
}

bool
MemberBlock::initializeFields(const Value& thisValue) {
	if (m_fieldInitializeArray.isEmpty()) // shortcut
		return true;

	Module* module = m_parent->getModule();

	Unit* unit = getParentUnitImpl();
	if (unit)
		module->m_unitMgr.setCurrentUnit(unit);

	Type* parentType = thisValue.getType();
	if (parentType->getTypeKindFlags() & TypeKindFlag_DataPtr)
		parentType = ((DataPtrType*)parentType)->getTargetType();
	else if (parentType->getTypeKindFlags() & TypeKindFlag_ClassPtr)
		parentType = ((ClassPtrType*)parentType)->getTargetType();

	size_t count = m_fieldInitializeArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Field* field = m_fieldInitializeArray[i];
		if (field->m_flags & ModuleItemFlag_Constructed) {
			field->m_flags &= ~ModuleItemFlag_Constructed;
			continue;
		}

		Value fieldValue;
		bool result = module->m_operatorMgr.getField(thisValue, parentType, field, &fieldValue);
		if (!result)
			return false;

		result = module->m_operatorMgr.parseInitializer(
			fieldValue,
			&field->m_constructor,
			&field->m_initializer
		);

		if (!result)
			return false;
	}

	return true;
}

bool
MemberBlock::callPropertyStaticConstructors() {
	bool result;

	Module* module = m_parent->getModule();

	size_t count = m_propertyStaticConstructArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Property* prop = m_propertyStaticConstructArray[i];
		if (prop->m_flags & ModuleItemFlag_Constructed) {
			prop->m_flags &= ~ModuleItemFlag_Constructed;
			continue;
		}

		Function* constructor = prop->getStaticConstructor();
		ASSERT(constructor);

		result = module->m_operatorMgr.callOperator(constructor);
		if (!result)
			return false;
	}

	return true;
}

bool
MemberBlock::callPropertyConstructors(const Value& thisValue) {
	bool result;

	Module* module = m_parent->getModule();

	size_t count = m_propertyConstructArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Property* prop = m_propertyConstructArray[i];
		if (prop->m_flags & ModuleItemFlag_Constructed) {
			prop->m_flags &= ~ModuleItemFlag_Constructed;
			continue;
		}

		OverloadableFunction constructor = prop->getConstructor();
		ASSERT(constructor);

		result = module->m_operatorMgr.callOperator(constructor, thisValue);
		if (!result)
			return false;
	}

	return true;
}

bool
MemberBlock::callPropertyDestructors(const Value& thisValue) {
	bool result;

	Module* module = m_parent->getModule();

	size_t count = m_propertyDestructArray.getCount();
	for (intptr_t i = count - 1; i >= 0; i--) {
		Function* destructor = m_propertyDestructArray[i]->getDestructor();
		ASSERT(destructor);

		result = module->m_operatorMgr.callOperator(destructor, thisValue);
		if (!result)
			return false;
	}

	return true;
}

bool
MemberBlock::addUnnamedMethod(
	Function* function,
	Function** targetFunction,
	OverloadableFunction* targetOverloadableFunction
) {
	if (targetFunction) {
		ASSERT(!targetOverloadableFunction);

		if (*targetFunction) {
			err::setFormatStringError("'%s' already exists", (*targetFunction)->getQualifiedName().sz());
			return false;
		}

		*targetFunction = function;
	} else {
		ASSERT(targetOverloadableFunction);

		if (!*targetOverloadableFunction) {
			*targetOverloadableFunction = function;
		} else {
			if ((*targetOverloadableFunction)->getItemKind() == ModuleItemKind_Function)
				*targetOverloadableFunction = function->getModule()->m_functionMgr.createFunctionOverload(targetOverloadableFunction->getFunction());

			bool result = targetOverloadableFunction->getFunctionOverload()->addOverload(function) != -1;
			if (!result)
				return false;
		}
	}

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
