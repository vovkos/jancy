#include "pch.h"
#include "jnc_ct_FunctionMgr.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_Parser.llk.h"

// #define _JNC_NO_JIT
// #define _JNC_NO_VERIFY

namespace jnc {
namespace ct {

//.............................................................................

FunctionMgr::FunctionMgr ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);

	memset (m_stdFunctionArray, 0, sizeof (m_stdFunctionArray));
	memset (m_lazyStdFunctionArray, 0, sizeof (m_lazyStdFunctionArray));
	m_currentFunction = NULL;
}

void
FunctionMgr::clear ()
{
	m_functionList.clear ();
	m_propertyList.clear ();
	m_propertyTemplateList.clear ();
	m_scheduleLauncherFunctionList.clear ();
	m_thunkFunctionList.clear ();
	m_thunkPropertyList.clear ();
	m_dataThunkPropertyList.clear ();
	m_thunkFunctionMap.clear ();
	m_thunkPropertyMap.clear ();
	m_scheduleLauncherFunctionMap.clear ();
	m_staticConstructArray.clear ();
	memset (m_stdFunctionArray, 0, sizeof (m_stdFunctionArray));
	memset (m_lazyStdFunctionArray, 0, sizeof (m_lazyStdFunctionArray));

	m_thisValue.clear ();
	m_currentFunction = NULL;
}

void
FunctionMgr::callStaticConstructors ()
{
	Function* addDestructor = getStdFunction (StdFunc_AddStaticDestructor);
	Type* dtorType = m_module->m_typeMgr.getStdType (StdType_BytePtr);

	size_t count = m_staticConstructArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		NamedTypeBlock* namedTypeBlock = m_staticConstructArray [i];

		Function* destructor = namedTypeBlock->getStaticDestructor ();
		if (destructor)
		{
			Value dtorValue;
			m_module->m_llvmIrBuilder.createBitCast (destructor, dtorType, &dtorValue);
			m_module->m_llvmIrBuilder.createCall (addDestructor, addDestructor->getType (), dtorValue, NULL);
		}

		Function* constructor = namedTypeBlock->getStaticConstructor ();
		ASSERT (constructor);

		m_module->m_llvmIrBuilder.createCall (constructor, constructor->getType (), NULL);
	}
}

Value
FunctionMgr::overrideThisValue (const Value& value)
{
	Value prevThisValue = m_thisValue;
	m_thisValue = value;
	return prevThisValue;
}

Function*
FunctionMgr::createFunction (
	FunctionKind functionKind,
	const sl::String& name,
	const sl::String& qualifiedName,
	const sl::String& tag,
	FunctionType* type
	)
{
	Function* function;
	switch (functionKind)
	{
	case FunctionKind_Thunk:
		function = AXL_MEM_NEW (ThunkFunction);
		m_thunkFunctionList.insertTail ((ThunkFunction*) function);
		break;

	case FunctionKind_ScheduleLauncher:
		function = AXL_MEM_NEW (ScheduleLauncherFunction);
		m_scheduleLauncherFunctionList.insertTail ((ScheduleLauncherFunction*) function);
		break;

	default:
		function = AXL_MEM_NEW (Function);
		m_functionList.insertTail (function);
	}

	function->m_module = m_module;
	function->m_functionKind = functionKind;
	function->m_name = name;
	function->m_qualifiedName = qualifiedName;
	function->m_tag = tag;
	function->m_type = type;
	function->m_typeOverload.addOverload (type);
	return function;
}

Property*
FunctionMgr::createProperty (
	PropertyKind propertyKind,
	const sl::String& name,
	const sl::String& qualifiedName,
	const sl::String& tag
	)
{
	Property* prop;

	switch (propertyKind)
	{
	case PropertyKind_Thunk:
		prop = AXL_MEM_NEW (ThunkProperty);
		m_thunkPropertyList.insertTail ((ThunkProperty*) prop);
		break;

	case PropertyKind_DataThunk:
		prop = AXL_MEM_NEW (DataThunkProperty);
		m_dataThunkPropertyList.insertTail ((DataThunkProperty*) prop);
		break;

	default:
		prop = AXL_MEM_NEW (Property);
		m_propertyList.insertTail (prop);
	}

	prop->m_module = m_module;
	prop->m_propertyKind = propertyKind;
	prop->m_name = name;
	prop->m_qualifiedName = qualifiedName;
	prop->m_tag = tag;
	m_module->markForLayout (prop, true);
	return prop;
}

