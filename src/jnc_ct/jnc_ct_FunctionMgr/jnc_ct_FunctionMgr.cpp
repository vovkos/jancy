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
#include "jnc_ct_MulticastClassType.h"
#include "jnc_ct_Parser.llk.h"

// #define _JNC_LLVM_VERIFY 1
// #define _JNC_LLVM_NO_JIT 1

namespace jnc {
namespace ct {

//..............................................................................

FunctionMgr::FunctionMgr()
{
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);

	memset(m_stdFunctionArray, 0, sizeof(m_stdFunctionArray));
	memset(m_lazyStdFunctionArray, 0, sizeof(m_lazyStdFunctionArray));
	memset(m_stdPropertyArray, 0, sizeof(m_stdPropertyArray));

	m_currentFunction = NULL;
}

void
FunctionMgr::clear()
{
	m_functionList.clear();
	m_propertyList.clear();
	m_propertyTemplateList.clear();
	m_lazyStdFunctionList.clear();
	m_lazyStdPropertyList.clear();
	m_thunkFunctionMap.clear();
	m_thunkPropertyMap.clear();
	m_schedLauncherFunctionMap.clear();
	m_asyncSequencerFunctionArray.clear();
	m_staticConstructArray.clear();
	memset(m_stdFunctionArray, 0, sizeof(m_stdFunctionArray));
	memset(m_lazyStdFunctionArray, 0, sizeof(m_lazyStdFunctionArray));
	memset(m_stdPropertyArray, 0, sizeof(m_stdPropertyArray));
	memset(m_lazyStdPropertyArray, 0, sizeof(m_lazyStdPropertyArray));

	m_thisValue.clear();
	m_promiseValue.clear();
	m_currentFunction = NULL;
}

void
FunctionMgr::callStaticConstructors()
{
	Function* addDestructor = getStdFunction(StdFunc_AddStaticDestructor);
	Type* dtorType = m_module->m_typeMgr.getStdType(StdType_BytePtr);

	size_t count = m_staticConstructArray.getCount();
	for (size_t i = 0; i < count; i++)
	{
		NamedTypeBlock* namedTypeBlock = m_staticConstructArray[i];

		Function* destructor = namedTypeBlock->getStaticDestructor();
		if (destructor)
		{
			Value dtorValue;
			m_module->m_llvmIrBuilder.createBitCast(destructor, dtorType, &dtorValue);
			m_module->m_llvmIrBuilder.createCall(addDestructor, addDestructor->getType(), dtorValue, NULL);
		}

		Function* constructor = namedTypeBlock->getStaticConstructor();
		ASSERT(constructor);

		m_module->m_llvmIrBuilder.createCall(constructor, constructor->getType(), NULL);
	}
}

Value
FunctionMgr::overrideThisValue(const Value& value)
{
	Value prevThisValue = m_thisValue;
	m_thisValue = value;
	return prevThisValue;
}

Function*
FunctionMgr::createFunction(
	FunctionKind functionKind,
	const sl::StringRef& name,
	const sl::StringRef& qualifiedName,
	FunctionType* type
	)
{
	Function* function;
	switch (functionKind)
	{
	case FunctionKind_Thunk:
		function = AXL_MEM_NEW(ThunkFunction);
		break;

	case FunctionKind_SchedLauncher:
		function = AXL_MEM_NEW(SchedLauncherFunction);
		break;

	case FunctionKind_AsyncSchedLauncher:
		function = AXL_MEM_NEW(AsyncSchedLauncherFunction);
		break;

	case FunctionKind_AsyncSequencer:
		function = AXL_MEM_NEW(AsyncSequencerFunction);
		m_asyncSequencerFunctionArray.append((AsyncSequencerFunction*)function);
		break;

	default:
		if (type->getFlags() & FunctionTypeFlag_Async)
		{
			function = AXL_MEM_NEW(AsyncLauncherFunction);
		}
		else
		{
			function = AXL_MEM_NEW(Function);
			function->m_functionKind = functionKind;
		}
	}

	function->m_module = m_module;
	function->m_name = name;
	function->m_qualifiedName = qualifiedName;
	function->m_type = type;
	function->m_typeOverload.addOverload(type);
	m_functionList.insertTail(function);
	return function;
}

Property*
FunctionMgr::createProperty(
	PropertyKind propertyKind,
	const sl::StringRef& name,
	const sl::StringRef& qualifiedName
	)
{
	Property* prop;

	switch (propertyKind)
	{
	case PropertyKind_Thunk:
		prop = AXL_MEM_NEW(ThunkProperty);
		break;

	case PropertyKind_DataThunk:
		prop = AXL_MEM_NEW(DataThunkProperty);
		break;

	default:
		prop = AXL_MEM_NEW(Property);
	}

	prop->m_module = m_module;
	prop->m_propertyKind = propertyKind;
	prop->m_name = name;
	prop->m_qualifiedName = qualifiedName;
	m_module->markForLayout(prop, true);
	m_propertyList.insertTail(prop);
	return prop;
}

PropertyTemplate*
FunctionMgr::createPropertyTemplate()
{
	PropertyTemplate* propertyTemplate = AXL_MEM_NEW(PropertyTemplate);
	propertyTemplate->m_module = m_module;
	m_propertyTemplateList.insertTail(propertyTemplate);
	return propertyTemplate;
}

bool
FunctionMgr::fireOnChanged()
{
	Function* function = m_currentFunction;

	ASSERT(
		function->m_functionKind == FunctionKind_Setter &&
		function->m_property &&
		function->m_property->getType()->getFlags() & PropertyTypeFlag_Bindable
		);

	Value propertyValue = function->m_property;

	if (function->m_thisType)
	{
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
FunctionMgr::setCurrentFunction(Function* function)
{
	Function* prevFunction = m_currentFunction;
	m_currentFunction = function;
	return prevFunction;
}

void
FunctionMgr::prologue(
	Function* function,
	const Token::Pos& pos
	)
{
	m_currentFunction = function;

	m_module->m_llvmIrBuilder.setCurrentDebugLoc(llvm::DebugLoc());

	// create entry blocks

	function->m_allocaBlock = m_module->m_controlFlowMgr.createBlock("function_entry");
	function->m_allocaBlock->markEntry();
	function->m_prologueBlock = m_module->m_controlFlowMgr.createBlock("function_prologue");
	function->m_prologueBlock->markEntry();

	m_module->m_controlFlowMgr.setCurrentBlock(function->m_allocaBlock);
	m_module->m_controlFlowMgr.jump(function->m_prologueBlock);
	m_module->m_llvmIrBuilder.setAllocaBlock(function->m_allocaBlock);
	m_module->m_controlFlowMgr.setCurrentBlock(function->m_prologueBlock);

	// create scope

	m_module->m_namespaceMgr.openNamespace(function->m_parentNamespace);

	function->m_scope = m_module->m_namespaceMgr.openScope(pos);

	if (function->m_extensionNamespace)
	{
		function->m_scope->m_usingSet.addGlobalNamespace(function->m_extensionNamespace);
		function->m_scope->m_usingSet.addExtensionNamespace(function->m_extensionNamespace);
	}

	if (function->m_type->getFlags() & FunctionTypeFlag_Unsafe)
		m_module->m_operatorMgr.enterUnsafeRgn();


	function->getType()->getCallConv()->createArgVariables(function);

	BasicBlock* bodyBlock = m_module->m_controlFlowMgr.createBlock("function_body");
	m_module->m_controlFlowMgr.jump(bodyBlock, bodyBlock);

	if (m_module->getCompileFlags() & ModuleCompileFlag_GcSafePointInPrologue)
		m_module->m_operatorMgr.gcSafePoint();

	// 'this' arg

	if (function->isMember())
		createThisValue();

	// static initializers

	if (function->m_functionKind == FunctionKind_StaticConstructor)
	{
		if (function->getProperty())
			function->getProperty()->initializeStaticFields();
		else if (function->getParentType())
			function->getParentType()->initializeStaticFields();
	}
}

void
FunctionMgr::createThisValue()
{
	Function* function = m_currentFunction;
	ASSERT(function && function->isMember());

	Value thisArgValue = function->getType()->getCallConv()->getThisArgValue(function);
	if (function->m_thisArgType->cmp(function->m_thisType) == 0)
	{
		if (function->m_thisType->getTypeKind() != TypeKind_DataPtr)
			m_thisValue = thisArgValue;
		else
			m_module->m_operatorMgr.makeLeanDataPtr(thisArgValue, &m_thisValue);
	}
	else
	{
		ASSERT(function->m_storageKind == StorageKind_Override);

		if (function->m_thisArgDelta == 0)
		{
			m_module->m_llvmIrBuilder.createBitCast(thisArgValue, function->m_thisType, &m_thisValue);
		}
		else
		{
			Value ptrValue;
			m_module->m_llvmIrBuilder.createBitCast(thisArgValue, m_module->m_typeMgr.getStdType(StdType_BytePtr), &ptrValue);
			m_module->m_llvmIrBuilder.createGep(ptrValue, (int32_t)function->m_thisArgDelta, NULL, &ptrValue);
			m_module->m_llvmIrBuilder.createBitCast(ptrValue, function->m_thisType, &m_thisValue);
		}
	}
}

bool
FunctionMgr::epilogue()
{
	bool result;

	Function* function = m_currentFunction;
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope();

	ASSERT(m_currentFunction && scope);

	if (function->m_functionKind == FunctionKind_Destructor)
	{
		ASSERT(m_thisValue);

		if (function->getProperty())
		{
			Property* prop = function->getProperty();
			result = prop->callMemberPropertyDestructors(m_thisValue);
		}
		else
		{
			ASSERT(function->getParentType()->getTypeKind() == TypeKind_Class && m_thisValue);
			ClassType* classType = (ClassType*)function->getParentType();

			result =
				classType->callMemberPropertyDestructors(m_thisValue) &&
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

	if (isBroken)
	{
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
	)
{
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
	const Token::Pos* pos
	)
{
	m_currentFunction = function;

	m_module->m_llvmIrBuilder.setCurrentDebugLoc(llvm::DebugLoc());

	function->m_allocaBlock = m_module->m_controlFlowMgr.createBlock("function_entry");
	function->m_allocaBlock->markEntry();
	function->m_prologueBlock = m_module->m_controlFlowMgr.createBlock("function_prologue");
	function->m_prologueBlock->markEntry();

	m_module->m_controlFlowMgr.setCurrentBlock(function->m_allocaBlock);
	m_module->m_controlFlowMgr.jump(function->m_prologueBlock);
	m_module->m_llvmIrBuilder.setAllocaBlock(function->m_allocaBlock);
	m_module->m_controlFlowMgr.setCurrentBlock(function->m_prologueBlock);

	function->m_scope = pos ?
		m_module->m_namespaceMgr.openScope(*pos) :
		m_module->m_namespaceMgr.openInternalScope();

	if (function->isMember() && function->getFunctionKind() != FunctionKind_AsyncSequencer)
		createThisValue();

	if (argCount)
	{
		llvm::Function::arg_iterator llvmArg = function->getLlvmFunction()->arg_begin();
		FunctionType* functionType = function->getType();
		CallConv* callConv = functionType->getCallConv();

		for (size_t i = 0; i < argCount; i++, llvmArg++)
		{
			Value argValue = callConv->getArgValue(&*llvmArg, functionType, i);
			argValueArray[i] = argValue;
		}
	}

	BasicBlock* bodyBlock = m_module->m_controlFlowMgr.createBlock("function_body");
	m_module->m_controlFlowMgr.jump(bodyBlock, bodyBlock);

	if (m_module->getCompileFlags() & ModuleCompileFlag_GcSafePointInInternalPrologue)
		m_module->m_operatorMgr.gcSafePoint();
}

void
FunctionMgr::internalEpilogue()
{
	Function* function = m_currentFunction;

	BasicBlock* currentBlock = m_module->m_controlFlowMgr.getCurrentBlock();
	if (!currentBlock->hasTerminator())
	{
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
	)
{
	if (!hasUnusedClosure && targetFunction->m_type->cmp(thunkFunctionType) == 0)
		return targetFunction;

	char signatureChar = 'D';

	if (hasUnusedClosure)
	{
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

	ThunkFunction* thunkFunction = (ThunkFunction*)createFunction(
		FunctionKind_Thunk,
		sl::String(),
		"jnc.directThunkFunction",
		thunkFunctionType
		);

	thunkFunction->m_storageKind = StorageKind_Static;
	thunkFunction->m_targetFunction = targetFunction;

	it->m_value = thunkFunction;

	m_module->markForCompile(thunkFunction);
	return thunkFunction;
}

Property*
FunctionMgr::getDirectThunkProperty(
	Property* targetProperty,
	PropertyType* thunkPropertyType,
	bool hasUnusedClosure
	)
{
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

	ThunkProperty* thunkProperty = (ThunkProperty*)createProperty(
		PropertyKind_Thunk,
		sl::String(),
		"jnc.g_directThunkProperty"
		);

	thunkProperty->m_storageKind = StorageKind_Static;
	thunkProperty->m_signature = signature;

	bool result = thunkProperty->create(targetProperty, thunkPropertyType, hasUnusedClosure);
	if (!result)
		return NULL;

	it->m_value = thunkProperty;

	thunkProperty->ensureLayout();
	return thunkProperty;
}

Property*
FunctionMgr::getDirectDataThunkProperty(
	Variable* targetVariable,
	PropertyType* thunkPropertyType,
	bool hasUnusedClosure
	)
{
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

	DataThunkProperty* thunkProperty = (DataThunkProperty*)createProperty(
		PropertyKind_DataThunk,
		sl::String(),
		"jnc.g_directDataThunkProperty"
		);

	thunkProperty->m_storageKind = StorageKind_Static;
	thunkProperty->m_signature = signature;
	thunkProperty->m_targetVariable = targetVariable;

	if (hasUnusedClosure)
		thunkPropertyType = thunkPropertyType->getStdObjectMemberPropertyType();

	result = thunkProperty->create(thunkPropertyType);
	if (!result)
		return NULL;

	it->m_value = thunkProperty;

	thunkProperty->ensureLayout();
	m_module->markForCompile(thunkProperty);
	return thunkProperty;
}

Function*
FunctionMgr::getSchedLauncherFunction(FunctionPtrType* targetFunctionPtrType)
{
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

	if (targetFunctionType->getFlags() & FunctionTypeFlag_Async)
	{
		Type* returnType = m_module->m_typeMgr.getStdType(StdType_PromisePtr);
		FunctionType* launcherFunctionType = m_module->m_typeMgr.getFunctionType(returnType, argArray);

		launcherFunction = createFunction(
			FunctionKind_AsyncSchedLauncher,
			sl::String(),
			"jnc.asyncSchedLauncher",
			launcherFunctionType
			);
	}
	else
	{
		FunctionType* launcherFunctionType = m_module->m_typeMgr.getFunctionType(argArray);

		launcherFunction = createFunction(
			FunctionKind_SchedLauncher,
			sl::String(),
			"jnc.schedLauncher",
			launcherFunctionType
			);
	}

	launcherFunction->m_storageKind = StorageKind_Static;

	it->m_value = launcherFunction;

	m_module->markForCompile(launcherFunction);
	return launcherFunction;
}

void
FunctionMgr::injectTlsPrologues()
{
	sl::Iterator<Function> it = m_functionList.getHead();
	for (; it; it++)
		if (it->isTlsRequired())
			injectTlsPrologue(*it);
}

void
FunctionMgr::injectTlsPrologue(Function* function)
{
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
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = tlsVariableArray[i].m_variable->getTlsField();
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
FunctionMgr::replaceAsyncAllocas()
{
	size_t count = m_asyncSequencerFunctionArray.getCount();
	for (size_t i = 0; i < count; i++)
		m_asyncSequencerFunctionArray[i]->replaceAllocas();
}

void
llvmFatalErrorHandler(
	void* context,
	const std::string& errorString,
	bool shouldGenerateCrashDump
	)
{
	throw err::Error(errorString.c_str());
}

bool
FunctionMgr::jitFunctions()
{
#if (_JNC_LLVM_NO_JIT)
	err::setFormatStringError("LLVM jitting is disabled");
	return false;
#endif

	llvm::ScopedFatalErrorHandler scopeErrorHandler(llvmFatalErrorHandler);
	llvm::ExecutionEngine* llvmExecutionEngine = m_module->getLlvmExecutionEngine();

	try
	{
		sl::Iterator<Function> it = m_functionList.getHead();
		for (; it; it++)
			if (!it->isEmpty())
				it->m_machineCode = llvmExecutionEngine->getPointerToFunction(it->getLlvmFunction());

		llvmExecutionEngine->finalizeObject();
	}
	catch(err::Error error)
	{
		err::setFormatStringError("LLVM jitting failed: %s", error->getDescription().sz());
		return false;
	}

	return true;
}

Function*
FunctionMgr::getStdFunction(StdFunc func)
{
	ASSERT((size_t)func < StdFunc__Count);

	if (m_stdFunctionArray[func])
		return m_stdFunctionArray[func];

	// 8 is enough for all the std functions

	Type* argTypeArray[8] = { 0 };
	llvm::Type* llvmArgTypeArray[8] = { 0 };

	Type* returnType;
	FunctionType* functionType;
	Function* function;

	switch (func)
	{
	case StdFunc_PrimeStaticClass:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_BoxPtr);
		argTypeArray[1] = m_module->m_typeMgr.getStdType(StdType_BytePtr);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 2);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.primeStaticClass", functionType);
		break;

	case StdFunc_TryAllocateClass:
		returnType = m_module->m_typeMgr.getStdType(StdType_AbstractClassPtr);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_BytePtr);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 1);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.tryAllocateClass", functionType);
		break;

	case StdFunc_AllocateClass:
		returnType = m_module->m_typeMgr.getStdType(StdType_AbstractClassPtr);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_BytePtr);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 1);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.allocateClass", functionType);
		break;

	case StdFunc_TryAllocateData:
		returnType = m_module->m_typeMgr.getStdType(StdType_AbstractDataPtr);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_BytePtr);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 1);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.tryAllocateData", functionType);
		break;

	case StdFunc_AllocateData:
		returnType = m_module->m_typeMgr.getStdType(StdType_AbstractDataPtr);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_BytePtr);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 1);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.allocateData", functionType);
		break;

