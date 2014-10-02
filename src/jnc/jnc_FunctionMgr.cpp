#include "pch.h"
#include "jnc_FunctionMgr.h"
#include "jnc_GcShadowStack.h"
#include "jnc_Module.h"

//#define _JNC_NO_JIT

namespace jnc {

//.............................................................................

FunctionMgr::FunctionMgr ()
{
	m_module = getCurrentThreadModule ();
	ASSERT (m_module);

	m_currentFunction = NULL;
	memset (m_stdFunctionArray, 0, sizeof (m_stdFunctionArray));
	memset (m_lazyStdFunctionArray, 0, sizeof (m_lazyStdFunctionArray));

	mt::callOnce (registerGcShadowStack, 0);
}

void
FunctionMgr::clear ()
{
	m_llvmFunctionMap.clear ();
	m_functionList.clear ();
	m_propertyList.clear ();
	m_propertyTemplateList.clear ();
	m_scheduleLauncherFunctionList.clear ();
	m_thunkFunctionList.clear ();
	m_thunkPropertyList.clear ();
	m_dataThunkPropertyList.clear ();
	m_thunkFunctionMap.clear ();
	m_thunkPropertyMap.clear ();
	m_lazyStdFunctionList.clear ();
	m_scheduleLauncherFunctionMap.clear ();
	m_emissionContextStack.clear ();
	m_thisValue.clear ();
	m_scopeLevelValue.clear ();

	m_currentFunction = NULL;
	memset (m_stdFunctionArray, 0, sizeof (m_stdFunctionArray));
	memset (m_lazyStdFunctionArray, 0, sizeof (m_lazyStdFunctionArray));
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
	const rtl::String& name,
	const rtl::String& qualifiedName,
	const rtl::String& tag,
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
	const rtl::String& name,
	const rtl::String& qualifiedName,
	const rtl::String& tag
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

#pragma AXL_TODO ("get rid of emission context stack: postpone compilation of nested function")

void
FunctionMgr::pushEmissionContext ()
{
	if (!m_currentFunction)
		return;

	EmissionContext* context = AXL_MEM_NEW (EmissionContext);
	context->m_currentFunction = m_currentFunction;
	context->m_thisValue = m_thisValue;
	context->m_scopeLevelValue = m_scopeLevelValue;
	context->m_tmpStackGcRootList.takeOver (&m_module->m_operatorMgr.m_tmpStackGcRootList);

	context->m_currentNamespace = m_module->m_namespaceMgr.m_currentNamespace;
	context->m_currentScope = m_module->m_namespaceMgr.m_currentScope;
	context->m_scopeLevelStack.takeOver (&m_module->m_namespaceMgr.m_scopeLevelStack);

	context->m_returnBlockArray = m_module->m_controlFlowMgr.m_returnBlockArray;
	context->m_currentBlock = m_module->m_controlFlowMgr.m_currentBlock;
	context->m_unreachableBlock = m_module->m_controlFlowMgr.m_unreachableBlock;
	context->m_controlFlowMgrFlags = m_module->m_controlFlowMgr.m_flags;
	context->m_llvmDebugLoc = m_module->m_llvmIrBuilder.getCurrentDebugLoc ();

	m_emissionContextStack.insertTail (context);

	m_module->m_namespaceMgr.m_scopeLevelStack.clear ();
	m_module->m_namespaceMgr.m_currentNamespace = &m_module->m_namespaceMgr.m_globalNamespace;
	m_module->m_namespaceMgr.m_currentScope = NULL;

	m_module->m_controlFlowMgr.m_returnBlockArray.clear ();
	m_module->m_controlFlowMgr.setCurrentBlock (NULL);
	m_module->m_controlFlowMgr.m_unreachableBlock = NULL;
	m_module->m_controlFlowMgr.m_flags = 0;
	m_module->m_llvmIrBuilder.setCurrentDebugLoc (llvm::DebugLoc ());
	// m_pModule->m_LlvmIrBuilder.SetCurrentDebugLoc (m_pModule->m_LlvmDiBuilder.GetEmptyDebugLoc ());

	m_module->m_variableMgr.deallocateTlsVariableArray (m_currentFunction->m_tlsVariableArray);

	m_currentFunction = NULL;
	m_thisValue.clear ();
	m_scopeLevelValue.clear ();
}

void
FunctionMgr::popEmissionContext ()
{
	ASSERT (m_currentFunction);
	m_module->m_variableMgr.deallocateTlsVariableArray (m_currentFunction->m_tlsVariableArray);

	if (m_emissionContextStack.isEmpty ())
	{
		m_currentFunction = NULL;
		m_thisValue.clear ();
		m_scopeLevelValue.clear ();
		m_module->m_namespaceMgr.m_scopeLevelStack.clear ();
		m_module->m_operatorMgr.m_tmpStackGcRootList.clear ();
		return;
	}

	EmissionContext* context = m_emissionContextStack.removeTail ();
	m_currentFunction = context->m_currentFunction;
	m_thisValue = context->m_thisValue;
	m_scopeLevelValue = context->m_scopeLevelValue;
	m_module->m_operatorMgr.m_tmpStackGcRootList.takeOver (&context->m_tmpStackGcRootList);

	m_module->m_namespaceMgr.m_currentNamespace = context->m_currentNamespace;
	m_module->m_namespaceMgr.m_currentScope = context->m_currentScope;
	m_module->m_namespaceMgr.m_scopeLevelStack.takeOver (&context->m_scopeLevelStack);

	m_module->m_controlFlowMgr.m_returnBlockArray = context->m_returnBlockArray;
	m_module->m_controlFlowMgr.setCurrentBlock (context->m_currentBlock);
	m_module->m_controlFlowMgr.m_unreachableBlock = context->m_unreachableBlock;
	m_module->m_controlFlowMgr.m_flags = context->m_controlFlowMgrFlags;
	m_module->m_llvmIrBuilder.setCurrentDebugLoc (context->m_llvmDebugLoc);

	AXL_MEM_DELETE (context);

	m_module->m_variableMgr.restoreTlsVariableArray (m_currentFunction->m_tlsVariableArray);
}

bool
FunctionMgr::fireOnChanged ()
{
	Function* function = m_currentFunction;

	ASSERT (
		function->m_functionKind == FunctionKind_Setter &&
		function->m_property &&
		function->m_property->getType ()->getFlags () & PropertyTypeFlagKind_Bindable
		);

	Value propertyValue = function->m_property;

	if (function->m_thisType)
	{
		ASSERT (m_thisValue);
		propertyValue.insertToClosureHead (m_thisValue);
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

	pushEmissionContext ();

	m_currentFunction = function;

	// create scope

	m_module->m_namespaceMgr.openNamespace (function->m_parentNamespace);

	function->m_scope = m_module->m_namespaceMgr.openScope (pos);

	// create entry block (gc roots come here)

	BasicBlock* entryBlock = m_module->m_controlFlowMgr.createBlock ("function_entry");
	BasicBlock* bodyBlock = m_module->m_controlFlowMgr.createBlock ("function_body");

	function->m_entryBlock = entryBlock;
	entryBlock->markEntry ();

	m_module->m_controlFlowMgr.setCurrentBlock (entryBlock);

	if (function->m_functionKind == FunctionKind_ModuleConstructor)
	{
		bool result = m_module->m_variableMgr.allocatePrimeStaticVariables ();
		if (!result)
			return false;
	}
	else // do not save / restore scope level in module constructor
	{
		Variable* variable = m_module->m_variableMgr.getStdVariable (StdVariableKind_ScopeLevel);
		m_module->m_llvmIrBuilder.createLoad (variable, NULL, &m_scopeLevelValue);
	}

	m_module->m_controlFlowMgr.jump (bodyBlock, bodyBlock);
	m_module->m_controlFlowMgr.m_unreachableBlock = NULL;
	m_module->m_controlFlowMgr.m_flags = 0; // clear jump flag

	// save scope level

	if (function->m_functionKind == FunctionKind_ModuleConstructor)
	{
		result = m_module->m_variableMgr.initializeGlobalStaticVariables ();
		if (!result)
			return false;
	}

	function->getType ()->getCallConv ()->createArgVariables (function);

	// 'this' arg

	if (function->isMember ())
		createThisValue ();

	// static initializers

	if (function->m_functionKind == FunctionKind_StaticConstructor)
	{
		DerivableType* type = function->getParentType ();
		type->initializeStaticFields ();
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

			Value ptrValue;
			m_module->m_llvmIrBuilder.createExtractValue (thisArgValue, 0, NULL, &ptrValue);

			ptrType = ptrType->getTargetType ()->getDataPtrType (DataPtrTypeKind_Lean, ptrType->getFlags ());
			m_thisValue.setLeanDataPtr (ptrValue.getLlvmValue (), ptrType, thisArgValue);
		}
	}
	else
	{
		ASSERT (function->m_storageKind == StorageKind_Override && function->m_thisArgDelta < 0);

		Value ptrValue;
		m_module->m_llvmIrBuilder.createBitCast (thisArgValue, m_module->getSimpleType (StdTypeKind_BytePtr), &ptrValue);
		m_module->m_llvmIrBuilder.createGep (ptrValue, (int32_t) function->m_thisArgDelta, NULL, &ptrValue);
		m_module->m_llvmIrBuilder.createBitCast (ptrValue, function->m_thisType, &m_thisValue);
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
		ASSERT (function->getParentType ()->getTypeKind () == TypeKind_Class && m_thisValue);

		ClassType* classType = (ClassType*) function->getParentType ();

		result =
			classType->callMemberPropertyDestructors (m_thisValue) &&
			classType->callMemberFieldDestructors (m_thisValue) &&
			classType->callBaseTypeDestructors (m_thisValue);

		if (!result)
			return false;
	}

	if (function->m_functionKind == FunctionKind_ModuleDestructor)
		m_module->m_variableMgr.m_staticDestructList.runDestructors ();

	result = m_module->m_controlFlowMgr.checkReturn ();
	if (!result)
		return false;

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

	m_module->m_namespaceMgr.closeScope ();
	m_module->m_namespaceMgr.closeNamespace ();

	popEmissionContext ();
	return true;
}

void
FunctionMgr::internalPrologue (
	Function* function,
	Value* argValueArray,
	size_t argCount
	)
{
	pushEmissionContext ();

	m_currentFunction = function;

	m_module->m_namespaceMgr.openInternalScope ();
	m_module->m_llvmIrBuilder.setCurrentDebugLoc (llvm::DebugLoc ());

	BasicBlock* entryBlock = m_module->m_controlFlowMgr.createBlock ("function_entry");
	BasicBlock* bodyBlock = m_module->m_controlFlowMgr.createBlock ("function_body");

	function->m_entryBlock = entryBlock;
	entryBlock->markEntry ();

	m_module->m_controlFlowMgr.setCurrentBlock (entryBlock);

	if (function->m_functionKind != FunctionKind_ModuleConstructor) // do not save / restore scope level in module constructor
	{
		Variable* variable = m_module->m_variableMgr.getStdVariable (StdVariableKind_ScopeLevel);
		m_module->m_llvmIrBuilder.createLoad (variable, NULL, &m_scopeLevelValue);
	}

	m_module->m_controlFlowMgr.jump (bodyBlock, bodyBlock);
	m_module->m_controlFlowMgr.m_unreachableBlock = NULL;
	m_module->m_controlFlowMgr.m_flags = 0;

	if (function->isMember ())
		createThisValue ();

	if (argCount)
	{
		llvm::Function::arg_iterator llvmArg = function->getLlvmFunction ()->arg_begin ();
		rtl::Array <FunctionArg*> argArray = function->getType ()->getArgArray ();

		CallConv* callConv = function->getType ()->getCallConv ();

		for (size_t i = 0; i < argCount; i++, llvmArg++)
			argValueArray [i] = callConv->getArgValue (argArray [i], llvmArg);
	}
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

	m_module->m_namespaceMgr.closeScope ();

	popEmissionContext ();
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

	rtl::String signature;
	signature.format (
		"%c%x.%s",
		signatureChar,
		targetFunction,
		thunkFunctionType->getSignature ().cc ()
		);

	rtl::StringHashTableMapIterator <Function*> thunk = m_thunkFunctionMap.visit (signature);
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

	rtl::String signature;
	signature.format (
		"%c%x.%s",
		hasUnusedClosure ? 'U' : 'D',
		targetProperty,
		thunkPropertyType->getSignature ().cc ()
		);

	rtl::StringHashTableMapIterator <Property*> thunk = m_thunkPropertyMap.visit (signature);
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

	rtl::String signature;
	signature.format (
		"%c%x.%s",
		hasUnusedClosure ? 'U' : 'D',
		targetVariable,
		thunkPropertyType->getSignature ().cc ()
		);

	rtl::StringHashTableMapIterator <Property*> thunk = m_thunkPropertyMap.visit (signature);
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
	rtl::String signature = targetFunctionPtrType->getSignature ();
	if (schedulerPtrTypeKind == ClassPtrTypeKind_Weak)
		signature += ".w";

	rtl::StringHashTableMapIterator <Function*> thunk = m_scheduleLauncherFunctionMap.visit (signature);
	if (thunk->m_value)
		return thunk->m_value;

	ClassPtrType* schedulerPtrType = ((ClassType*) m_module->getSimpleType (StdTypeKind_Scheduler))->getClassPtrType (schedulerPtrTypeKind);

	rtl::Array <FunctionArg*> argArray  = targetFunctionPtrType->getTargetType ()->getArgArray ();
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
	rtl::Iterator <Function> functionIt = m_functionList.getHead ();
	for (; functionIt; functionIt++)
	{
		Function* function = *functionIt;
		if (function->getEntryBlock () && !function->getTlsVariableArray ().isEmpty ())
			injectTlsPrologue (function);
	}

	rtl::Iterator <ThunkFunction> thunkFunctionIt = m_thunkFunctionList.getHead ();
	for (; thunkFunctionIt; thunkFunctionIt++)
	{
		Function* function = *thunkFunctionIt;
		if (!function->getTlsVariableArray ().isEmpty ())
			injectTlsPrologue (function);
	}

	rtl::Iterator <ScheduleLauncherFunction> scheduleLauncherFunctionIt = m_scheduleLauncherFunctionList.getHead ();
	for (; scheduleLauncherFunctionIt; scheduleLauncherFunctionIt++)
	{
		Function* function = *scheduleLauncherFunctionIt;
		if (!function->getTlsVariableArray ().isEmpty ())
			injectTlsPrologue (function);
	}

	return true;
}

void
FunctionMgr::injectTlsPrologue (Function* function)
{
	BasicBlock* block = function->getEntryBlock ();
	ASSERT (block);

	rtl::Array <TlsVariable> tlsVariableArray = function->getTlsVariableArray ();
	ASSERT (!tlsVariableArray.isEmpty ());

	m_module->m_controlFlowMgr.setCurrentBlock (block);

	llvm::BasicBlock::iterator llvmAnchor = block->getLlvmBlock ()->begin ();

	if (llvm::isa <llvm::CallInst> (llvmAnchor)) // skip gc-enter
		llvmAnchor++;

	m_module->m_llvmIrBuilder.setInsertPoint (llvmAnchor);

	Function* getTls = getStdFunction (StdFuncKind_GetTls);
	Value tlsValue;
	llvmAnchor = m_module->m_llvmIrBuilder.createCall (getTls, getTls->getType (), &tlsValue);

	// tls variables used in this function

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

	// skip all the gep's to get past tls prologue

	llvmAnchor++;
	while (llvm::isa <llvm::GetElementPtrInst> (llvmAnchor))
		llvmAnchor++;

	function->m_llvmPostTlsPrologueInst = llvmAnchor;
}

class JitEventListener: public llvm::JITEventListener
{
protected:
	FunctionMgr* m_functionMgr;

public:
	JitEventListener (FunctionMgr* functionMgr)
	{
		m_functionMgr = functionMgr;
	}

	virtual
	void
	notifyObjectEmitted (const llvm::ObjectImage& LLvmObjectImage)
	{
		// printf ("NotifyObjectEmitted\n");
	}

	virtual
	void
	notifyFunctionEmitted (
		const llvm::Function& llvmFunction,
		void* p,
		size_t size,
		const EmittedFunctionDetails& details
		)
	{
		Function* function = m_functionMgr->findFunctionByLlvmFunction ((llvm::Function*) &llvmFunction);
		if (function)
		{
			function->m_pfMachineCode = p;
			function->m_machineCodeSize = size;
		}
	}
};

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

	ScopeThreadModule scopeModule (m_module);
	llvm::ScopedFatalErrorHandler scopeErrorHandler (llvmFatalErrorHandler);

	JitEventListener jitEventListener (this);
	llvm::ExecutionEngine* llvmExecutionEngine = m_module->getLlvmExecutionEngine ();
	llvmExecutionEngine->RegisterJITEventListener (&jitEventListener);

	try
	{
		rtl::Iterator <Function> functionIt = m_functionList.getHead ();
		for (; functionIt; functionIt++)
		{
			Function* function = *functionIt;

			if (!function->getEntryBlock ())
				continue;

			void* pf = llvmExecutionEngine->getPointerToFunction (function->getLlvmFunction ());
			function->m_pfMachineCode = pf;

			// ASSERT (pFunction->m_pfMachineCode == pf && pFunction->m_MachineCodeSize != 0);
		}

		// for MC jitter this should do all the job
		llvmExecutionEngine->finalizeObject ();
	}
	catch (err::Error error)
	{
		err::setFormatStringError ("LLVM jitting failed: %s", error->getDescription ().cc ());
		llvmExecutionEngine->UnregisterJITEventListener (&jitEventListener);
		return false;
	}

	llvmExecutionEngine->UnregisterJITEventListener (&jitEventListener);
	return true;
}

Function*
FunctionMgr::getStdFunction (StdFuncKind func)
{
	ASSERT ((size_t) func < StdFuncKind__Count);

	if (m_stdFunctionArray [func])
		return m_stdFunctionArray [func];

	Type* argTypeArray [8] = { 0 }; // 8 is enough for all the std functions

	Type* returnType;
	FunctionType* functionType;
	Function* function;

	switch (func)
	{
	case StdFuncKind_RuntimeError:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
		argTypeArray [0] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int);
		argTypeArray [1] = m_module->m_typeMgr.getStdType (StdTypeKind_BytePtr);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 2);
		function = createFunction (FunctionKind_Internal, "jnc.runtimeError", functionType);
		break;

