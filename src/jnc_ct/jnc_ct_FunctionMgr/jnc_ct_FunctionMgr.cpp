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
#include "jnc_ct_FunctionMgr.h"
#include "jnc_ct_SchedLauncherFunction.h"
#include "jnc_ct_AsyncLauncherFunction.h"
#include "jnc_ct_AsyncSchedLauncherFunction.h"
#include "jnc_ct_AsyncSequencerFunction.h"
#include "jnc_ct_ThunkFunction.h"
#include "jnc_ct_ThunkProperty.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_Jit.h"
#include "jnc_ct_MulticastClassType.h"
#include "jnc_ct_ExtensionNamespace.h"
#include "jnc_ct_Parser.llk.h"

// #define _JNC_LLVM_VERIFY 1
// #define _JNC_LLVM_NO_JIT 1

namespace jnc {
namespace ct {

//..............................................................................

FunctionMgr::FunctionMgr() {
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);

	memset(m_stdFunctionArray, 0, sizeof(m_stdFunctionArray));
	memset(m_stdPropertyArray, 0, sizeof(m_stdPropertyArray));

	m_currentFunction = NULL;
}

void
FunctionMgr::clear() {
	m_functionList.clear();
	m_functionOverloadList.clear();
	m_propertyList.clear();
	m_propertyTemplateList.clear();
	m_thunkFunctionMap.clear();
	m_thunkPropertyMap.clear();
	m_schedLauncherFunctionMap.clear();
	m_asyncSequencerFunctionArray.clear();

	for (size_t i = 0; i < countof(m_globalCtorDtorArrayTable); i++)
		m_globalCtorDtorArrayTable[i].clear();

	memset(m_stdFunctionArray, 0, sizeof(m_stdFunctionArray));
	memset(m_stdPropertyArray, 0, sizeof(m_stdPropertyArray));

	m_thisValue.clear();
	m_promiseValue.clear();
	m_currentFunction = NULL;
}

Value
FunctionMgr::overrideThisValue(const Value& value) {
	Value prevThisValue = m_thisValue;
	m_thisValue = value;
	return prevThisValue;
}

bool
FunctionMgr::addGlobalCtorDtor(
	GlobalCtorDtorKind kind,
	Function* function
) {
	ASSERT((size_t)kind < countof(m_globalCtorDtorArrayTable));

	if (!function->getType()->getArgArray().isEmpty()) {
		err::setFormatStringError("global constructor cannot have arguments");
		return false;
	}

	m_globalCtorDtorArrayTable[kind].append(function);

	if (!function->m_prologueBlock) // otherwise, already compiled (e.g., global-initializer)
		m_module->markForCompile(function);

	return true;
}

void
FunctionMgr::addFunction(
	Function* function,
	const sl::StringRef& name,
	const sl::StringRef& qualifiedName,
	FunctionType* type
) {
	function->m_module = m_module;
	function->m_name = name;
	function->m_qualifiedName = qualifiedName;
	function->m_type = type;
	m_functionList.insertTail(function);
}

FunctionOverload*
FunctionMgr::createFunctionOverload(Function* function) {
	FunctionOverload* overload = new FunctionOverload;
	*(ModuleItemDecl*)overload = *(ModuleItemDecl*)function;
	*(FunctionName*)overload = *(FunctionName*)function;
	overload->m_module = m_module;
	overload->m_overloadArray.append(function);
	overload->m_typeOverload.addOverload(function->m_type);
	m_functionOverloadList.insertTail(overload);
	return overload;
}

void
FunctionMgr::addProperty(
	Property* prop,
	const sl::StringRef& name,
	const sl::StringRef& qualifiedName
) {
	prop->m_module = m_module;
	prop->m_name = name;
	prop->m_qualifiedName = qualifiedName;
	m_propertyList.insertTail(prop);
}

PropertyTemplate*
FunctionMgr::createPropertyTemplate() {
	PropertyTemplate* propertyTemplate = new PropertyTemplate;
	propertyTemplate->m_module = m_module;
	m_propertyTemplateList.insertTail(propertyTemplate);
	return propertyTemplate;
}

bool
FunctionMgr::fireOnChanged() {
	Function* function = m_currentFunction;

	ASSERT(
		function->m_functionKind == FunctionKind_Setter &&
		function->m_property &&
		function->m_property->getType()->getFlags() & PropertyTypeFlag_Bindable
	);

	Value propertyValue = function->m_property;

	if (function->m_thisType) {
		ASSERT(m_thisValue);

		Closure* closure = propertyValue.createClosure();
		closure->insertThisArgValue(m_thisValue);
	}

	Value onChanged;

	return
		m_module->m_operatorMgr.getPropertyOnChanged(propertyValue, &onChanged) &&
		m_module->m_operatorMgr.memberOperator(&onChanged, "call") &&
		m_module->m_operatorMgr.callOperator(onChanged);
}

Function*
FunctionMgr::setCurrentFunction(Function* function) {
	Function* prevFunction = m_currentFunction;
	m_currentFunction = function;
	return prevFunction;
}

void
FunctionMgr::prologue(
	Function* function,
	const lex::LineCol& pos
) {
	m_currentFunction = function;

	// create entry blocks

	if (m_module->hasCodeGen())
		m_module->m_llvmIrBuilder.setCurrentDebugLoc(llvm::DebugLoc());

	function->m_allocaBlock = m_module->m_controlFlowMgr.createBlock("function_entry");
	function->m_allocaBlock->markEntry();
	function->m_prologueBlock = m_module->m_controlFlowMgr.createBlock("function_prologue");
	function->m_prologueBlock->markEntry();

	m_module->m_controlFlowMgr.setCurrentBlock(function->m_allocaBlock);
	m_module->m_controlFlowMgr.jump(function->m_prologueBlock, function->m_prologueBlock);

	if (m_module->hasCodeGen())
		m_module->m_llvmIrBuilder.setAllocaBlock(function->m_allocaBlock);

	// create scope

	m_module->m_namespaceMgr.openNamespace(function->m_parentNamespace);

	function->m_scope = m_module->m_namespaceMgr.openScope(pos);

	if (function->m_extensionNamespace) {
		function->m_scope->m_usingSet.addGlobalNamespace(function->m_extensionNamespace);
		function->m_scope->m_usingSet.addExtensionNamespace(function->m_extensionNamespace);
	}

	if (function->m_type->getFlags() & FunctionTypeFlag_Unsafe)
		m_module->m_operatorMgr.enterUnsafeRgn();

	if (m_module->hasCodeGen()) {
		function->getType()->getCallConv()->createArgVariables(function);
	} else {
		sl::Array<FunctionArg*> argArray = function->getType()->getArgArray();
		size_t argCount = argArray.getCount();
		for (size_t i = 0; i < argCount; i++) {
			FunctionArg* arg = argArray[i];
			if (!arg->isNamed())
				continue;

			Variable* argVariable = m_module->m_variableMgr.createArgVariable(arg, i);
			function->getScope()->addItem(argVariable);
		}
	}

	BasicBlock* bodyBlock = m_module->m_controlFlowMgr.createBlock("function_body");
	m_module->m_controlFlowMgr.jump(bodyBlock, bodyBlock);

	if (m_module->getCompileFlags() & ModuleCompileFlag_GcSafePointInPrologue)
		m_module->m_operatorMgr.gcSafePoint();

	// 'this' arg

	if (function->isMember())
		createThisValue();
}

void
FunctionMgr::createThisValue() {
	Function* function = m_currentFunction;
	ASSERT(function && function->isMember());

	if (!m_module->hasCodeGen()) {
		m_thisValue = function->m_thisType;
		return;
	}

	Value thisArgValue = function->getType()->getCallConv()->getThisArgValue(function);
	if (function->m_thisArgType->cmp(function->m_thisType) == 0) {
		if (function->m_thisType->getTypeKind() != TypeKind_DataPtr)
			m_thisValue = thisArgValue;
		else
			m_module->m_operatorMgr.makeLeanDataPtr(thisArgValue, &m_thisValue);
	} else {
		ASSERT(function->m_storageKind == StorageKind_Override);

		if (function->m_thisArgDelta == 0) {
			m_module->m_llvmIrBuilder.createBitCast(thisArgValue, function->m_thisType, &m_thisValue);
		} else {
			Value ptrValue;
			m_module->m_llvmIrBuilder.createBitCast(thisArgValue, m_module->m_typeMgr.getStdType(StdType_ByteThinPtr), &ptrValue);
			m_module->m_llvmIrBuilder.createGep(ptrValue, (int32_t)function->m_thisArgDelta, NULL, &ptrValue);
			m_module->m_llvmIrBuilder.createBitCast(ptrValue, function->m_thisType, &m_thisValue);
		}
	}
}

bool
FunctionMgr::epilogue() {
	bool result;

	Function* function = m_currentFunction;
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope();
	if (!scope || !(scope->m_flags & ScopeFlag_Function)) {
		err::setError("invalid scope structure due to previous errors");
		return false;
	}

	if (function->m_functionKind == FunctionKind_Destructor &&
		function->m_storageKind == StorageKind_Member) {
		ASSERT(m_thisValue);

		if (function->getProperty()) {
			Property* prop = function->getProperty();
			result = prop->callPropertyDestructors(m_thisValue);
		} else {
			ASSERT(function->getParentType()->getTypeKind() == TypeKind_Class && m_thisValue);
			ClassType* classType = (ClassType*)function->getParentType();

			result =
				classType->callPropertyDestructors(m_thisValue) &&
				classType->callBaseTypeDestructors(m_thisValue);
		}

		if (!result)
			return false;
	}

	result = m_module->m_controlFlowMgr.checkReturn();
	if (!result)
		return false;

	if (function->m_type->getFlags() & FunctionTypeFlag_Unsafe)
		m_module->m_operatorMgr.leaveUnsafeRgn();

	finalizeFunction(function, true);

#if (_JNC_DEBUG && _JNC_LLVM_VERIFY)
#	if (LLVM_VERSION < 0x030500)
	bool isBroken = llvm::verifyFunction(*function->getLlvmFunction(), llvm::ReturnStatusAction);
#	else
	bool isBroken = llvm::verifyFunction(*function->getLlvmFunction());
#	endif

	if (isBroken) {
		err::setFormatStringError(
			"LLVM verification fail for '%s'",
			function->getQualifiedName().sz()
		);

		return false;
	}
#endif

	return true;
}

void
FunctionMgr::finalizeFunction(
	Function* function,
	bool wasNamespaceOpened
) {
	ASSERT(function == m_currentFunction);

	m_module->m_namespaceMgr.closeScope();

	if (wasNamespaceOpened)
		m_module->m_namespaceMgr.closeNamespace();

	m_module->m_operatorMgr.resetUnsafeRgn();
	m_module->m_variableMgr.finalizeFunction();
	m_module->m_gcShadowStackMgr.finalizeFunction();
	m_module->m_controlFlowMgr.finalizeFunction();

	size_t count = function->m_tlsVariableArray.getCount();
	for (size_t i = 0; i < count; i++)
		function->m_tlsVariableArray[i].m_variable->m_llvmValue = NULL;

	m_thisValue.clear();
	m_promiseValue.clear();
	m_currentFunction = NULL;
}

void
FunctionMgr::internalPrologue(
	Function* function,
	Value* argValueArray,
	size_t argCount,
	const lex::LineCol* pos
) {
	m_currentFunction = function;

	if (m_module->hasCodeGen())
		m_module->m_llvmIrBuilder.setCurrentDebugLoc(llvm::DebugLoc());

	function->m_allocaBlock = m_module->m_controlFlowMgr.createBlock("function_entry");
	function->m_allocaBlock->markEntry();
	function->m_prologueBlock = m_module->m_controlFlowMgr.createBlock("function_prologue");
	function->m_prologueBlock->markEntry();

	m_module->m_controlFlowMgr.setCurrentBlock(function->m_allocaBlock);
	m_module->m_controlFlowMgr.jump(function->m_prologueBlock, function->m_prologueBlock);

	if (m_module->hasCodeGen())
		m_module->m_llvmIrBuilder.setAllocaBlock(function->m_allocaBlock);

	function->m_scope = pos ?
		m_module->m_namespaceMgr.openScope(*pos) :
		m_module->m_namespaceMgr.openInternalScope();

	if (function->isMember() && function->getFunctionKind() != FunctionKind_AsyncSequencer)
		createThisValue();

	if (argCount)
		if (m_module->hasCodeGen())
			function->getType()->getCallConv()->getArgValueArray(function, argValueArray, argCount);
		else {
			sl::Array<FunctionArg*> argArray = function->getType()->getArgArray();
			ASSERT(argCount <= argArray.getCount());

			for (size_t i = 0; i < argCount; i++)
				argValueArray[i].setType(argArray[i]->getType());
		}

	BasicBlock* bodyBlock = m_module->m_controlFlowMgr.createBlock("function_body");
	m_module->m_controlFlowMgr.jump(bodyBlock, bodyBlock);

	if (m_module->getCompileFlags() & ModuleCompileFlag_GcSafePointInInternalPrologue)
		m_module->m_operatorMgr.gcSafePoint();
}

void
FunctionMgr::internalEpilogue() {
	Function* function = m_currentFunction;

	BasicBlock* currentBlock = m_module->m_controlFlowMgr.getCurrentBlock();
	if (m_module->hasCodeGen() && !currentBlock->hasTerminator()) {
		Type* returnType = function->getType()->getReturnType();

		Value returnValue;
		if (returnType->getTypeKind() != TypeKind_Void)
			returnValue = returnType->getZeroValue();

		m_module->m_controlFlowMgr.ret(returnValue);
	}

	finalizeFunction(function, false);
}

Function*
FunctionMgr::getDirectThunkFunction(
	Function* targetFunction,
	FunctionType* thunkFunctionType,
	bool hasUnusedClosure
) {
	if (!hasUnusedClosure && targetFunction->m_type->cmp(thunkFunctionType) == 0)
		return targetFunction;

	char signatureChar = 'D';

	if (hasUnusedClosure) {
		signatureChar = 'U';
		thunkFunctionType = thunkFunctionType->getStdObjectMemberMethodType();
	}

	sl::String signature;
	signature.format(
		"%c%x.%s",
		signatureChar,
		targetFunction,
		thunkFunctionType->getSignature().sz()
	);

	sl::StringHashTableIterator<Function*> it = m_thunkFunctionMap.visit(signature);
	if (it->m_value)
		return it->m_value;

	ThunkFunction* thunkFunction = createFunction<ThunkFunction>(
		sl::String(),
		"jnc.directThunkFunction",
		thunkFunctionType
	);

	thunkFunction->m_storageKind = StorageKind_Static;
	thunkFunction->m_targetFunction = targetFunction;
	it->m_value = thunkFunction;

	return thunkFunction;
}

Property*
FunctionMgr::getDirectThunkProperty(
	Property* targetProperty,
	PropertyType* thunkPropertyType,
	bool hasUnusedClosure
) {
	if (!hasUnusedClosure && targetProperty->m_type->cmp(thunkPropertyType) == 0)
		return targetProperty;

	sl::String signature;
	signature.format(
		"%c%x.%s",
		hasUnusedClosure ? 'U' : 'D',
		targetProperty,
		thunkPropertyType->getSignature().sz()
	);

	sl::StringHashTableIterator<Property*> it = m_thunkPropertyMap.visit(signature);
	if (it->m_value)
		return it->m_value;

	ThunkProperty* thunkProperty = createProperty<ThunkProperty>(
		sl::String(),
		"jnc.g_directThunkProperty"
	);

	thunkProperty->m_storageKind = StorageKind_Static;

	bool result = thunkProperty->create(targetProperty, thunkPropertyType, hasUnusedClosure);
	ASSERT(result);

	it->m_value = thunkProperty;
	return thunkProperty;
}

Property*
FunctionMgr::getDirectDataThunkProperty(
	Variable* targetVariable,
	PropertyType* thunkPropertyType,
	bool hasUnusedClosure
) {
	bool result;

	sl::String signature;
	signature.format(
		"%c%x.%s",
		hasUnusedClosure ? 'U' : 'D',
		targetVariable,
		thunkPropertyType->getSignature().sz()
	);

	sl::StringHashTableIterator<Property*> it = m_thunkPropertyMap.visit(signature);
	if (it->m_value)
		return it->m_value;

	DataThunkProperty* thunkProperty = createProperty<DataThunkProperty>(
		sl::String(),
		"jnc.g_directDataThunkProperty"
	);

	thunkProperty->m_storageKind = StorageKind_Static;
	thunkProperty->m_targetVariable = targetVariable;

	if (hasUnusedClosure)
		thunkPropertyType = thunkPropertyType->getStdObjectMemberPropertyType();

	result = thunkProperty->create(thunkPropertyType);
	ASSERT(result);

	it->m_value = thunkProperty;
	return thunkProperty;
}

Function*
FunctionMgr::getSchedLauncherFunction(FunctionPtrType* targetFunctionPtrType) {
	sl::String signature = targetFunctionPtrType->getSignature();
	sl::StringHashTableIterator<Function*> it = m_schedLauncherFunctionMap.visit(signature);
	if (it->m_value)
		return it->m_value;

	Type* schedulerPtrType = m_module->m_typeMgr.getStdType(StdType_SchedulerPtr);
	FunctionType* targetFunctionType = targetFunctionPtrType->getTargetType();
	sl::Array<FunctionArg*> argArray  = targetFunctionType->getArgArray();
	argArray.insert(0, targetFunctionPtrType->getSimpleFunctionArg());
	argArray.insert(1, schedulerPtrType->getSimpleFunctionArg());

	Function* launcherFunction;

	if (targetFunctionType->getFlags() & FunctionTypeFlag_Async) {
		Type* returnType = m_module->m_typeMgr.getStdType(StdType_PromisePtr);
		FunctionType* launcherFunctionType = m_module->m_typeMgr.getFunctionType(returnType, argArray);

		launcherFunction = createFunction<AsyncSchedLauncherFunction>(
			sl::String(),
			"jnc.asyncSchedLauncher",
			launcherFunctionType
		);
	} else {
		FunctionType* launcherFunctionType = m_module->m_typeMgr.getFunctionType(argArray);

		launcherFunction = createFunction<SchedLauncherFunction>(
			sl::String(),
			"jnc.schedLauncher",
			launcherFunctionType
		);
	}

	launcherFunction->m_storageKind = StorageKind_Static;

	it->m_value = launcherFunction;
	return launcherFunction;
}

bool
FunctionMgr::finalizeNamespaceProperties(const sl::ConstIterator<Property>& prevIt) {
	sl::Iterator<Property> it = prevIt ? (Property*)prevIt.getNext().p() : m_propertyList.getHead();
	for (; it; it++) {
		if (!it->getStorageKind()) {
			ASSERT(m_module->getCompileErrorCount());
			continue; // ignore invalid properties (errors already emitted)
		}

		bool result = it->finalize();
		if (!result)
			return false;
	}

	return true;
}

void
FunctionMgr::injectTlsPrologues() {
	sl::Iterator<Function> it = m_functionList.getHead();
	for (; it; it++)
		if (it->isTlsRequired())
			injectTlsPrologue(*it);
}

void
FunctionMgr::injectTlsPrologue(Function* function) {
	BasicBlock* block = function->getPrologueBlock();
	ASSERT(block);

	m_module->m_controlFlowMgr.setCurrentBlock(block);
	m_module->m_llvmIrBuilder.setInsertPoint(&*block->getLlvmBlock()->begin());

	Function* getTls = getStdFunction(StdFunc_GetTls);

	Value tlsValue;
	m_module->m_llvmIrBuilder.createCall(getTls, getTls->getType(), &tlsValue);

	// tls variables used in this function

	sl::Array<TlsVariable> tlsVariableArray = function->getTlsVariableArray();
	size_t count = tlsVariableArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Field* field = tlsVariableArray[i].m_variable->getTlsField();
		ASSERT(field);

		Value ptrValue;
		m_module->m_llvmIrBuilder.createGep2(tlsValue, field->getLlvmIndex(), NULL, &ptrValue);
		tlsVariableArray[i].m_llvmAlloca->replaceAllUsesWith(ptrValue.getLlvmValue());
	}