PropertyTemplate*
FunctionMgr::createPropertyTemplate ()
{
	PropertyTemplate* propertyTemplate = AXL_MEM_NEW (PropertyTemplate);
	propertyTemplate->m_module = m_module;
	m_propertyTemplateList.insertTail (propertyTemplate);
	return propertyTemplate;
}

bool
FunctionMgr::fireOnChanged ()
{
	Function* function = m_currentFunction;

	ASSERT (
		function->m_functionKind == FunctionKind_Setter &&
		function->m_property &&
		function->m_property->getType ()->getFlags () & PropertyTypeFlag_Bindable
		);

	Value propertyValue = function->m_property;

	if (function->m_thisType)
	{
		ASSERT (m_thisValue);

		Closure* closure = propertyValue.createClosure ();
		closure->insertThisArgValue (m_thisValue);
	}

	Value onChanged;

	return
		m_module->m_operatorMgr.getPropertyOnChanged (propertyValue, &onChanged) &&
		m_module->m_operatorMgr.memberOperator (&onChanged, "call") &&
		m_module->m_operatorMgr.callOperator (onChanged);
}

Function*
FunctionMgr::setCurrentFunction (Function* function)
{
	Function* prevFunction = m_currentFunction;
	m_currentFunction = function;
	return prevFunction;
}

bool
FunctionMgr::prologue (
	Function* function,
	const Token::Pos& pos
	)
{
	bool result;

	m_currentFunction = function;

	// create entry block 

	function->m_entryBlock = m_module->m_controlFlowMgr.createBlock ("function_entry");
	function->m_entryBlock->markEntry ();

	m_module->m_controlFlowMgr.setCurrentBlock (function->m_entryBlock);

	// create scope

	m_module->m_namespaceMgr.openNamespace (function->m_parentNamespace);

	function->m_scope = m_module->m_namespaceMgr.openScope (pos);

	if (function->m_extensionNamespace)
	{
		function->m_scope->m_usingSet.addGlobalNamespace (function->m_extensionNamespace);
		function->m_scope->m_usingSet.addExtensionNamespace (function->m_extensionNamespace);
	}

	if (function->m_type->getFlags () & FunctionTypeFlag_Unsafe)
		m_module->m_operatorMgr.enterUnsafeRgn ();


	function->getType ()->getCallConv ()->createArgVariables (function);

	BasicBlock* bodyBlock = m_module->m_controlFlowMgr.createBlock ("function_body");
	m_module->m_controlFlowMgr.jump (bodyBlock, bodyBlock);

	uint_t compileFlags = m_module->getCompileFlags ();

	if (compileFlags & ModuleCompileFlag_CheckStackOverflowInPrologue)
		m_module->m_operatorMgr.checkStackOverflow ();

	if (compileFlags & ModuleCompileFlag_GcSafePointInPrologue)
		m_module->m_operatorMgr.gcSafePoint ();

	if (function->m_functionKind == FunctionKind_ModuleConstructor)
	{
		result = m_module->m_variableMgr.allocateInitializeGlobalVariables ();
		if (!result)
			return false;

		callStaticConstructors ();
	}

	// 'this' arg

	if (function->isMember ())
		createThisValue ();

	// static initializers

	if (function->m_functionKind == FunctionKind_StaticConstructor)
	{
		if (function->getProperty ())
			function->getProperty ()->initializeStaticFields ();
		else if (function->getParentType ())
			function->getParentType ()->initializeStaticFields ();		
	}

	return true;
}