	case StdFuncKind_CheckNullPtr:
		function = createCheckNullPtr ();
		break;

	case StdFuncKind_CheckScopeLevel:
		function = createCheckScopeLevel ();
		break;

	case StdFuncKind_CheckClassPtrScopeLevel:
		function = createCheckClassPtrScopeLevel ();
		break;

	case StdFuncKind_CheckDataPtrRange:
		function = createCheckDataPtrRange ();
		break;

	case StdFuncKind_DynamicCastClassPtr:
		returnType = m_module->m_typeMgr.getStdType (StdTypeKind_ObjectPtr);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdTypeKind_ObjectPtr);
		argTypeArray [1] = m_module->m_typeMgr.getStdType (StdTypeKind_BytePtr);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 2);
		function = createFunction (FunctionKind_Internal, "jnc.dynamicCastClassPtr", functionType);
		break;

	case StdFuncKind_StrengthenClassPtr:
		returnType = m_module->m_typeMgr.getStdType (StdTypeKind_ObjectPtr);
		argTypeArray [0] = ((ClassType*) m_module->m_typeMgr.getStdType (StdTypeKind_ObjectClass))->getClassPtrType (ClassPtrTypeKind_Weak);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 1);
		function = createFunction (FunctionKind_Internal, "jnc.strengthenClassPtr", functionType);
		break;

	case StdFuncKind_GetDataPtrSpan:
		function = createGetDataPtrSpan ();
		break;

	case StdFuncKind_GcAllocate:
		returnType = m_module->m_typeMgr.getStdType (StdTypeKind_BytePtr);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdTypeKind_BytePtr);
		argTypeArray [1] = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 2);
		function = createFunction (FunctionKind_Internal, "jnc.gcAllocate", functionType);
		break;

	case StdFuncKind_GcTryAllocate:
		returnType = m_module->m_typeMgr.getStdType (StdTypeKind_BytePtr);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdTypeKind_BytePtr);
		argTypeArray [1] = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 2);
		function = createFunction (FunctionKind_Internal, "jnc.gcTryAllocate", functionType);
		break;

	case StdFuncKind_MarkGcRoot:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdTypeKind_BytePtr)->getDataPtrType_c ();
		argTypeArray [1] = m_module->m_typeMgr.getStdType (StdTypeKind_BytePtr);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 2);
		function = createFunction (FunctionKind_Internal, "jnc.markGcRoot", functionType);
		function->m_llvmFunction = llvm::Intrinsic::getDeclaration (m_module->getLlvmModule (), llvm::Intrinsic::gcroot);
		break;

	case StdFuncKind_GetTls:
		returnType = m_module->m_variableMgr.getTlsStructType ()->getDataPtrType (DataPtrTypeKind_Thin);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, NULL, 0);
		function = createFunction (FunctionKind_Internal, "jnc.getTls", functionType);
		break;

	case StdFuncKind_GcEnter:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, NULL, 0);
		function = createFunction (FunctionKind_Internal, "jnc.gcEnter", functionType);
		break;

	case StdFuncKind_GcLeave:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, NULL, 0);
		function = createFunction (FunctionKind_Internal, "jnc.gcLeave", functionType);
		break;

	case StdFuncKind_GcPulse:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, NULL, 0);
		function = createFunction (FunctionKind_Internal, "jnc.gcPulse", functionType);
		break;

	case StdFuncKind_RunGc:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, NULL, 0);
		function = createFunction ("runGc", "jnc.runGc", functionType);
		break;

	case StdFuncKind_CreateThread:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int64_u);
		argTypeArray [0] = ((FunctionType*) m_module->m_typeMgr.getStdType (StdTypeKind_SimpleFunction))->getFunctionPtrType (FunctionPtrTypeKind_Normal, PtrTypeFlagKind_Safe);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 1);
		function = createFunction ("createThread", "jnc.createThread", functionType);
		break;

	case StdFuncKind_Sleep:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
		argTypeArray [0] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32_u);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 1);
		function = createFunction ("sleep", "jnc.sleep", functionType);
		break;

	case StdFuncKind_GetTimestamp:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int64_u);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdTypeKind_ObjectPtr);
		argTypeArray [1] = m_module->m_typeMgr.getStdType (StdTypeKind_BytePtr);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, NULL, 0);
		function = createFunction ("getTimestamp", "jnc.getTimestamp", functionType);
		break;

	case StdFuncKind_GetCurrentThreadId:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int64_u);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, NULL, 0);
		function = createFunction ("getCurrentThreadId", "jnc.getCurrentThreadId", functionType);
		break;

	case StdFuncKind_GetLastError:
		returnType = m_module->m_typeMgr.getStdType (StdTypeKind_Error)->getDataPtrType (DataPtrTypeKind_Normal, PtrTypeFlagKind_Const);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, NULL, 0);
		function = createFunction ("getLastError", "jnc.getLastError", functionType);
		break;

	case StdFuncKind_StrLen:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);
		argTypeArray [0] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Char)->getDataPtrType (DataPtrTypeKind_Normal, PtrTypeFlagKind_Const);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 1);
		function = createFunction ("strlen", "strlen", functionType);
		break;

	case StdFuncKind_MemCpy:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
		argTypeArray [0] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void)->getDataPtrType (DataPtrTypeKind_Normal);
		argTypeArray [1] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void)->getDataPtrType (DataPtrTypeKind_Normal, PtrTypeFlagKind_Const);
		argTypeArray [2] = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 3);
		function = createFunction ("memcpy", "memcpy", functionType);
		break;

	case StdFuncKind_MemCat:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void)->getDataPtrType (DataPtrTypeKind_Normal);
		argTypeArray [0] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void)->getDataPtrType (DataPtrTypeKind_Normal, PtrTypeFlagKind_Const);
		argTypeArray [1] = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);
		argTypeArray [2] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void)->getDataPtrType (DataPtrTypeKind_Normal, PtrTypeFlagKind_Const);
		argTypeArray [3] = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 4);
		function = createFunction ("memcat", "memcat", functionType);
		break;

	case StdFuncKind_Rand:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, NULL, 0);
		function = createFunction ("rand", "rand", functionType);
		break;

	case StdFuncKind_Printf:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int_p);
		argTypeArray [0] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Char)->getDataPtrType (TypeKind_DataPtr, DataPtrTypeKind_Thin, PtrTypeFlagKind_Const);
		functionType = m_module->m_typeMgr.getFunctionType (
			m_module->m_typeMgr.getCallConv (CallConvKind_Cdecl),
			returnType,
			argTypeArray, 1,
			FunctionTypeFlagKind_VarArg
			);
		function = createFunction ("printf", "printf", functionType);
		break;

	case StdFuncKind_Atoi:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int);
		argTypeArray [0] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Char)->getDataPtrType (TypeKind_DataPtr, DataPtrTypeKind_Normal, PtrTypeFlagKind_Const);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 1);
		function = createFunction ("atoi", "atoi", functionType);
		break;

	case StdFuncKind_Format:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Char)->getDataPtrType (TypeKind_DataPtr, DataPtrTypeKind_Normal, PtrTypeFlagKind_Const);
		argTypeArray [0] = returnType;
		functionType = m_module->m_typeMgr.getFunctionType (
			m_module->m_typeMgr.getCallConv (CallConvKind_Cdecl),
			returnType,
			argTypeArray, 1,
			FunctionTypeFlagKind_VarArg
			);
		function = createFunction ("format", "jnc.format", functionType);
		break;

	case StdFuncKind_AppendFmtLiteral_a:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdTypeKind_FmtLiteral)->getDataPtrType_c (),
		argTypeArray [1] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Char)->getDataPtrType_c (TypeKind_DataPtr, PtrTypeFlagKind_Const),
		argTypeArray [2] = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT),
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 3);
		function = createFunction (FunctionKind_Internal, "jnc.appendFmtLiteral_a", functionType);
		break;

	case StdFuncKind_AppendFmtLiteral_p:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdTypeKind_FmtLiteral)->getDataPtrType_c (),
		argTypeArray [1] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Char)->getDataPtrType_c (TypeKind_DataPtr, PtrTypeFlagKind_Const),
		argTypeArray [2] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Char)->getDataPtrType (DataPtrTypeKind_Normal, PtrTypeFlagKind_Const),
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 3);
		function = createFunction (FunctionKind_Internal, "jnc.appendFmtLiteral_p", functionType);
		break;

	case StdFuncKind_AppendFmtLiteral_i32:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdTypeKind_FmtLiteral)->getDataPtrType_c (),
		argTypeArray [1] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Char)->getDataPtrType_c (TypeKind_DataPtr, PtrTypeFlagKind_Const),
		argTypeArray [2] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32),
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 3);
		function = createFunction (FunctionKind_Internal, "jnc.appendFmtLiteral_i32", functionType);
		break;

	case StdFuncKind_AppendFmtLiteral_ui32:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdTypeKind_FmtLiteral)->getDataPtrType_c (),
		argTypeArray [1] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Char)->getDataPtrType_c (TypeKind_DataPtr, PtrTypeFlagKind_Const),
		argTypeArray [2] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32_u),
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 3);
		function = createFunction (FunctionKind_Internal, "jnc.appendFmtLiteral_ui32", functionType);
		break;

	case StdFuncKind_AppendFmtLiteral_i64:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdTypeKind_FmtLiteral)->getDataPtrType_c (),
		argTypeArray [1] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Char)->getDataPtrType_c (TypeKind_DataPtr, PtrTypeFlagKind_Const),
		argTypeArray [2] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int64),
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 3);
		function = createFunction (FunctionKind_Internal, "jnc.appendFmtLiteral_i64", functionType);
		break;

	case StdFuncKind_AppendFmtLiteral_ui64:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdTypeKind_FmtLiteral)->getDataPtrType_c (),
		argTypeArray [1] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Char)->getDataPtrType_c (TypeKind_DataPtr, PtrTypeFlagKind_Const),
		argTypeArray [2] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int64_u),
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 3);
		function = createFunction (FunctionKind_Internal, "jnc.appendFmtLiteral_ui64", functionType);
		break;

	case StdFuncKind_AppendFmtLiteral_f:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdTypeKind_FmtLiteral)->getDataPtrType_c (),
		argTypeArray [1] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Char)->getDataPtrType_c (TypeKind_DataPtr, PtrTypeFlagKind_Const),
		argTypeArray [2] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Double),
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 3);
		function = createFunction (FunctionKind_Internal, "jnc.appendFmtLiteral_f", functionType);
		break;

	case StdFuncKind_SimpleMulticastCall:
		function = ((MulticastClassType*) m_module->m_typeMgr.getStdType (StdTypeKind_SimpleMulticast))->getMethod (MulticastMethodKind_Call);
		break;

	default:
		ASSERT (false);
		function = NULL;
	}

	m_stdFunctionArray [func] = function;
	return function;
}