	case StdFunc_TryAllocateArray:
		returnType = m_module->m_typeMgr.getStdType(StdType_AbstractDataPtr);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_BytePtr);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 2);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.tryAllocateArray", functionType);
		break;

	case StdFunc_AllocateArray:
		returnType = m_module->m_typeMgr.getStdType(StdType_AbstractDataPtr);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_BytePtr);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 2);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.allocateArray", functionType);
		break;

	case StdFunc_CreateDataPtrValidator:
		returnType = m_module->m_typeMgr.getStdType(StdType_DataPtrValidatorPtr);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_BoxPtr);
		argTypeArray[1] = m_module->m_typeMgr.getStdType(StdType_BytePtr);
		argTypeArray[2] = m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 3);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.createDataPtrValidator", functionType);
		break;

	case StdFunc_TryCheckDataPtrRangeIndirect:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Bool);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_BytePtr);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT);
		argTypeArray[2] = m_module->m_typeMgr.getStdType(StdType_DataPtrValidatorPtr);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 3, FunctionTypeFlag_ErrorCode);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.tryCheckDataPtrRangeIndirect", functionType);
		break;

	case StdFunc_CheckDataPtrRangeIndirect:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_BytePtr);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT);
		argTypeArray[2] = m_module->m_typeMgr.getStdType(StdType_DataPtrValidatorPtr);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 3);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.checkDataPtrRangeIndirect", functionType);
		break;

	case StdFunc_LlvmMemcpy:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_BytePtr);
		argTypeArray[1] = m_module->m_typeMgr.getStdType(StdType_BytePtr);
		argTypeArray[2] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32);
		argTypeArray[3] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32);
		argTypeArray[4] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Bool);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 5);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.llvmMemcpy", functionType);

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
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_BytePtr);
		argTypeArray[1] = m_module->m_typeMgr.getStdType(StdType_BytePtr);
		argTypeArray[2] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32);
		argTypeArray[3] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32);
		argTypeArray[4] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Bool);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 5);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.llvmMemmove", functionType);

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
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_BytePtr);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int8);
		argTypeArray[2] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32);
		argTypeArray[3] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32);
		argTypeArray[4] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Bool);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 5);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.llvmMemset", functionType);

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
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.getTls", functionType);
		break;

	case StdFunc_SetJmp:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_SjljFrame)->getDataPtrType_c();
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 1);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.setJmp", functionType);
		break;

	case StdFunc_DynamicThrow:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, NULL, 0);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.dynamicThrow", functionType);
		break;

	case StdFunc_AsyncRet:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_PromisePtr);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 2);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.asyncRet", functionType);
		break;

	case StdFunc_AsyncThrow:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_PromisePtr);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 1);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.asyncThrow", functionType);
		break;

	case StdFunc_VariantUnaryOperator:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		argTypeArray[0] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 2);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.variantUnaryOperator", functionType);
		break;

	case StdFunc_VariantBinaryOperator:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		argTypeArray[0] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		argTypeArray[2] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 3);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.variantBinaryOperator", functionType);
		break;

	case StdFunc_VariantRelationalOperator:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Bool);
		argTypeArray[0] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		argTypeArray[2] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 3);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.variantRelationalOperator", functionType);
		break;

	case StdFunc_VariantMemberOperator:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		argTypeArray[0] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		argTypeArray[1] = m_module->m_typeMgr.getStdType(jnc_StdType_CharConstPtr);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 2);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.variantMemberOperator", functionType);
		break;

	case StdFunc_VariantIndexOperator:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		argTypeArray[0] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 2);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.variantIndexOperator", functionType);
		break;

	case StdFunc_VariantMemberProperty_get:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		argTypeArray[0] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant)->getDataPtrType(DataPtrTypeKind_Normal, PtrTypeFlag_Const);
		argTypeArray[1] = m_module->m_typeMgr.getStdType(jnc_StdType_CharConstPtr);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 2);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.variantMemberProperty.get", functionType);
		break;

	case StdFunc_VariantMemberProperty_set:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		argTypeArray[0] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant)->getDataPtrType();
		argTypeArray[1] = m_module->m_typeMgr.getStdType(jnc_StdType_CharConstPtr);
		argTypeArray[2] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 3);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.variantMemberProperty.set", functionType);
		break;

	case StdFunc_VariantIndexProperty_get:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		argTypeArray[0] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant)->getDataPtrType(DataPtrTypeKind_Normal, PtrTypeFlag_Const);
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 2);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.variantIndexProperty.get", functionType);
		break;

	case StdFunc_VariantIndexProperty_set:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		argTypeArray[0] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant)->getDataPtrType();
		argTypeArray[1] = m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT);
		argTypeArray[2] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 3);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.variantIndexProperty.set", functionType);
		break;

	case StdFunc_GcSafePoint:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, NULL, 0);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.gcSafePoint", functionType);
		break;

	case StdFunc_CollectGarbage:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, NULL, 0);
		function = createFunction(FunctionKind_Normal, "collectGarbage", "jnc.collectGarbage", functionType);
		break;

	case StdFunc_GetGcStats:
		returnType = m_module->m_typeMgr.getStdType(StdType_GcStats);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, NULL, 0);
		function = createFunction(FunctionKind_Normal, "getGcStats", "jnc.getGcStats", functionType);
		break;

	case StdFunc_GcTriggers_get:
		returnType = m_module->m_typeMgr.getStdType(StdType_GcTriggers);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, NULL, 0);
		function = createFunction(FunctionKind_Getter, sl::String(), "jnc.g_gcTriggers.get", functionType);
		break;

	case StdFunc_GcTriggers_set:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_GcTriggers);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 1);
		function = createFunction(FunctionKind_Setter, sl::String(), "jnc.g_gcTriggers.set", functionType);
		break;

	case StdFunc_SetGcShadowStackFrameMap:
		returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		argTypeArray[0] = m_module->m_typeMgr.getStdType(StdType_GcShadowStackFrame)->getDataPtrType_c();
		argTypeArray[1] = m_module->m_typeMgr.getStdType(StdType_BytePtr);
		argTypeArray[2] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int);
		functionType = m_module->m_typeMgr.getFunctionType(returnType, argTypeArray, 3);
		function = createFunction(FunctionKind_Internal, sl::String(), "jnc.setGcShadowStackFrameMap", functionType);
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
	case StdFunc_TryCheckDataPtrRangeDirect:
	case StdFunc_CheckDataPtrRangeDirect:
	case StdFunc_TryLazyGetDynamicLibFunction:
	case StdFunc_LazyGetDynamicLibFunction:
	case StdFunc_GetDynamicField:
	case StdFunc_CreateConstDataPtr:
		function = parseStdFunction(func);
		break;

	case StdFunc_CreateDataPtr:
		ASSERT(m_lazyStdFunctionArray[StdFunc_CreateDataPtr]);
		m_lazyStdFunctionArray[StdFunc_CreateDataPtr]->detach();
		function = parseStdFunction(func);
		getStdFunction(StdFunc_CreateConstDataPtr); // parse and add overload, too
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
FunctionMgr::parseStdFunction(StdFunc func)
{
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
	)
{
	bool result;

	Lexer lexer;
	lexer.create("jnc_StdFunctions.jnc", source);

	if (stdNamespace)
		m_module->m_namespaceMgr.openStdNamespace(stdNamespace);

	Parser parser(m_module);
	parser.create(SymbolKind_normal_item_declaration);
	parser.m_stage = Parser::Stage_Pass2; // no imports

	for (;;)
	{
		const Token* token = lexer.getToken();

		result = parser.parseToken(token);
		if (!result)
		{
			TRACE("parse std function error: %s\n", err::getLastErrorDescription().sz());
			ASSERT(false);
		}

		if (token->m_token == TokenKind_Eof) // EOF token must be parsed
			break;

		lexer.nextToken();
	}

	if (stdNamespace)
		m_module->m_namespaceMgr.closeNamespace();

	ModuleItem* item = parser.m_lastDeclaredItem;
	ASSERT(item && item->getItemKind() == ModuleItemKind_Function);

	result = m_module->postParseStdItem();
	ASSERT(result);

	return (Function*)item;
}