	// unfortunately, erasing could not be safely done inside the above loop (cause of InsertPoint)
	// so just have a dedicated loop for erasing alloca's

	count = tlsVariableArray.getCount();
	for (size_t i = 0; i < count; i++)
		tlsVariableArray[i].m_llvmAlloca->eraseFromParent();
}

void
FunctionMgr::replaceAsyncAllocas() {
	size_t count = m_asyncSequencerFunctionArray.getCount();
	for (size_t i = 0; i < count; i++)
		m_asyncSequencerFunctionArray[i]->replaceAllocas();
}

#if (LLVM_VERSION_MAJOR < 14)
void
llvmFatalErrorHandler(
	void* context,
	const std::string& errorString,
	bool shouldGenerateCrashDump
) {
	throw err::Error(errorString.c_str());
}
#else
void
llvmFatalErrorHandler(
	void* context,
	const char* errorString,
	bool shouldGenerateCrashDump
) {
	throw err::Error(errorString);
}
#endif

bool
FunctionMgr::jitFunctions() {
#if (_JNC_LLVM_NO_JIT)
	err::setFormatStringError("LLVM jitting is disabled");
	return false;
#endif

	llvm::ScopedFatalErrorHandler scopeErrorHandler(llvmFatalErrorHandler);

	try {
		sl::Iterator<Function> it = m_functionList.getHead();
		for (; it; it++)
			if (!it->isEmpty()) {
				void* p = m_module->m_jit->jit(*it);
				if (!p)
					return false;

				it->m_machineCode = p;
			}

		m_module->m_jit->finalizeObject();
	} catch (err::Error error) {
		err::setFormatStringError("LLVM jitting failed: %s", error->getDescription().sz());
		return false;
	}

	return true;
}

