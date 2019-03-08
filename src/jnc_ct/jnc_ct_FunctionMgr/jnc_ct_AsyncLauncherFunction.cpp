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
AsyncLauncherFunction::compile()
{
	bool result;

	// prepare promise class type

	sl::String qualifiedName = getQualifiedName();
	sl::String promiseName = qualifiedName + ".Promise";
	sl::String sequencerName = qualifiedName + ".sequencer";

	ClassType* promiseType = m_module->m_typeMgr.createClassType(sl::String(), promiseName);
	promiseType->addBaseType(m_module->m_typeMgr.getStdType(StdType_Promise));

	if (isMember())
		promiseType->createField(m_thisType);

	// compile launcher

	m_module->m_unitMgr.setCurrentUnit(m_parentUnit);

	Token::Pos beginPos = m_body.getHead()->m_pos;
	Token::Pos endPos = m_body.getTail()->m_pos;

	m_module->m_functionMgr.prologue(this, beginPos);
	m_module->m_namespaceMgr.getCurrentScope()->getUsingSet()->append(NULL, &m_usingSet);

	sl::Array<Variable*> argVariableArray = m_module->m_variableMgr.getArgVariableArray();
	size_t argCount = argVariableArray.getCount();

	for (size_t i = 0; i < argCount; i++)
	{
		Variable* argVar = argVariableArray[i];
		promiseType->createField(argVar->getName(), argVar->getType());
	}

	result = promiseType->ensureLayout();
	ASSERT(result);

	sl::Array<StructField*> argFieldArray = promiseType->getMemberFieldArray();
	ASSERT(argFieldArray.getCount() == (isMember() ? argCount + 1 : argCount));

	Value promiseValue;
	result = m_module->m_operatorMgr.newOperator(promiseType, &promiseValue);
	ASSERT(result);

	size_t j = 0;

	if (isMember())
	{
		StructField* argField = argFieldArray[0];
		Value argFieldValue;

		result = m_module->m_operatorMgr.getField(promiseValue, argField, &argFieldValue);
		ASSERT(result);

		result = m_module->m_operatorMgr.storeDataRef(argFieldValue, m_module->m_functionMgr.getThisValue());
		ASSERT(result);

		j = 1;
	}

	for (size_t i = 0; i < argCount; i++, j++)
	{
		Variable* argVar = argVariableArray[i];
		StructField* argField = argFieldArray[j];
		Value argFieldValue;

		result = m_module->m_operatorMgr.getField(promiseValue, argField, &argFieldValue);
		ASSERT(result);

		result = m_module->m_operatorMgr.storeDataRef(argFieldValue, argVar);
		ASSERT(result);
	}

	Type* argType = promiseType->getClassPtrType(ClassPtrTypeKind_Normal, PtrTypeFlag_Safe);
	uint_t flags = m_type->getFlags() & FunctionTypeFlag_AsyncErrorCode;

	FunctionType* functionType = m_module->m_typeMgr.getFunctionType(&argType, 1, flags);

	AsyncSequencerFunction* sequencerFunc = (AsyncSequencerFunction*) m_module->m_functionMgr.createFunction(
		FunctionKind_AsyncSequencer,
		sl::String(),
		sequencerName,
		functionType
		);

	sequencerFunc->m_asyncLauncher = this;
	sequencerFunc->m_parentUnit = m_parentUnit;
	sequencerFunc->m_parentNamespace = m_parentNamespace;
	sequencerFunc->m_thisArgType = m_thisArgType;
	sequencerFunc->m_thisType = m_thisType;
	sequencerFunc->m_thisArgDelta = m_thisArgDelta;

	sequencerFunc->setBody(&m_body);

	m_module->m_operatorMgr.callOperator(sequencerFunc, promiseValue);

	result = m_module->m_controlFlowMgr.ret(promiseValue);
	ASSERT(result);

	m_module->m_namespaceMgr.setSourcePos(endPos);
	return m_module->m_functionMgr.epilogue();
}

//..............................................................................

} // namespace ct
} // namespace jnc