void
FunctionMgr::createThisValue ()
{
	Function* function = m_currentFunction;
	ASSERT (function && function->isMember ());

	Value thisArgValue = function->getType ()->getCallConv ()->getThisArgValue (function);
	if (function->m_thisArgType->cmp (function->m_thisType) == 0)
	{
		if (function->m_thisType->getTypeKind () != TypeKind_DataPtr)
		{
			m_thisValue = thisArgValue;
		}
		else // make it lean
		{
			ASSERT (
				thisArgValue.getType ()->getTypeKind () == TypeKind_DataPtr &&
				((DataPtrType*) thisArgValue.getType ())->getPtrTypeKind () == DataPtrTypeKind_Normal);

			DataPtrType* ptrType = ((DataPtrType*) thisArgValue.getType ());
			ptrType = ptrType->getTargetType ()->getDataPtrType (DataPtrTypeKind_Lean, ptrType->getFlags ());
			Type* validatorType = m_module->m_typeMgr.getStdType (StdType_DataPtrValidatorPtr);

			Value ptrValue;
			Value validatorValue;
			m_module->m_llvmIrBuilder.createExtractValue (thisArgValue, 0, NULL, &ptrValue);
			m_module->m_llvmIrBuilder.createExtractValue (thisArgValue, 1, validatorType, &validatorValue);
			m_module->m_llvmIrBuilder.createBitCast (ptrValue, ptrType, &ptrValue);
			m_thisValue.setLeanDataPtr (ptrValue.getLlvmValue (), ptrType, validatorValue);
		}
	}
	else
	{
		ASSERT (function->m_storageKind == StorageKind_Override);
		
		if (function->m_thisArgDelta == 0)
		{
			m_module->m_llvmIrBuilder.createBitCast (thisArgValue, function->m_thisType, &m_thisValue);
		}
		else
		{
			Value ptrValue;
			m_module->m_llvmIrBuilder.createBitCast (thisArgValue, m_module->m_typeMgr.getStdType (StdType_BytePtr), &ptrValue);
			m_module->m_llvmIrBuilder.createGep (ptrValue, (int32_t) function->m_thisArgDelta, NULL, &ptrValue);
			m_module->m_llvmIrBuilder.createBitCast (ptrValue, function->m_thisType, &m_thisValue);
		}
	}
}

bool
FunctionMgr::epilogue ()
{
	bool result;

	Function* function = m_currentFunction;
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();

	ASSERT (m_currentFunction && scope);

	if (function->m_functionKind == FunctionKind_Destructor)
	{
		ASSERT (m_thisValue);

		if (function->getProperty ())
		{
			Property* prop = function->getProperty ();
			result = prop->callMemberPropertyDestructors (m_thisValue);
		}
		else
		{
			ASSERT (function->getParentType ()->getTypeKind () == TypeKind_Class && m_thisValue);
			ClassType* classType = (ClassType*) function->getParentType ();

			result =
				classType->callMemberPropertyDestructors (m_thisValue) &&
				classType->callBaseTypeDestructors (m_thisValue);
		}

		if (!result)
			return false;
	}

	result = m_module->m_controlFlowMgr.checkReturn ();
	if (!result)
		return false;

	if (function->m_type->getFlags () & FunctionTypeFlag_Unsafe)
		m_module->m_operatorMgr.leaveUnsafeRgn ();

	finalizeFunction (function, true);

#if (defined (_AXL_DEBUG) && !defined (_JNC_NO_VERIFY))
	try
	{
		llvm::verifyFunction (*function->getLlvmFunction (), llvm::ReturnStatusAction);
	}
	catch (err::Error error)
	{
		err::setFormatStringError (
			"LLVM verification fail for '%s': %s",
			function->m_tag.cc (),
			error->getDescription ().cc ()
			);

		return false;
	}
#endif

	return true;
}

void
FunctionMgr::finalizeFunction (
	Function* function,
	bool wasNamespaceOpened
	)
{
	ASSERT (function == m_currentFunction);

	m_module->m_namespaceMgr.closeScope ();

	if (wasNamespaceOpened)
		m_module->m_namespaceMgr.closeNamespace ();

	m_module->m_operatorMgr.resetUnsafeRgn ();
	m_module->m_variableMgr.finalizeLiftedStackVariables ();
	m_module->m_gcShadowStackMgr.finalizeFunction ();
	m_module->m_controlFlowMgr.finalizeFunction ();
	m_module->m_llvmIrBuilder.moveAllAllocas (function->getEntryBlock ());

	size_t count = function->m_tlsVariableArray.getCount ();
	for (size_t i = 0; i < count; i++)
		function->m_tlsVariableArray [i].m_variable->m_llvmValue = NULL;

	m_thisValue.clear ();
	m_currentFunction = NULL;
}