LazyStdFunction*
FunctionMgr::getLazyStdFunction (StdFuncKind func)
{
	ASSERT ((size_t) func < StdFuncKind__Count);

	if (m_lazyStdFunctionArray [func])
		return m_lazyStdFunctionArray [func];

	const char* nameTable [StdFuncKind__Count] =
	{
		NULL, // EStdFunc_RuntimeError,
		NULL, // EStdFunc_CheckNullPtr,
		NULL, // EStdFunc_CheckScopeLevel,
		NULL, // EStdFunc_CheckClassPtrScopeLevel,
		NULL, // EStdFunc_CheckDataPtrRange,
		NULL, // EStdFunc_DynamicCastClassPtr,
		NULL, // EStdFunc_StrengthenClassPtr,
		"getDataPtrSpan",   // EStdFunc_GetDataPtrSpan,
		NULL, // EStdFunc_GcAllocate,
		NULL, // EStdFunc_GcTryAllocate,
		NULL, // EStdFunc_GcEnter,
		NULL, // EStdFunc_GcLeave,
		NULL, // EStdFunc_GcPulse,
		NULL, // EStdFunc_MarkGcRoot,
		"runGc",              // EStdFunc_RunGc,
		"getCurrentThreadId", // EStdFunc_GetCurrentThreadId,
		"createThread",       // EStdFunc_CreateThread,
		"sleep",              // EStdFunc_Sleep,
		"getTimestamp",       // EStdFunc_GetTimestamp,
		"format",             // EStdFunc_Format,
		"strlen",             // EStdFunc_StrLen,
		"memcpy",             // EStdFunc_MemCpy,
		"memcat",             // EStdFunc_MemCat,
		"rand",               // EStdFunc_Rand,
		"printf",             // EStdFunc_Printf,
		"atoi",               // EStdFunc_Atoi,
		NULL, // EStdFunc_GetTls,
		NULL, // EStdFunc_AppendFmtLiteral_a,
		NULL, // EStdFunc_AppendFmtLiteral_p,
		NULL, // EStdFunc_AppendFmtLiteral_i32,
		NULL, // EStdFunc_AppendFmtLiteral_ui32,
		NULL, // EStdFunc_AppendFmtLiteral_i64,
		NULL, // EStdFunc_AppendFmtLiteral_ui64,
		NULL, // EStdFunc_AppendFmtLiteral_f,
		NULL, // EStdFunc_SimpleMulticastCall,
		"getLastError",       // EStdFunc_GetLastError,
	};

	const char* name = nameTable [func];
	ASSERT (name);

	LazyStdFunction* function = AXL_MEM_NEW (LazyStdFunction);
	function->m_module = m_module;
	function->m_name = name;
	function->m_func = func;
	m_lazyStdFunctionList.insertTail (function);
	m_lazyStdFunctionArray [func] = function;
	return function;
}