Function*
FunctionMgr::getStdFunction(StdFunc func) {
	ASSERT((size_t)func < StdFunc__Count);

	if (m_stdFunctionArray[func])
		return m_stdFunctionArray[func];

	// 8 is enough for all the std functions

	Type* argTypeArray[8] = { 0 };
	llvm::Type* llvmArgTypeArray[8] = { 0 };

	Type* returnType;
	FunctionType* functionType;
	Function* function;

	switch (func) {
	case StdFunc_PrimeStaticClass:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_BoxPtr);
		argTypeArray[1] = m_module->m_typeMgr.getStdType(StdType_ByteThinPtr);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 2);
		function = createInternalFunction("jnc.primeStaticClass", functionType);
		break;

	case StdFunc_TryAllocateClass:
		returnType = m_module->m_typeMgr.getStdType(StdType_AbstractClassPtr);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_ByteThinPtr);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 1);
		function = createInternalFunction("jnc.tryAllocateClass", functionType);
		break;

	case StdFunc_AllocateClass:
		returnType = m_module->m_typeMgr.getStdType(StdType_AbstractClassPtr);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_ByteThinPtr);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 1);
		function = createInternalFunction("jnc.allocateClass", functionType);
		break;

	case StdFunc_TryAllocateData:
		returnType = m_module->m_typeMgr.getStdType(StdType_AbstractDataPtr);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_ByteThinPtr);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 1);
		function = createInternalFunction("jnc.tryAllocateData", functionType);
		break;

	case StdFunc_AllocateData:
		returnType = m_module->m_typeMgr.getStdType(StdType_AbstractDataPtr);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_ByteThinPtr);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 1);
		function = createInternalFunction("jnc.allocateData", functionType);
		break;

	case StdFunc_TryAllocateArray:
		returnType = m_module->m_typeMgr.getStdType(StdType_AbstractDataPtr);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_ByteThinPtr);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 2);
		function = createInternalFunction("jnc.tryAllocateArray", functionType);
		break;

	case StdFunc_AllocateArray:
		returnType = m_module->m_typeMgr.getStdType(StdType_AbstractDataPtr);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_ByteThinPtr);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 2);
		function = createInternalFunction("jnc.allocateArray", functionType);
		break;

	case StdFunc_CreateDataPtrValidator:
		returnType = m_module->m_typeMgr.getStdType(StdType_DataPtrValidatorPtr);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_BoxPtr);
		argTypeArray[1] = m_module->m_typeMgr.getStdType(StdType_ByteThinPtr);
		argTypeArray[2] = m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 3);
		function = createInternalFunction("jnc.createDataPtrValidator", functionType);
		break;

	case StdFunc_TryCheckDataPtrRangeIndirect:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Bool);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_ByteThinPtr);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT);
		argTypeArray[2] = m_module->m_typeMgr.getStdType(StdType_DataPtrValidatorPtr);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 3, FunctionTypeFlag_ErrorCode);
		function = createInternalFunction("jnc.tryCheckDataPtrRangeIndirect", functionType);
		break;

	case StdFunc_CheckDataPtrRangeIndirect:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_ByteThinPtr);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT);
		argTypeArray[2] = m_module->m_typeMgr.getStdType(StdType_DataPtrValidatorPtr);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 3);
		function = createInternalFunction("jnc.checkDataPtrRangeIndirect", functionType);
		break;

	case StdFunc_LlvmMemcpy:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_ByteThinPtr);
		argTypeArray[1] = m_module->m_typeMgr.getStdType(StdType_ByteThinPtr);
		argTypeArray[2] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32);
		argTypeArray[3] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32);
		argTypeArray[4] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Bool);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 5);
		function = createInternalFunction("jnc.llvmMemcpy", functionType);

		llvmArgTypeArray[0] = argTypeArray[0]->getLlvmType();
		llvmArgTypeArray[1] = argTypeArray[1]->getLlvmType();
		llvmArgTypeArray[2] = argTypeArray[2]->getLlvmType();
		function->m_llvmFunction = llvm::Intrinsic::getDeclaration(
			m_module->getLlvmModule(),
			llvm::Intrinsic::memcpy,
			llvm::ArrayRef<llvm::Type*> (llvmArgTypeArray, 3)
		);
		break;

	case StdFunc_LlvmMemmove:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_ByteThinPtr);
		argTypeArray[1] = m_module->m_typeMgr.getStdType(StdType_ByteThinPtr);
		argTypeArray[2] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32);
		argTypeArray[3] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32);
		argTypeArray[4] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Bool);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 5);
		function = createInternalFunction("jnc.llvmMemmove", functionType);

		llvmArgTypeArray[0] = argTypeArray[0]->getLlvmType();
		llvmArgTypeArray[1] = argTypeArray[1]->getLlvmType();
		llvmArgTypeArray[2] = argTypeArray[2]->getLlvmType();
		function->m_llvmFunction = llvm::Intrinsic::getDeclaration(
			m_module->getLlvmModule(),
			llvm::Intrinsic::memmove,
			llvm::ArrayRef<llvm::Type*> (llvmArgTypeArray, 3)
		);
		break;

	case StdFunc_LlvmMemset:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_ByteThinPtr);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int8);
		argTypeArray[2] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32);
		argTypeArray[3] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32);
		argTypeArray[4] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Bool);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 5);
		function = createInternalFunction("jnc.llvmMemset", functionType);

		llvmArgTypeArray[0] = argTypeArray[0]->getLlvmType();
		llvmArgTypeArray[1] = argTypeArray[2]->getLlvmType();
		function->m_llvmFunction = llvm::Intrinsic::getDeclaration(
			m_module->getLlvmModule(),
			llvm::Intrinsic::memset,
			llvm::ArrayRef<llvm::Type*> (llvmArgTypeArray, 2)
		);

		break;

	case StdFunc_GetTls:
		returnType = m_module->m_variableMgr.getTlsStructType()->getDataPtrType(DataPtrTypeKind_Thin);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, NULL, 0);
		function = createInternalFunction("jnc.getTls", functionType);
		break;

	case StdFunc_SetJmp:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_SjljFrame)->getDataPtrType_c();
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 1);
		function = createInternalFunction("jnc.setJmp", functionType);
		function->getLlvmFunction()->addFnAttr(llvm::Attribute::ReturnsTwice);
		break;

	case StdFunc_SaveSignalInfo:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_SjljFrame)->getDataPtrType_c();
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 1);
		function = createInternalFunction("jnc.saveSignalInfo", functionType);
		break;

	case StdFunc_DynamicThrow:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, NULL, 0);
		function = createInternalFunction("jnc.dynamicThrow", functionType);
		function->getLlvmFunction()->addFnAttr(llvm::Attribute::NoReturn);
		break;

	case StdFunc_AsyncRet:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_PromisePtr);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 2);
		function = createInternalFunction("jnc.asyncRet", functionType);
		break;

	case StdFunc_AsyncThrow:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_PromisePtr);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 1);
		function = createInternalFunction("jnc.asyncThrow", functionType);
		break;

	case StdFunc_VariantUnaryOperator:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		argTypeArray[0] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 2);
		function = createInternalFunction("jnc.variantUnaryOperator", functionType);
		break;

	case StdFunc_VariantBinaryOperator:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		argTypeArray[0] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		argTypeArray[2] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 3);
		function = createInternalFunction("jnc.variantBinaryOperator", functionType);
		break;

	case StdFunc_VariantRelationalOperator:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Bool);
		argTypeArray[0] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		argTypeArray[2] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 3);
		function = createInternalFunction("jnc.variantRelationalOperator", functionType);
		break;

	case StdFunc_VariantMemberOperator:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		argTypeArray[0] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		argTypeArray[1] = m_module->m_typeMgr.getStdType(jnc_StdType_CharConstThinPtr);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 2);
		function = createInternalFunction("jnc.variantMemberOperator", functionType);
		break;

	case StdFunc_VariantIndexOperator:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		argTypeArray[0] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 2);
		function = createInternalFunction("jnc.variantIndexOperator", functionType);
		break;

	case StdFunc_VariantMemberProperty_get:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		argTypeArray[0] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant)->getDataPtrType(DataPtrTypeKind_Normal, PtrTypeFlag_Const);
		argTypeArray[1] = m_module->m_typeMgr.getStdType(jnc_StdType_CharConstThinPtr);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 2);
		function = createInternalFunction("jnc.variantMemberProperty.get", functionType);
		break;

	case StdFunc_VariantMemberProperty_set:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		argTypeArray[0] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant)->getDataPtrType();
		argTypeArray[1] = m_module->m_typeMgr.getStdType(jnc_StdType_CharConstThinPtr);
		argTypeArray[2] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 3);
		function = createInternalFunction("jnc.variantMemberProperty.set", functionType);
		break;

	case StdFunc_VariantIndexProperty_get:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		argTypeArray[0] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant)->getDataPtrType(DataPtrTypeKind_Normal, PtrTypeFlag_Const);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 2);
		function = createInternalFunction("jnc.variantIndexProperty.get", functionType);
		break;

	case StdFunc_VariantIndexProperty_set:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		argTypeArray[0] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant)->getDataPtrType();
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT);
		argTypeArray[2] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 3);
		function = createInternalFunction("jnc.variantIndexProperty.set", functionType);
		break;

	case StdFunc_StringCreate:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_String);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_CharConstPtr);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 2);
		function = createInternalFunction("jnc.stringCreate", functionType);
		break;

	case StdFunc_StringSz:
		returnType = m_module->m_typeMgr.getStdType(StdType_CharConstPtr);
		argTypeArray[0] = m_module->m_typeMgr.getPrimitiveType(TypeKind_String);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 1);
		function = createInternalFunction("jnc.stringSz", functionType);
		break;

	case StdFunc_StringRefSz:
		returnType = m_module->m_typeMgr.getStdType(StdType_CharConstPtr);
		argTypeArray[0] = m_module->m_typeMgr.getPrimitiveType(TypeKind_String)->getDataPtrType_c(
			TypeKind_DataPtr,
			PtrTypeFlag_Const
		);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 1);
		function = createInternalFunction("jnc.stringRefSz", functionType);
		break;

	case StdFunc_GcSafePoint:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, NULL, 0);
		function = createInternalFunction("jnc.gcSafePoint", functionType);
		break;

	case StdFunc_SetGcShadowStackFrameMap:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_GcShadowStackFrame)->getDataPtrType_c();
		argTypeArray[1] = m_module->m_typeMgr.getStdType(StdType_ByteThinPtr);
		argTypeArray[2] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 3);
		function = createInternalFunction("jnc.setGcShadowStackFrameMap", functionType);
		break;

	case StdFunc_DynamicSizeOf:
	case StdFunc_DynamicCountOf:
	case StdFunc_DynamicTypeSizeOf:
	case StdFunc_DynamicFieldSizeOf:
	case StdFunc_DynamicFieldCountOf:
	case StdFunc_DynamicCastDataPtr:
	case StdFunc_DynamicCastClassPtr:
	case StdFunc_DynamicCastVariant:
	case StdFunc_StrengthenClassPtr:
	case StdFunc_AssertionFailure:
	case StdFunc_AddStaticDestructor:
	case StdFunc_AddStaticClassDestructor:
	case StdFunc_AppendFmtLiteral_a:
	case StdFunc_AppendFmtLiteral_p:
	case StdFunc_AppendFmtLiteral_i32:
	case StdFunc_AppendFmtLiteral_ui32:
	case StdFunc_AppendFmtLiteral_i64:
	case StdFunc_AppendFmtLiteral_ui64:
	case StdFunc_AppendFmtLiteral_f:
	case StdFunc_AppendFmtLiteral_v:
	case StdFunc_AppendFmtLiteral_s:
	case StdFunc_AppendFmtLiteral_re:
	case StdFunc_TryCheckDataPtrRangeDirect:
	case StdFunc_CheckDataPtrRangeDirect:
	case StdFunc_TryLazyGetDynamicLibFunction:
	case StdFunc_LazyGetDynamicLibFunction:
	case StdFunc_GetDynamicField:
		function = parseStdFunction(func);
		break;

	case StdFunc_SimpleMulticastCall:
		function = ((MulticastClassType*)m_module->m_typeMgr.getStdType(StdType_SimpleMulticast))->getMethod(MulticastMethodKind_Call);
		break;

	default:
		ASSERT(false);
		function = NULL;
	}

	m_stdFunctionArray[func] = function;
	return function;
}