void
FunctionMgr::internalPrologue (
	Function* function,
	Value* argValueArray,
	size_t argCount
	)
{
	m_currentFunction = function;

	m_module->m_llvmIrBuilder.setCurrentDebugLoc (llvm::DebugLoc ());

	function->m_entryBlock = m_module->m_controlFlowMgr.createBlock ("function_entry");
	function->m_entryBlock->markEntry ();

	m_module->m_controlFlowMgr.setCurrentBlock (function->m_entryBlock);

	function->m_scope = m_module->m_namespaceMgr.openInternalScope ();

	if (function->isMember ())
		createThisValue ();

	if (argCount)
	{
		llvm::Function::arg_iterator llvmArg = function->getLlvmFunction ()->arg_begin ();
		FunctionType* functionType = function->getType ();
		CallConv* callConv = functionType->getCallConv ();

		for (size_t i = 0; i < argCount; i++, llvmArg++)
		{
			Value argValue = callConv->getArgValue (llvmArg, functionType, i);
			argValueArray [i] = argValue;

			AXL_TODO ("remove arg GC roots and instead add all user-allocated data to some kind of call-site-root-set")

			Type* argType = argValue.getType ();
			if (argType->getFlags () & TypeFlag_GcRoot)
			{
				Value argVariable;
				m_module->m_llvmIrBuilder.createAlloca (argType, "gcRoot", NULL, &argVariable);
				m_module->m_llvmIrBuilder.createStore (argValue, argVariable);
				m_module->m_gcShadowStackMgr.markGcRoot (argVariable, argType);
			}
		}
	}

	BasicBlock* bodyBlock = m_module->m_controlFlowMgr.createBlock ("function_body");
	m_module->m_controlFlowMgr.jump (bodyBlock, bodyBlock);

	uint_t compileFlags = m_module->getCompileFlags ();

	if (compileFlags & ModuleCompileFlag_CheckStackOverflowInInternalPrologue)
		m_module->m_operatorMgr.checkStackOverflow ();

	if (compileFlags & ModuleCompileFlag_GcSafePointInInternalPrologue)
		m_module->m_operatorMgr.gcSafePoint ();
}

void
FunctionMgr::internalEpilogue ()
{
	Function* function = m_currentFunction;

	BasicBlock* currentBlock = m_module->m_controlFlowMgr.getCurrentBlock ();
	if (!currentBlock->hasTerminator ())
	{
		Type* returnType = function->getType ()->getReturnType ();

		Value returnValue;
		if (returnType->getTypeKind () != TypeKind_Void)
			returnValue = returnType->getZeroValue ();

		m_module->m_controlFlowMgr.ret (returnValue);
	}

	finalizeFunction (function, false);
}

Function*
FunctionMgr::getDirectThunkFunction (
	Function* targetFunction,
	FunctionType* thunkFunctionType,
	bool hasUnusedClosure
	)
{
	if (!hasUnusedClosure && targetFunction->m_type->cmp (thunkFunctionType) == 0)
		return targetFunction;

	char signatureChar = 'D';

	if (hasUnusedClosure)
	{
		signatureChar = 'U';
		thunkFunctionType = thunkFunctionType->getStdObjectMemberMethodType ();
	}

	sl::String signature;
	signature.format (
		"%c%x.%s",
		signatureChar,
		targetFunction,
		thunkFunctionType->getSignature ().cc ()
		);

	sl::StringHashTableMapIterator <Function*> thunk = m_thunkFunctionMap.visit (signature);
	if (thunk->m_value)
		return thunk->m_value;

	ThunkFunction* thunkFunction = (ThunkFunction*) createFunction (FunctionKind_Thunk, thunkFunctionType);
	thunkFunction->m_storageKind = StorageKind_Static;
	thunkFunction->m_tag = "directThunkFunction";
	thunkFunction->m_signature = signature;
	thunkFunction->m_targetFunction = targetFunction;

	thunk->m_value = thunkFunction;

	m_module->markForCompile (thunkFunction);
	return thunkFunction;
}

Property*
FunctionMgr::getDirectThunkProperty (
	Property* targetProperty,
	PropertyType* thunkPropertyType,
	bool hasUnusedClosure
	)
{
	if (!hasUnusedClosure && targetProperty->m_type->cmp (thunkPropertyType) == 0)
		return targetProperty;

	sl::String signature;
	signature.format (
		"%c%x.%s",
		hasUnusedClosure ? 'U' : 'D',
		targetProperty,
		thunkPropertyType->getSignature ().cc ()
		);

	sl::StringHashTableMapIterator <Property*> thunk = m_thunkPropertyMap.visit (signature);
	if (thunk->m_value)
		return thunk->m_value;

	ThunkProperty* thunkProperty = (ThunkProperty*) createProperty (PropertyKind_Thunk);
	thunkProperty->m_storageKind = StorageKind_Static;
	thunkProperty->m_signature = signature;
	thunkProperty->m_tag = "g_directThunkProperty";

	bool result = thunkProperty->create (targetProperty, thunkPropertyType, hasUnusedClosure);
	if (!result)
		return NULL;

	thunk->m_value = thunkProperty;

	thunkProperty->ensureLayout ();
	return thunkProperty;
}