Function*
FunctionMgr::createCheckNullPtr ()
{
	Type* returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
	Type* argTypeArray [] =
	{
		m_module->m_typeMgr.getStdType (StdTypeKind_BytePtr),
		m_module->m_typeMgr.getPrimitiveType (TypeKind_Int),
	};

	FunctionType* functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, countof (argTypeArray));
	Function* function = createFunction (FunctionKind_Internal, "jnc.checkNullPtr", functionType);

	Value argValueArray [2];
	internalPrologue (function, argValueArray, countof (argValueArray));

	Value argValue1 = argValueArray [0];
	Value argValue2 = argValueArray [1];

	BasicBlock* failBlock = m_module->m_controlFlowMgr.createBlock ("iface_fail");
	BasicBlock* successBlock = m_module->m_controlFlowMgr.createBlock ("iface_success");

	Value nullValue = m_module->m_typeMgr.getStdType (StdTypeKind_BytePtr)->getZeroValue ();

	Value cmpValue;
	m_module->m_llvmIrBuilder.createEq_i (argValue1, nullValue, &cmpValue);
	m_module->m_controlFlowMgr.conditionalJump (cmpValue, failBlock, successBlock);

	m_module->m_llvmIrBuilder.runtimeError (argValue2);

	m_module->m_controlFlowMgr.follow (successBlock);

	internalEpilogue ();

	return function;
}