LazyStdFunction*
FunctionMgr::getLazyStdFunction(StdFunc stdFunc)
{
	ASSERT((size_t)stdFunc < StdFunc__Count);

	if (m_lazyStdFunctionArray[stdFunc])
		return m_lazyStdFunctionArray[stdFunc];

	LazyStdFunction* function = AXL_MEM_NEW(LazyStdFunction);
	function->m_module = m_module;
	function->m_stdFunc = stdFunc;
	m_lazyStdFunctionList.insertTail(function);
	m_lazyStdFunctionArray[stdFunc] = function;
	return function;
}

Property*
FunctionMgr::getStdProperty(StdProp stdProp)
{
	ASSERT((size_t)stdProp < StdProp__Count);

	if (m_stdPropertyArray[stdProp])
		return m_stdPropertyArray[stdProp];

	Property* prop;
	switch (stdProp)
	{
	case StdProp_VariantMember:
		prop = createProperty(PropertyKind_Internal, sl::String(), "jnc.g_variantMember");
		prop->m_storageKind = StorageKind_Static;
		prop->m_getter = getStdFunction(StdFunc_VariantMemberProperty_get);
		prop->m_setter = getStdFunction(StdFunc_VariantMemberProperty_set);
		prop->m_type = m_module->m_typeMgr.getPropertyType(prop->m_getter->getType(), prop->m_setter->getType());
		break;

	case StdProp_VariantIndex:
		prop = createProperty(PropertyKind_Internal, sl::String(), "jnc.g_variantIndex");
		prop->m_storageKind = StorageKind_Static;
		prop->m_getter = getStdFunction(StdFunc_VariantIndexProperty_get);
		prop->m_setter = getStdFunction(StdFunc_VariantIndexProperty_set);
		prop->m_type = m_module->m_typeMgr.getPropertyType(prop->m_getter->getType(), prop->m_setter->getType());
		break;

	case StdProp_GcTriggers:
		prop = createProperty(PropertyKind_Normal, "g_gcTriggers", "jnc.g_gcTriggers");
		prop->m_storageKind = StorageKind_Static;
		prop->m_getter = getStdFunction(StdFunc_GcTriggers_get);
		prop->m_setter = getStdFunction(StdFunc_GcTriggers_set);
		prop->m_type = m_module->m_typeMgr.getPropertyType(prop->m_getter->getType(), prop->m_setter->getType());
		break;

	default:
		ASSERT(false);
		prop = NULL;
	}

	m_stdPropertyArray[stdProp] = prop;
	return prop;
}

LazyStdProperty*
FunctionMgr::getLazyStdProperty(StdProp stdProp)
{
	ASSERT((size_t)stdProp < StdProp__Count);

	if (m_lazyStdPropertyArray[stdProp])
		return m_lazyStdPropertyArray[stdProp];

	LazyStdProperty* prop = AXL_MEM_NEW(LazyStdProperty);
	prop->m_module = m_module;
	prop->m_stdProp = stdProp;
	m_lazyStdPropertyList.insertTail(prop);
	m_lazyStdPropertyArray[stdProp] = prop;
	return prop;
}

//..............................................................................

} // namespace ct
} // namespace jnc