Property*
FunctionMgr::getDirectDataThunkProperty (
	Variable* targetVariable,
	PropertyType* thunkPropertyType,
	bool hasUnusedClosure
	)
{
	bool result;

	sl::String signature;
	signature.format (
		"%c%x.%s",
		hasUnusedClosure ? 'U' : 'D',
		targetVariable,
		thunkPropertyType->getSignature ().cc ()
		);

	sl::StringHashTableMapIterator <Property*> thunk = m_thunkPropertyMap.visit (signature);
	if (thunk->m_value)
		return thunk->m_value;

	DataThunkProperty* thunkProperty = (DataThunkProperty*) createProperty (PropertyKind_DataThunk);
	thunkProperty->m_storageKind = StorageKind_Static;
	thunkProperty->m_signature = signature;
	thunkProperty->m_targetVariable = targetVariable;
	thunkProperty->m_tag = "g_directDataThunkProperty";

	if (hasUnusedClosure)
		thunkPropertyType = thunkPropertyType->getStdObjectMemberPropertyType ();

	result = thunkProperty->create (thunkPropertyType);
	if (!result)
		return NULL;

	thunk->m_value = thunkProperty;

	thunkProperty->ensureLayout ();
	m_module->markForCompile (thunkProperty);
	return thunkProperty;
}

Function*
FunctionMgr::getScheduleLauncherFunction (
	FunctionPtrType* targetFunctionPtrType,
	ClassPtrTypeKind schedulerPtrTypeKind
	)
{
	sl::String signature = targetFunctionPtrType->getSignature ();
	if (schedulerPtrTypeKind == ClassPtrTypeKind_Weak)
		signature += ".w";

	sl::StringHashTableMapIterator <Function*> thunk = m_scheduleLauncherFunctionMap.visit (signature);
	if (thunk->m_value)
		return thunk->m_value;

	ClassPtrType* schedulerPtrType = ((ClassType*) m_module->m_typeMgr.getStdType (StdType_Scheduler))->getClassPtrType (schedulerPtrTypeKind);

	sl::Array <FunctionArg*> argArray  = targetFunctionPtrType->getTargetType ()->getArgArray ();
	argArray.insert (0, targetFunctionPtrType->getSimpleFunctionArg ());
	argArray.insert (1, schedulerPtrType->getSimpleFunctionArg ());

	FunctionType* launcherType = m_module->m_typeMgr.getFunctionType (argArray);
	ScheduleLauncherFunction* launcherFunction = (ScheduleLauncherFunction*) createFunction (FunctionKind_ScheduleLauncher, launcherType);
	launcherFunction->m_storageKind = StorageKind_Static;
	launcherFunction->m_tag = "scheduleLauncherFunction";
	launcherFunction->m_signature = signature;

	thunk->m_value = launcherFunction;

	m_module->markForCompile (launcherFunction);
	return launcherFunction;
}

bool
FunctionMgr::injectTlsPrologues ()
{
	sl::Iterator <Function> functionIt = m_functionList.getHead ();
	for (; functionIt; functionIt++)
	{
		Function* function = *functionIt;
		if (function->getEntryBlock () && function->isTlsRequired ())
			injectTlsPrologue (function);
	}

	sl::Iterator <ThunkFunction> thunkFunctionIt = m_thunkFunctionList.getHead ();
	for (; thunkFunctionIt; thunkFunctionIt++)
	{
		Function* function = *thunkFunctionIt;
		if (function->isTlsRequired ())
			injectTlsPrologue (function);
	}

	sl::Iterator <ScheduleLauncherFunction> scheduleLauncherFunctionIt = m_scheduleLauncherFunctionList.getHead ();
	for (; scheduleLauncherFunctionIt; scheduleLauncherFunctionIt++)
	{
		Function* function = *scheduleLauncherFunctionIt;
		if (function->isTlsRequired ())
			injectTlsPrologue (function);
	}

	return true;
}