Function*
FunctionMgr::createCheckScopeLevel ()
{
	Type* returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
	Type* argTypeArray [] =
	{
		m_module->m_typeMgr.getStdType (StdTypeKind_ObjHdrPtr),
		m_module->m_typeMgr.getStdType (StdTypeKind_ObjHdrPtr),
	};

	FunctionType* functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, countof (argTypeArray));
	Function* function = createFunction (FunctionKind_Internal, "jnc.checkScopeLevel", functionType);

	Value argValueArray [2];
	internalPrologue (function, argValueArray, countof (argValueArray));

	Value argValue1 = argValueArray [0];
	Value argValue2 = argValueArray [1];

	BasicBlock* noNullBlock = m_module->m_controlFlowMgr.createBlock ("scope_nonull");
	BasicBlock* failBlock = m_module->m_controlFlowMgr.createBlock ("scope_fail");
	BasicBlock* successBlock = m_module->m_controlFlowMgr.createBlock ("scope_success");

	Value cmpValue;
	Value nullValue = m_module->m_typeMgr.getStdType (StdTypeKind_ObjHdrPtr)->getZeroValue ();

	m_module->m_llvmIrBuilder.createEq_i (argValue1, nullValue, &cmpValue);
	m_module->m_controlFlowMgr.conditionalJump (cmpValue, successBlock, noNullBlock, noNullBlock);

	m_module->m_llvmIrBuilder.createGep2 (argValue1, 0, NULL, &argValue1);
	m_module->m_llvmIrBuilder.createLoad (argValue1, NULL, &argValue1);
	m_module->m_llvmIrBuilder.createGep2 (argValue2, 0, NULL, &argValue2);
	m_module->m_llvmIrBuilder.createLoad (argValue2, NULL, &argValue2);
	m_module->m_llvmIrBuilder.createGt_u (argValue1, argValue2, &cmpValue);
	m_module->m_controlFlowMgr.conditionalJump (cmpValue, failBlock, successBlock);
	m_module->m_llvmIrBuilder.runtimeError (RuntimeErrorKind_ScopeMismatch);

	m_module->m_controlFlowMgr.follow (successBlock);

	internalEpilogue ();

	return function;
}

