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
#include "jnc_ct_MulticastClassType.h"
#include "jnc_ct_Module.h"
#include "jnc_rt_Runtime.h"

namespace jnc {
namespace ct {

//..............................................................................

MulticastClassType::MulticastClassType() {
	m_classTypeKind = ClassTypeKind_Multicast;
	m_namespaceStatus = NamespaceStatus_Ready;
	m_targetType = NULL;
	m_snapshotType = NULL;
	m_eventClassPtrTypeTuple = NULL;
	memset(m_fieldArray, 0, sizeof(m_fieldArray));
	memset(m_methodArray, 0, sizeof(m_methodArray));
}

void
MulticastClassType::prepareTypeString() {
	TypeStringTuple* tuple = getTypeStringTuple();
	tuple->m_typeStringPrefix = m_targetType->getTypeModifierString() + " multicast";
	tuple->m_typeStringSuffix = m_targetType->getTargetType()->getTypeStringSuffix();
}

void
MulticastClassType::prepareDoxyLinkedText() {
	TypeStringTuple* tuple = getTypeStringTuple();
	tuple->m_doxyLinkedTextPrefix = m_targetType->getTypeModifierString() + " multicast";
	tuple->m_doxyLinkedTextSuffix = m_targetType->getTargetType()->getDoxyLinkedTextSuffix();
}

void
MulticastClassType::prepareDoxyTypeString() {
	Type::prepareDoxyTypeString();
	m_targetType->getTargetType()->appendDoxyArgString(&getTypeStringTuple()->m_doxyTypeString);
}

bool
MulticastClassType::calcLayout() {
	bool result =
		ClassType::calcLayout() &&
		m_snapshotType->ensureLayout();

	if (!result)
		return false;

	// we also need to explicitly mark call methods for compile:
	// [1] Multicast.call may be never explicitly called for events --
	// e.g., we may generate events from C/C++ and not from Jancy;
	// [2] McSnapshot.call is never called directly -- it's referenced from RTL
	// in MulticastImpl::getSnapshot

	m_module->markForCompile(m_methodArray[MulticastMethodKind_Call]);
	m_module->markForCompile(m_snapshotType->getMethodArray()[McSnapshotMethodKind_Call]);
	return true;
}

bool
MulticastClassType::compileCallMethod(Function* function) {
	ASSERT(function == m_methodArray[MulticastMethodKind_Call]);

	bool result = m_methodArray[MulticastMethodKind_Call]->getType()->ensureLayout();
	if (!result)
		return false;

	char buffer[256];
	sl::Array<Value> argValueArray(rc::BufKind_Stack, buffer, sizeof(buffer));

	size_t argCount = function->getType()->getArgArray().getCount();
	argValueArray.setCount(argCount);

	m_module->m_functionMgr.internalPrologue(function, argValueArray.p(), argCount);

	Function* getSnapshot = m_methodArray[MulticastMethodKind_GetSnapshot];

	Value snapshotValue;
	result = m_module->m_operatorMgr.callOperator(getSnapshot, argValueArray[0], &snapshotValue);
	if (!result)
		return false;

	sl::BoxList<Value> argList;
	for (size_t i = 1; i < argCount; i++)
		argList.insertTail(argValueArray[i]);

	m_module->m_operatorMgr.callOperator(snapshotValue, &argList);
	m_module->m_controlFlowMgr.ret();
	m_module->m_functionMgr.internalEpilogue();
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