void
FunctionMgr::injectTlsPrologue (Function* function)
{
	BasicBlock* block = function->getEntryBlock ();
	ASSERT (block);

	m_module->m_controlFlowMgr.setCurrentBlock (block);
	m_module->m_llvmIrBuilder.setInsertPoint (block->getLlvmBlock ()->begin ());

	Function* getTls = getStdFunction (StdFunc_GetTls);
	
	Value tlsValue;
	m_module->m_llvmIrBuilder.createCall (getTls, getTls->getType (), &tlsValue);

	// tls variables used in this function

	sl::Array <TlsVariable> tlsVariableArray = function->getTlsVariableArray ();
	size_t count = tlsVariableArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = tlsVariableArray [i].m_variable->getTlsField ();
		ASSERT (field);

		Value ptrValue;
		m_module->m_llvmIrBuilder.createGep2 (tlsValue, field->getLlvmIndex (), NULL, &ptrValue);
		tlsVariableArray [i].m_llvmAlloca->replaceAllUsesWith (ptrValue.getLlvmValue ());
	}

	// unfortunately, erasing could not be safely done inside the above loop (cause of InsertPoint)
	// so just have a dedicated loop for erasing alloca's

	count = tlsVariableArray.getCount ();
	for (size_t i = 0; i < count; i++)
		tlsVariableArray [i].m_llvmAlloca->eraseFromParent ();
}

void
llvmFatalErrorHandler (
	void* context,
	const std::string& errorString,
	bool shouldGenerateCrashDump
	)
{
	throw err::createStringError (errorString.c_str ());
}

bool
FunctionMgr::jitFunctions ()
{
#ifdef _JNC_NO_JIT
	err::setFormatStringError ("LLVM jitting is disabled");
	return false;
#endif

	llvm::ScopedFatalErrorHandler scopeErrorHandler (llvmFatalErrorHandler);

	llvm::ExecutionEngine* llvmExecutionEngine = m_module->getLlvmExecutionEngine ();
	
	try
	{
		sl::Iterator <Function> functionIt = m_functionList.getHead ();
		for (; functionIt; functionIt++)
		{
			Function* function = *functionIt;
			if (!function->getEntryBlock ())
				continue;

			llvm::Function* llvmFunction = function->getLlvmFunction ();
			function->m_machineCode = llvmExecutionEngine->getPointerToFunction (llvmFunction);
		}

		llvmExecutionEngine->finalizeObject ();
	}
	catch (err::Error error)
	{
		err::setFormatStringError ("LLVM jitting failed: %s", error->getDescription ().cc ());
		return false;
	}

	return true;
}