Function*
FunctionMgr::createCheckClassPtrScopeLevel ()
{
	Type* returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
	Type* argTypeArray [] =
	{
		m_module->m_typeMgr.getStdType (StdTypeKind_ObjectPtr),
		m_module->m_typeMgr.getStdType (StdTypeKind_ObjHdrPtr),
	};

	FunctionType* functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, countof (argTypeArray));
	Function* function = createFunction (FunctionKind_Internal, "jnc.checkClassPtrScopeLevel", functionType);

	Value argValueArray [2];
	internalPrologue (function, argValueArray, countof (argValueArray));

	Value argValue1 = argValueArray [0];
	Value argValue2 = argValueArray [1];

	BasicBlock* noNullBlock = m_module->m_controlFlowMgr.createBlock ("scope_nonull");
	BasicBlock* failBlock = m_module->m_controlFlowMgr.createBlock ("scope_fail");
	BasicBlock* successBlock = m_module->m_controlFlowMgr.createBlock ("scope_success");

	Value cmpValue;
	Value nullValue = m_module->m_typeMgr.getStdType (StdTypeKind_ObjectPtr)->getZeroValue ();

	m_module->m_llvmIrBuilder.createEq_i (argValue1, nullValue, &cmpValue);
	m_module->m_controlFlowMgr.conditionalJump (cmpValue, successBlock, noNullBlock, noNullBlock);

	static int32_t llvmIndexArray [] =
	{
		0, // TIfaceHdr**
		0, // TIfaceHdr*
		1, // TObjHdr**
	};

	Value objPtrValue;
	m_module->m_llvmIrBuilder.createGep (argValue1, llvmIndexArray, countof (llvmIndexArray), NULL, &objPtrValue); // TObjHdr** ppObject
	m_module->m_llvmIrBuilder.createLoad (objPtrValue, NULL, &objPtrValue);  // TObjHdr* pObject

	Value srcScopeLevelValue;
	m_module->m_llvmIrBuilder.createGep2 (objPtrValue, 0, NULL, &srcScopeLevelValue);     // size_t* pScopeLevel
	m_module->m_llvmIrBuilder.createLoad (srcScopeLevelValue, NULL, &srcScopeLevelValue); // size_t ScopeLevel

	m_module->m_llvmIrBuilder.createGep2 (argValue2, 0, NULL, &argValue2);
	m_module->m_llvmIrBuilder.createLoad (argValue2, NULL, &argValue2);

	m_module->m_llvmIrBuilder.createGt_u (srcScopeLevelValue, argValue2, &cmpValue); // SrcScopeLevel > DstScopeLevel
	m_module->m_controlFlowMgr.conditionalJump (cmpValue, failBlock, successBlock);
	m_module->m_llvmIrBuilder.runtimeError (RuntimeErrorKind_ScopeMismatch);

	m_module->m_controlFlowMgr.follow (successBlock);

	internalEpilogue ();

	return function;
}