Function*
FunctionMgr::parseStdFunction(StdFunc func) {
	const StdItemSource* source = getStdFunctionSource(func);
	ASSERT(source->m_source);

	return parseStdFunction(
		source->m_stdNamespace,
		sl::StringRef(source->m_source, source->m_length)
	);
}

Function*
FunctionMgr::parseStdFunction(
	StdNamespace stdNamespace,
	const sl::StringRef& source
) {
	sl::StringRef fileName = "jnc_StdFunctions.jnc";
	Lexer lexer;
	lexer.create(fileName, source);

	if (stdNamespace)
		m_module->m_namespaceMgr.openStdNamespace(stdNamespace);

	Parser parser(m_module, NULL, Parser::Mode_Compile);
	parser.create(fileName, SymbolKind_normal_item_declaration);
#if (_LLK_RANDOM_ERRORS)
	parser.disableRandomErrors();
#endif

	bool isEof;

	do {
		Token* token = lexer.takeToken();
		isEof = token->m_token == TokenKind_Eof; // EOF token must be parsed
		bool result = parser.consumeToken(token);
		if (!result) {
			TRACE("parse std function error: %s\n", err::getLastErrorDescription().sz());
			ASSERT(false);
		}
	} while (!isEof);

	if (stdNamespace)
		m_module->m_namespaceMgr.closeNamespace();

	ModuleItem* item = parser.getLastDeclaredItem();
	ASSERT(item && item->getItemKind() == ModuleItemKind_Function);
	return (Function*)item;
}