Function*
FunctionMgr::getStdFunction (StdFunc func)
{
	ASSERT ((size_t) func < StdFunc__Count);

	if (m_stdFunctionArray [func])
		return m_stdFunctionArray [func];

	// 8 is enough for all the std functions

	Type* argTypeArray [8] = { 0 }; 
	llvm::Type* llvmArgTypeArray [8] = { 0 };

	Type* returnType;
	FunctionType* functionType;
	Function* function;
	const StdItemSource* source;

	switch (func)
	{
	case StdFunc_PrimeStaticClass:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_BoxPtr);
		argTypeArray [1] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 2);
		function = createFunction (FunctionKind_Internal, "jnc.primeStaticClass", functionType);
		break;

	case StdFunc_TryAllocateClass:
		returnType = m_module->m_typeMgr.getStdType (StdType_AbstractClassPtr);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 1);
		function = createFunction (FunctionKind_Internal, "jnc.tryAllocateClass", functionType);
		break;

	case StdFunc_AllocateClass:
		returnType = m_module->m_typeMgr.getStdType (StdType_AbstractClassPtr);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 1);
		function = createFunction (FunctionKind_Internal, "jnc.allocateClass", functionType);
		break;

	case StdFunc_TryAllocateData:
		returnType = m_module->m_typeMgr.getStdType (StdType_DataPtrStruct);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 1);
		function = createFunction (FunctionKind_Internal, "jnc.tryAllocateData", functionType);
		break;

	case StdFunc_AllocateData:
		returnType = m_module->m_typeMgr.getStdType (StdType_DataPtrStruct);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 1);
		function = createFunction (FunctionKind_Internal, "jnc.allocateData", functionType);
		break;

	case StdFunc_TryAllocateArray:
		returnType = m_module->m_typeMgr.getStdType (StdType_DataPtrStruct);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		argTypeArray [1] = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 2);
		function = createFunction (FunctionKind_Internal, "jnc.tryAllocateArray", functionType);
		break;

	case StdFunc_AllocateArray:
		returnType = m_module->m_typeMgr.getStdType (StdType_DataPtrStruct);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		argTypeArray [1] = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 2);
		function = createFunction (FunctionKind_Internal, "jnc.allocateArray", functionType);
		break;

	case StdFunc_CreateDataPtrValidator:
		returnType = m_module->m_typeMgr.getStdType (StdType_DataPtrValidatorPtr);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_BoxPtr);
		argTypeArray [1] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		argTypeArray [2] = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 3);
		function = createFunction (FunctionKind_Internal, "jnc.createDataPtrValidator", functionType);
		break;

	case StdFunc_TryCheckDataPtrRangeIndirect:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Bool);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		argTypeArray [1] = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);
		argTypeArray [2] = m_module->m_typeMgr.getStdType (StdType_DataPtrValidatorPtr);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 3, FunctionTypeFlag_ErrorCode);
		function = createFunction (FunctionKind_Internal, "jnc.tryCheckDataPtrRangeIndirect", functionType);
		break;

	case StdFunc_CheckDataPtrRangeIndirect:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		argTypeArray [1] = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);
		argTypeArray [2] = m_module->m_typeMgr.getStdType (StdType_DataPtrValidatorPtr);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 3);
		function = createFunction (FunctionKind_Internal, "jnc.checkDataPtrRangeIndirect", functionType);
		break;

	case StdFunc_LlvmMemcpy:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		argTypeArray [1] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		argTypeArray [2] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32);
		argTypeArray [3] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32);
		argTypeArray [4] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Bool);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 5);
		function = createFunction (FunctionKind_Internal, "jnc.llvmMemcpy", functionType);

		llvmArgTypeArray [0] = argTypeArray [0]->getLlvmType ();
		llvmArgTypeArray [1] = argTypeArray [1]->getLlvmType ();
		llvmArgTypeArray [2] = argTypeArray [2]->getLlvmType ();
		function->m_llvmFunction = llvm::Intrinsic::getDeclaration (
			m_module->getLlvmModule (), 
			llvm::Intrinsic::memcpy,
			llvm::ArrayRef <llvm::Type*> (llvmArgTypeArray, 3)
			);
		break;

	case StdFunc_LlvmMemmove:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		argTypeArray [1] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		argTypeArray [2] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32);
		argTypeArray [3] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32);
		argTypeArray [4] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Bool);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 5);
		function = createFunction (FunctionKind_Internal, "jnc.llvmMemmove", functionType);

		llvmArgTypeArray [0] = argTypeArray [0]->getLlvmType ();
		llvmArgTypeArray [1] = argTypeArray [1]->getLlvmType ();
		llvmArgTypeArray [2] = argTypeArray [2]->getLlvmType ();
		function->m_llvmFunction = llvm::Intrinsic::getDeclaration (
			m_module->getLlvmModule (), 
			llvm::Intrinsic::memmove,
			llvm::ArrayRef <llvm::Type*> (llvmArgTypeArray, 3)
			);
		break;

	case StdFunc_LlvmMemset:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		argTypeArray [1] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int8);
		argTypeArray [2] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32);
		argTypeArray [3] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32);
		argTypeArray [4] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Bool);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 5);
		function = createFunction (FunctionKind_Internal, "jnc.llvmMemset", functionType);

		llvmArgTypeArray [0] = argTypeArray [0]->getLlvmType ();
		llvmArgTypeArray [1] = argTypeArray [2]->getLlvmType ();
		function->m_llvmFunction = llvm::Intrinsic::getDeclaration (
			m_module->getLlvmModule (), 
			llvm::Intrinsic::memset,
			llvm::ArrayRef <llvm::Type*> (llvmArgTypeArray, 2)
			);

		break;

	case StdFunc_GetTls:
		returnType = m_module->m_variableMgr.getTlsStructType ()->getDataPtrType (DataPtrTypeKind_Thin);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, NULL, 0);
		function = createFunction (FunctionKind_Internal, "jnc.getTls", functionType);
		break;

	case StdFunc_SetJmp:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_SjljFrame)->getDataPtrType_c ();
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 1);
		function = createFunction (FunctionKind_Internal, "jnc.setJmp", functionType);
		break;

	case StdFunc_DynamicThrow:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, NULL, 0);
		function = createFunction (FunctionKind_Internal, "jnc.dynamicThrow", functionType);
		break;
		
	case StdFunc_VariantUnaryOperator:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Variant);
		argTypeArray [0] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int);
		argTypeArray [1] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Variant);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 2);
		function = createFunction (FunctionKind_Internal, "jnc.variantUnaryOperator", functionType);
		break;

	case StdFunc_VariantBinaryOperator:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Variant);
		argTypeArray [0] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int);
		argTypeArray [1] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Variant);
		argTypeArray [2] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Variant);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 3);
		function = createFunction (FunctionKind_Internal, "jnc.variantBinaryOperator", functionType);
		break;

	case StdFunc_VariantRelationalOperator:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Bool);
		argTypeArray [0] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int);
		argTypeArray [1] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Variant);
		argTypeArray [2] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Variant);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 3);
		function = createFunction (FunctionKind_Internal, "jnc.variantRelationalOperator", functionType);
		break;

	case StdFunc_GcSafePoint:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, NULL, 0);
		function = createFunction (FunctionKind_Internal, "jnc.gcSafePoint", functionType);
		break;

	case StdFunc_SetGcShadowStackFrameMap:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_GcShadowStackFrame)->getDataPtrType_c ();
		argTypeArray [1] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		argTypeArray [2] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Bool);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 3);
		function = createFunction (FunctionKind_Internal, "jnc.setGcShadowStackFrameMap", functionType);
		break;

	case StdFunc_DynamicSizeOf:
	case StdFunc_DynamicCountOf:
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
	case StdFunc_TryCheckNullPtr:
	case StdFunc_CheckNullPtr:
	case StdFunc_CheckStackOverflow:
	case StdFunc_CheckDivByZero_i32:
	case StdFunc_CheckDivByZero_i64:
	case StdFunc_CheckDivByZero_f32:
	case StdFunc_CheckDivByZero_f64:
	case StdFunc_TryLazyGetDynamicLibFunction:
	case StdFunc_LazyGetDynamicLibFunction:
		source = getStdFunctionSource (func);			
		ASSERT (source->m_source);

		function = parseStdFunction (
			source->m_stdNamespace,
			source->m_source,
			source->m_length
			);
		break;

	case StdFunc_SimpleMulticastCall:
		function = ((MulticastClassType*) m_module->m_typeMgr.getStdType (StdType_SimpleMulticast))->getMethod (MulticastMethodKind_Call);
		break;

	default:
		ASSERT (false);
		function = NULL;
	}

	m_stdFunctionArray [func] = function;
	return function;
}