Function*
FunctionMgr::createCheckDataPtrRange ()
{
	Type* returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
	Type* argTypeArray [] =
	{
		m_module->m_typeMgr.getStdType (StdTypeKind_BytePtr),
		m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT),
		m_module->m_typeMgr.getStdType (StdTypeKind_BytePtr),
		m_module->m_typeMgr.getStdType (StdTypeKind_BytePtr),
	};

	FunctionType* functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, countof (argTypeArray));
	Function* function = createFunction (FunctionKind_Internal, "jnc.checkDataPtrRange", functionType);

	Value argValueArray [4];
	internalPrologue (function, argValueArray, countof (argValueArray));

	Value argValue1 = argValueArray [0];
	Value argValue2 = argValueArray [1];
	Value argValue3 = argValueArray [2];
	Value argValue4 = argValueArray [3];

	BasicBlock* failBlock = m_module->m_controlFlowMgr.createBlock ("sptr_fail");
	BasicBlock* successBlock = m_module->m_controlFlowMgr.createBlock ("sptr_success");
	BasicBlock* cmp2Block = m_module->m_controlFlowMgr.createBlock ("sptr_cmp2");
	BasicBlock* cmp3Block = m_module->m_controlFlowMgr.createBlock ("sptr_cmp3");

	Value nullValue = m_module->m_typeMgr.getStdType (StdTypeKind_BytePtr)->getZeroValue ();

	Value cmpValue;
	m_module->m_llvmIrBuilder.createEq_i (argValue1, nullValue, &cmpValue);
	m_module->m_controlFlowMgr.conditionalJump (cmpValue, failBlock, cmp2Block, cmp2Block);

	m_module->m_llvmIrBuilder.createLt_u (argValue1, argValue3, &cmpValue);
	m_module->m_controlFlowMgr.conditionalJump (cmpValue, failBlock, cmp3Block, cmp3Block);

	Value ptrEndValue;
	m_module->m_llvmIrBuilder.createGep (argValue1, argValue2, NULL ,&ptrEndValue);
	m_module->m_llvmIrBuilder.createGt_u (ptrEndValue, argValue4, &cmpValue);
	m_module->m_controlFlowMgr.conditionalJump (cmpValue, failBlock, successBlock);

	m_module->m_llvmIrBuilder.runtimeError (RuntimeErrorKind_DataPtrOutOfRange);

	m_module->m_controlFlowMgr.follow (successBlock);

	internalEpilogue ();

	return function;
}

