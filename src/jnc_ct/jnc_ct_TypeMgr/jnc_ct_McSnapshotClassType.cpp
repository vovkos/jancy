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
#include "jnc_ct_McSnapshotClassType.h"
#include "jnc_ct_Module.h"
#include "jnc_rt_Runtime.h"

namespace jnc {
namespace ct {

//..............................................................................

McSnapshotClassType::McSnapshotClassType() {
	m_classTypeKind = ClassTypeKind_McSnapshot;
	m_namespaceStatus = NamespaceStatus_Ready;
	m_targetType = NULL;
	memset(m_fieldArray, 0, sizeof(m_fieldArray));
	memset(m_methodArray, 0, sizeof(m_methodArray));
}

void
McSnapshotClassType::prepareTypeString() {
	TypeStringTuple* tuple = getTypeStringTuple();
	tuple->m_typeStringPrefix = m_targetType->getTypeModifierString() + " mcsnapshot";
	tuple->m_typeStringSuffix = m_targetType->getTargetType()->getTypeStringSuffix();
}

void
McSnapshotClassType::prepareDoxyLinkedText() {
	TypeStringTuple* tuple = getTypeStringTuple();
	tuple->m_doxyLinkedTextPrefix = m_targetType->getTypeModifierString() + " mcsnapshot";
	tuple->m_doxyLinkedTextSuffix = m_targetType->getTargetType()->getDoxyLinkedTextSuffix();
}

void
McSnapshotClassType::prepareDoxyTypeString() {
	Type::prepareDoxyTypeString();
	m_targetType->getTargetType()->appendDoxyArgString(&getTypeStringTuple()->m_doxyTypeString);
}

bool
McSnapshotClassType::compileCallMethod(Function* function) {
	ASSERT(function == m_methodArray[McSnapshotMethodKind_Call]);

	bool result = function->getType()->ensureLayout();
	if (!result)
		return false;

	if (!m_module->hasCodeGen())
		return true;

	sl::Array<FunctionArg*> argArray = function->getType()->getArgArray();
	size_t argCount = argArray.getCount();

	char buffer[256];
	sl::Array<Value> argValueArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	argValueArray.setCount(argCount);

	m_module->m_functionMgr.internalPrologue(function, argValueArray, argCount);

	sl::BoxList<Value> argValueList;
	for (size_t i = 1; i < argCount; i++)
		argValueList.insertTail(argValueArray[i]);

	Type* ptrType = m_targetType->getDataPtrType_c();

	Value ptrVariable;
	Value ptrValue;
	Value ptrEndValue;

	StructType* ifaceStruct = getIfaceStructType();
	int32_t ptrGepIdxArray[] = { 0, 1, 0 };
	Type* ptrFieldType = m_fieldArray[McSnapshotFieldKind_PtrArray]->getType();

	m_module->m_llvmIrBuilder.createAlloca(ptrType, NULL, &ptrVariable);
	m_module->m_llvmIrBuilder.createGep(argValueArray[0], ifaceStruct, ptrGepIdxArray, countof(ptrGepIdxArray), NULL, &ptrValue);
	m_module->m_llvmIrBuilder.createLoad(ptrValue, ptrFieldType, &ptrValue);
	m_module->m_llvmIrBuilder.createBitCast(ptrValue, ptrType, &ptrValue);
	m_module->m_llvmIrBuilder.createStore(ptrValue, ptrVariable);

	Value countValue;

	int32_t countGepIdxArray[] = { 0, 2 };
	m_module->m_llvmIrBuilder.createGep(argValueArray[0], ifaceStruct, countGepIdxArray, countof(countGepIdxArray), NULL, &countValue);
	m_module->m_llvmIrBuilder.createLoad(countValue, countValue.getType(), &countValue);
	m_module->m_llvmIrBuilder.createGep(ptrValue, m_targetType, countValue, ptrType, &ptrEndValue);

	BasicBlock* conditionBlock = m_module->m_controlFlowMgr.createBlock("call_loop_cond");
	BasicBlock* bodyBlock = m_module->m_controlFlowMgr.createBlock("call_loop_body");
	BasicBlock* followBlock = m_module->m_controlFlowMgr.createBlock("call_loop_follow");

	m_module->m_controlFlowMgr.follow(conditionBlock);

	Value idxValue;
	Value cmpValue;

	m_module->m_llvmIrBuilder.createLoad(ptrVariable, ptrType, &ptrValue);
	m_module->m_llvmIrBuilder.createGe_u(ptrValue, ptrEndValue, &cmpValue);
	m_module->m_controlFlowMgr.conditionalJump(cmpValue, followBlock, bodyBlock, bodyBlock);

	Value pfnValue;

	m_module->m_llvmIrBuilder.createLoad(ptrValue, m_targetType, &pfnValue);
	m_module->m_operatorMgr.callOperator(pfnValue, &argValueList);

	m_module->m_llvmIrBuilder.createGep(ptrValue, m_targetType, 1, ptrType, &ptrValue);
	m_module->m_llvmIrBuilder.createStore(ptrValue, ptrVariable);
	m_module->m_controlFlowMgr.jump(conditionBlock, followBlock);

	m_module->m_functionMgr.internalEpilogue();

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