Function*
FunctionMgr::parseStdFunction (
	StdNamespace stdNamespace,
	const char* source,
	size_t length
	)
{
	bool result;

	Lexer lexer;
	lexer.create ("jnc_StdFunctions.jnc", source, length);

	if (stdNamespace)
		m_module->m_namespaceMgr.openStdNamespace (stdNamespace);

	Parser parser (m_module);
	parser.create (SymbolKind_normal_item_declaration);
	parser.m_stage = Parser::StageKind_Pass2; // no imports

	for (;;)
	{
		const Token* token = lexer.getToken ();

		result = parser.parseToken (token);
		if (!result)
		{
			dbg::trace ("parse std function error: %s\n", err::getLastErrorDescription ().cc ());
			ASSERT (false);
		}

		if (token->m_token == TokenKind_Eof) // EOF token must be parsed
			break;

		lexer.nextToken ();
	}

	if (stdNamespace)
		m_module->m_namespaceMgr.closeNamespace ();

	ModuleItem* item = parser.m_lastDeclaredItem;
	ASSERT (item && item->getItemKind () == ModuleItemKind_Function);

	result = m_module->postParseStdItem ();
	ASSERT (result);

	return (Function*) item;
}

LazyStdFunction*
FunctionMgr::getLazyStdFunction (StdFunc func)
{
	ASSERT ((size_t) func < StdFunc__Count);

	if (m_lazyStdFunctionArray [func])
		return m_lazyStdFunctionArray [func];

	LazyStdFunction* function = AXL_MEM_NEW (LazyStdFunction);
	function->m_module = m_module;
	function->m_func = func;
	m_lazyStdFunctionList.insertTail (function);
	m_lazyStdFunctionArray [func] = function;
	return function;
}

//.............................................................................

} // namespace ct
} // namespace jnc