Property*
FunctionMgr::getStdProperty(StdProp stdProp) {
	ASSERT((size_t)stdProp < StdProp__Count);

	if (m_stdPropertyArray[stdProp])
		return m_stdPropertyArray[stdProp];

	Property* prop;
	switch (stdProp) {
	case StdProp_VariantMember:
		prop = createInternalProperty("jnc.g_variantMember");
		prop->m_storageKind = StorageKind_Static;
		prop->m_getter = getStdFunction(StdFunc_VariantMemberProperty_get);
		prop->m_setter = getStdFunction(StdFunc_VariantMemberProperty_set);
		prop->m_type = m_module->m_typeMgr.getPropertyType(prop->m_getter->getType(), prop->m_setter.getFunction()->getType());
		break;

	case StdProp_VariantIndex:
		prop = createInternalProperty("jnc.g_variantIndex");
		prop->m_storageKind = StorageKind_Static;
		prop->m_getter = getStdFunction(StdFunc_VariantIndexProperty_get);
		prop->m_setter = getStdFunction(StdFunc_VariantIndexProperty_set);
		prop->m_type = m_module->m_typeMgr.getPropertyType(prop->m_getter->getType(), prop->m_setter.getFunction()->getType());
		break;

	default:
		ASSERT(false);
		prop = NULL;
	}

	m_stdPropertyArray[stdProp] = prop;
	return prop;
}

//..............................................................................

} // namespace ct
} // namespace jnc