// size_t
// jnc.GetDataPtrSpan (jnc.DataPtr Ptr);

Function*
FunctionMgr::createGetDataPtrSpan ()
{
	Type* intPtrType = m_module->getSimpleType (TypeKind_Int_p);
	Type* argTypeArray [] =
	{
		m_module->m_typeMgr.getPrimitiveType (TypeKind_Void)->getDataPtrType (DataPtrTypeKind_Normal, PtrTypeFlagKind_Const),
	};

	FunctionType* functionType = m_module->m_typeMgr.getFunctionType (intPtrType, argTypeArray, countof (argTypeArray));
	Function* function = createFunction ("getDataPtrSpan", "jnc.getDataPtrSpan", functionType);

	Value argValue;
	internalPrologue (function, &argValue, 1);

	Value ptrValue;
	Value rangeEndValue;
	Value spanValue;
	m_module->m_llvmIrBuilder.createExtractValue (argValue, 0, NULL, &ptrValue);
	m_module->m_llvmIrBuilder.createExtractValue (argValue, 2, NULL, &rangeEndValue);
	m_module->m_llvmIrBuilder.createPtrToInt (ptrValue, intPtrType, &ptrValue);
	m_module->m_llvmIrBuilder.createPtrToInt (rangeEndValue, intPtrType, &rangeEndValue);
	m_module->m_llvmIrBuilder.createSub_i (rangeEndValue, ptrValue, intPtrType, &spanValue);
	m_module->m_llvmIrBuilder.createRet (spanValue);

	internalEpilogue ();

	return function;
}

//.............................................................................

} // namespace jnc {

