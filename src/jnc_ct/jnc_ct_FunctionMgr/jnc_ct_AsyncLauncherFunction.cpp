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
#include "jnc_ct_AsyncLauncherFunction.h"
#include "jnc_ct_AsyncSequencerFunction.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_Parser.llk.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
AsyncLauncherFunction::compile() {
	bool result;

	// prepare promise class type

	sl::String qualifiedName = getQualifiedName();
	sl::String promiseName = qualifiedName + ".Promise";
	sl::String sequencerName = qualifiedName + ".sequencer";

	ClassType* promiseType = m_module->m_typeMgr.createInternalClassType(promiseName);
	promiseType->addBaseType(m_module->m_typeMgr.getStdType(StdType_Promise));

	if (isMember())
		promiseType->createField(m_thisType);

	// compile launcher

	m_module->m_unitMgr.setCurrentUnit(m_parentUnit);

	m_module->m_functionMgr.prologue(this, m_bodyPos);
	m_module->m_namespaceMgr.getCurrentScope()->getUsingSet()->append(&m_usingSet);

	sl::Array<Variable*> argVariableArray = m_module->m_variableMgr.getArgVariableArray();
	size_t argCount = argVariableArray.getCount();
	for (size_t i = 0; i < argCount; i++) {
		Variable* argVar = argVariableArray[i];
		promiseType->createField(argVar->getName(), argVar->getType());
	}

	result = promiseType->ensureLayout();
	ASSERT(result);

	const sl::Array<Field*>& argFieldArray = promiseType->getFieldArray();
	ASSERT(argFieldArray.getCount() == (isMember() ? argCount + 1 : argCount));

	Value promiseValue;
	result = m_module->m_operatorMgr.newOperator(promiseType, &promiseValue);
	ASSERT(result);

	size_t j = 0;

	if (isMember()) {
		Field* argField = argFieldArray[0];
		Value argFieldValue;

		result = m_module->m_operatorMgr.getField(promiseValue, argField, &argFieldValue);
		ASSERT(result);

		result = m_module->m_operatorMgr.storeDataRef(argFieldValue, m_module->m_functionMgr.getThisValue());
		ASSERT(result);

		j = 1;
	}

	for (size_t i = 0; i < argCount; i++, j++) {
		Variable* argVar = argVariableArray[i];
		Field* argField = argFieldArray[j];
		Value argFieldValue;

		result = m_module->m_operatorMgr.getField(promiseValue, argField, &argFieldValue);
		ASSERT(result);

		result = m_module->m_operatorMgr.storeDataRef(argFieldValue, argVar);
		ASSERT(result);
	}

	Type* argType = promiseType->getClassPtrType(ClassPtrTypeKind_Normal, PtrTypeFlag_Safe);
	uint_t flags = m_type->getFlags() & FunctionTypeFlag_AsyncErrorCode;

	FunctionType* functionType = m_module->m_typeMgr.getFunctionType(&argType, 1, flags);

	AsyncSequencerFunction* sequencerFunc = m_module->m_functionMgr.createFunction<AsyncSequencerFunction>(
		sl::String(),
		sequencerName,
		functionType
	);

	m_module->m_functionMgr.m_asyncSequencerFunctionArray.append(sequencerFunc);

	sequencerFunc->m_asyncLauncher = this;
	sequencerFunc->m_parentUnit = m_parentUnit;
	sequencerFunc->m_parentNamespace = m_parentNamespace;
	sequencerFunc->m_thisArgType = m_thisArgType;
	sequencerFunc->m_thisType = m_thisType;
	sequencerFunc->m_thisArgDelta = m_thisArgDelta;
	sequencerFunc->setBody(m_pragmaSettings, m_bodyPos, m_body);

	m_module->m_operatorMgr.callOperator(sequencerFunc, promiseValue);

	result = m_module->m_controlFlowMgr.ret(promiseValue);
	ASSERT(result);

	return m_module->m_functionMgr.epilogue();
}

bool
AsyncLauncherFunction::generateCodeAssist() {
	ASSERT(isClassPtrType(m_type->getReturnType(), (ClassType*)m_module->m_typeMgr.getStdType(StdType_Promise))); // should only be called once

	m_type = m_module->m_typeMgr.getFunctionType(
		m_type->getAsyncReturnType(),
		m_type->getArgArray()
	);

	return Function::compile();
}

//..............................................................................

} // namespace ct
} // namespace jnc
