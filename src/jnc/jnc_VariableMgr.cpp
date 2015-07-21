#include "pch.h"
#include "jnc_VariableMgr.h"
#include "jnc_Parser.llk.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

VariableMgr::VariableMgr ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);
	
	m_tlsStructType = NULL;

	memset (m_stdVariableArray, 0, sizeof (m_stdVariableArray));
	getStdVariable (StdVariable_GcShadowStackTop); // this variable is required even if it's not used
}

void
VariableMgr::clear ()
{
	m_variableList.clear ();
	m_aliasList.clear ();

	m_staticVariableArray.clear ();
	m_staticGcRootArray.clear ();
	m_globalStaticVariableArray.clear ();

	m_tlsVariableArray.clear ();
	m_tlsStructType = NULL;

	memset (m_stdVariableArray, 0, sizeof (m_stdVariableArray));
	getStdVariable (StdVariable_GcShadowStackTop); // this variable is required even if it's not used
}

Variable*
VariableMgr::getStdVariable (StdVariable stdVariable)
{
	ASSERT ((size_t) stdVariable < StdVariable__Count);

	if (m_stdVariableArray [stdVariable])
		return m_stdVariableArray [stdVariable];

	Variable* variable;

	switch (stdVariable)
	{
	case StdVariable_GcShadowStackTop:
		variable = createVariable (
			StorageKind_Thread,
			"g_gcShadowStackTop",
			"jnc.g_gcShadowStackTop",
			m_module->m_typeMgr.getStdType (StdType_BytePtr)
			);
		break;

	case StdVariable_GcSafePointTarget:
		variable = createVariable (
			StorageKind_Static,
			"g_gcSafePointTarget",
			"jnc.g_gcSafePointTarget",
			m_module->m_typeMgr.getPrimitiveType (TypeKind_IntPtr)->getDataPtrType_c ()
			);
		break;

	default:
		ASSERT (false);
		variable = NULL;
	}

	m_stdVariableArray [stdVariable] = variable;
	return variable;
}

Variable*
VariableMgr::createVariable (
	StorageKind storageKind,
	const rtl::String& name,
	const rtl::String& qualifiedName,
	Type* type,
	uint_t ptrTypeFlags,
	rtl::BoxList <Token>* constructor,
	rtl::BoxList <Token>* initializer
	)
{
	bool result;

	Variable* variable = AXL_MEM_NEW (Variable);
	variable->m_module = m_module;
	variable->m_name = name;
	variable->m_qualifiedName = qualifiedName;
	variable->m_tag = qualifiedName;
	variable->m_type = type;
	variable->m_storageKind = storageKind;
	variable->m_ptrTypeFlags = ptrTypeFlags;
	variable->m_scope = m_module->m_namespaceMgr.getCurrentScope ();

	if (constructor)
		variable->m_constructor.takeOver (constructor);

	if (initializer)
		variable->m_initializer.takeOver (initializer);

	m_variableList.insertTail (variable);

	Value ptrValue;

	switch (storageKind)
	{
	case StorageKind_Static:
		m_staticVariableArray.append (variable);

		if (!m_module->m_namespaceMgr.getCurrentScope ())
		{
			m_globalStaticVariableArray.append (variable);
			break;
		}

		variable->m_llvmValue = createLlvmGlobalVariable (type, qualifiedName);
		break;

	case StorageKind_Thread:
		m_tlsVariableArray.append (variable);
		break;

	case StorageKind_Stack:
		m_module->m_llvmIrBuilder.createAlloca (type, qualifiedName, NULL, &ptrValue);
		variable->m_llvmValue = ptrValue.getLlvmValue ();
		break;

	case StorageKind_Heap:
		result = m_module->m_operatorMgr.gcHeapAllocate (type, &ptrValue);
		if (!result)
			return NULL;
		
		variable->m_llvmValue = ptrValue.getLlvmValue ();
		break;

	default:
		ASSERT (false);
	}

	if (type->getTypeKindFlags () & TypeKindFlag_Import)
	{
		variable->m_type_i = (ImportType*) type;
		m_module->markForLayout (variable);
	}

	return variable;
}

Variable*
VariableMgr::createSimpleStaticVariable (
	const rtl::String& name,
	const rtl::String& qualifiedName,
	Type* type,
	const Value& value,
	uint_t ptrTypeFlags
	)
{
	ASSERT (type->getTypeKind () != TypeKind_Class);

	Variable* variable = AXL_MEM_NEW (Variable);
	variable->m_module = m_module;
	variable->m_name = name;
	variable->m_qualifiedName = qualifiedName;
	variable->m_tag = qualifiedName;
	variable->m_type = type;
	variable->m_storageKind = StorageKind_Static;
	variable->m_ptrTypeFlags = ptrTypeFlags;
	variable->m_scope = m_module->m_namespaceMgr.getCurrentScope ();
	variable->m_llvmValue = createLlvmGlobalVariable (type, qualifiedName, value);

	m_variableList.insertTail (variable);
	return variable;
}

bool
VariableMgr::initializeVariable (Variable* variable)
{
	switch (variable->m_storageKind)
	{
	case StorageKind_Static:
		if (variable->m_type->getTypeKind () == TypeKind_Class)
			primeStaticClassVariable (variable);
		break;

	case StorageKind_Thread:
		break;

	case StorageKind_Stack:
		if (variable->m_type->getFlags () & TypeFlag_GcRoot)
		{
			m_module->m_operatorMgr.zeroInitialize (variable);
			m_module->m_operatorMgr.markStackGcRoot (StackGcRootKind_Scope, variable, variable->m_type);
		}
		else if ((variable->m_type->getTypeKindFlags () & TypeKindFlag_Aggregate) || variable->m_initializer.isEmpty ())
		{
			m_module->m_operatorMgr.zeroInitialize (variable);
		}
		break;

	case StorageKind_Heap:
		m_module->m_operatorMgr.markStackGcRoot (StackGcRootKind_Scope, variable, variable->m_type->getDataPtrType_c ());
		break;

	default:
		ASSERT (false);
	};

	return m_module->m_operatorMgr.parseInitializer (
		variable,
		variable->m_itemDecl->getParentUnit (),
		variable->m_constructor,
		variable->m_initializer
		);
}

llvm::GlobalVariable*
VariableMgr::createLlvmGlobalVariable (
	Type* type,
	const char* tag,
	const Value& initValue
	)
{
	llvm::Constant* llvmInitConstant = initValue ? 
		(llvm::Constant*) initValue.getLlvmValue () :
		(llvm::Constant*) type->getZeroValue ().getLlvmValue ();

	return new llvm::GlobalVariable (
		*m_module->getLlvmModule (),
		type->getLlvmType (),
		false,
		llvm::GlobalVariable::InternalLinkage,
		llvmInitConstant,
		tag
		);
}

void
VariableMgr::primeStaticClassVariable (Variable* variable)
{
	ASSERT (variable->m_storageKind == StorageKind_Static && variable->m_type->getTypeKind () == TypeKind_Class);

	Function* primeStaticClass = m_module->m_functionMgr.getStdFunction (StdFunction_PrimeStaticClass);

	Value argValueArray [2];
	m_module->m_llvmIrBuilder.createBitCast (
		variable->m_llvmValue,  
		m_module->m_typeMgr.getStdType (StdType_BoxPtr), 
		&argValueArray [0]
		);

	argValueArray [1].createConst (
		&variable->m_type, 
		m_module->m_typeMgr.getStdType (StdType_BytePtr)
		);

	m_module->m_llvmIrBuilder.createCall (
		primeStaticClass, 
		primeStaticClass->getType (),
		argValueArray, 
		2,
		NULL
		);

	Value ifaceValue;
	m_module->m_llvmIrBuilder.createGep2 (variable->m_llvmValue, 1, NULL, &ifaceValue);
	variable->m_llvmValue = ifaceValue.getLlvmValue ();

	Function* destructor = ((ClassType*) variable->m_type)->getDestructor ();
	if (destructor)
	{
		Function* addDestructor = m_module->m_functionMgr.getStdFunction (StdFunction_AddStaticClassDestructor);

		Value argValueArray [2];

		m_module->m_llvmIrBuilder.createBitCast (destructor, m_module->m_typeMgr.getStdType (StdType_BytePtr), &argValueArray [0]);
		m_module->m_llvmIrBuilder.createBitCast (variable, m_module->m_typeMgr.getStdType (StdType_AbstractClassPtr), &argValueArray [1]);
		m_module->m_llvmIrBuilder.createCall (addDestructor, addDestructor->getType (), argValueArray, countof (argValueArray), NULL);
	}
}

Variable*
VariableMgr::createOnceFlagVariable (StorageKind storageKind)
{
	return createVariable (
		storageKind,
		"onceFlag",
		"onceFlag",
		m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32),
		storageKind == StorageKind_Static ? PtrTypeFlag_Volatile : 0
		);
}

Variable*
VariableMgr::createStaticDataPtrValidatorVariable (Variable* variable)
{
	ASSERT (variable->m_storageKind == StorageKind_Static);

	// create static box

	StructType* boxType = (StructType*) m_module->m_typeMgr.getStdType (StdType_StaticDataBox);

	uintptr_t flags = BoxFlag_StaticData | BoxFlag_StrongMark | BoxFlag_WeakMark;

	Value variablePtrValue;
	m_module->m_llvmIrBuilder.createBitCast (
		variable, 
		m_module->m_typeMgr.getStdType (StdType_BytePtr), 
		&variablePtrValue
		);

	ASSERT (llvm::isa <llvm::Constant> (variablePtrValue.getLlvmValue ()));

	llvm::Constant* llvmMemberArray [4];
	llvmMemberArray [0] = Value::getLlvmConst (m_module->m_typeMgr.getStdType (StdType_BytePtr), &variable->m_type);
	llvmMemberArray [1] = Value::getLlvmConst (m_module->m_typeMgr.getPrimitiveType (TypeKind_IntPtr_u), &flags);
	llvmMemberArray [2] = (llvm::Constant*) variablePtrValue.getLlvmValue ();

	llvm::Constant* llvmBoxConst = llvm::ConstantStruct::get (
		(llvm::StructType*) boxType->getLlvmType (),
		llvm::ArrayRef <llvm::Constant*> (llvmMemberArray, 3)
		);

	rtl::String boxTag = variable->m_tag + ".box";

	llvm::GlobalVariable* llvmBoxVariable = new llvm::GlobalVariable (
		*m_module->getLlvmModule (),
		boxType->getLlvmType (),
		false,
		llvm::GlobalVariable::InternalLinkage,
		llvmBoxConst,
		boxTag.cc ()
		);

	// now validator

	StructType* validatorType = (StructType*) m_module->m_typeMgr.getStdType (StdType_DataPtrValidator);

	Value boxPtrValue;
	m_module->m_llvmIrBuilder.createBitCast (
		llvmBoxVariable, 
		m_module->m_typeMgr.getStdType (StdType_BoxPtr), 
		&boxPtrValue
		);

	size_t size = variable->m_type->getSize ();

	ASSERT (llvm::isa <llvm::Constant> (boxPtrValue.getLlvmValue ()));

	llvmMemberArray [0] = (llvm::Constant*) boxPtrValue.getLlvmValue ();
	llvmMemberArray [1] = (llvm::Constant*) boxPtrValue.getLlvmValue ();
	llvmMemberArray [2] = (llvm::Constant*) variablePtrValue.getLlvmValue ();
	llvmMemberArray [3] = Value::getLlvmConst (m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT), &size);

	llvm::Constant* llvmValidatorConst = llvm::ConstantStruct::get (
		(llvm::StructType*) validatorType->getLlvmType (),
		llvm::ArrayRef <llvm::Constant*> (llvmMemberArray, 4)
		);

	rtl::String validatorTag = variable->m_tag + ".validator";

	llvm::GlobalVariable* llvmValidatorVariable = new llvm::GlobalVariable (
		*m_module->getLlvmModule (),
		validatorType->getLlvmType (),
		false,
		llvm::GlobalVariable::InternalLinkage,
		llvmValidatorConst,
		validatorTag.cc ()
		);

	Variable* validatorVariable = AXL_MEM_NEW (Variable);
	validatorVariable->m_module = m_module;
	validatorVariable->m_name = validatorTag;
	validatorVariable->m_qualifiedName = validatorTag;
	validatorVariable->m_tag = validatorTag;
	validatorVariable->m_type = validatorType;
	validatorVariable->m_storageKind = StorageKind_Static;
	validatorVariable->m_ptrTypeFlags = 0;
	validatorVariable->m_scope = NULL;
	validatorVariable->m_llvmValue = llvmValidatorVariable;
	m_variableList.insertTail (validatorVariable);

	return validatorVariable;
}

void
VariableMgr::liftStackVariable (Variable* variable)
{
	ASSERT (variable->m_storageKind == StorageKind_Stack);
	ASSERT (llvm::isa <llvm::AllocaInst> (variable->m_llvmValue));
	ASSERT (variable->m_scope == m_module->m_namespaceMgr.getCurrentScope ());

	llvm::AllocaInst* llvmAlloca = (llvm::AllocaInst*) (llvm::AllocaInst*) variable->m_llvmValue;
	BasicBlock* currentBlock = m_module->m_controlFlowMgr.getCurrentBlock ();
	m_module->m_llvmIrBuilder.setInsertPoint (llvmAlloca);

	Value typeValue (&variable->m_type, m_module->m_typeMgr.getStdType (StdType_BytePtr));
	Value ptrValue;
	Value variableValue;
	Value validatorValue;

	Function* allocate = m_module->m_functionMgr.getStdFunction (StdFunction_AllocateData);
	bool result = m_module->m_operatorMgr.callOperator (allocate, typeValue, &ptrValue);
	ASSERT (result);

	m_module->m_llvmIrBuilder.createExtractValue (ptrValue, 0, NULL, &variableValue);
	m_module->m_llvmIrBuilder.createExtractValue (ptrValue, 1, NULL, &validatorValue);
	m_module->m_llvmIrBuilder.createBitCast (variableValue, variable->m_type->getDataPtrType_c (), &variableValue);
	m_module->m_operatorMgr.markStackGcRoot (StackGcRootKind_Scope, variableValue, variable->m_type->getDataPtrType_c ());
	m_module->m_llvmIrBuilder.setInsertPoint (currentBlock);

	variable->m_llvmValue = variableValue.getLlvmValue ();
	variable->m_leanDataPtrValidator->m_validatorValue = validatorValue;
	variable->m_storageKind = StorageKind_Heap;

	llvmAlloca->replaceAllUsesWith (variable->m_llvmValue);
	llvmAlloca->eraseFromParent ();
}

Variable*
VariableMgr::createArgVariable (FunctionArg* arg)
{
	Variable* variable = createSimpleStackVariable (
		arg->getName (),
		arg->getType (),
		arg->getPtrTypeFlags ()
		);

	variable->m_parentUnit = arg->getParentUnit ();
	variable->m_pos = *arg->getPos ();
	variable->m_flags |= ModuleItemFlag_User;

	if ((m_module->getFlags () & ModuleFlag_DebugInfo) &&
		(variable->getFlags () & ModuleItemFlag_User))
	{
		variable->m_llvmDiDescriptor = m_module->m_llvmDiBuilder.createLocalVariable (
			variable,
			llvm::dwarf::DW_TAG_arg_variable
			);

		m_module->m_llvmDiBuilder.createDeclare (variable);
	}

	// arg variables are not initialized (stored to directly), so mark gc root manually

	if (variable->m_type->getFlags () & TypeFlag_GcRoot)
		m_module->m_operatorMgr.markStackGcRoot (StackGcRootKind_Function, variable, variable->m_type);
	
	return variable;
}

Alias*
VariableMgr::createAlias (
	const rtl::String& name,
	const rtl::String& qualifiedName,
	Type* type,
	rtl::BoxList <Token>* initializer
	)
{
	ASSERT (initializer);

	Alias* alias = AXL_MEM_NEW (Alias);
	alias->m_name = name;
	alias->m_qualifiedName = qualifiedName;
	alias->m_tag = qualifiedName;
	alias->m_type = type;
	alias->m_initializer.takeOver (initializer);

	m_aliasList.insertTail (alias);

	if (type->getTypeKindFlags () & TypeKindFlag_Import)
	{
		alias->m_type_i = (ImportType*) type;
		m_module->markForLayout (alias);
	}

	return alias;
}

bool
VariableMgr::createTlsStructType ()
{
	bool result;

	StructType* type = m_module->m_typeMgr.createStructType ("Tls", "jnc.Tls");

	size_t count = m_tlsVariableArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		Variable* variable = m_tlsVariableArray [i];

		if (variable->m_type->getTypeKindFlags () & TypeKindFlag_Aggregate)
		{
			err::setFormatStringError ("'thread' variables cannot have aggregate type '%s'",  variable->m_type->getTypeString ().cc ());
			return false;
		}

		variable->m_tlsField = type->createField (variable->m_type);
	}

	result = type->ensureLayout ();
	if (!result)
		return false;

	m_tlsStructType = type;
	return true;
}

bool
VariableMgr::allocateInitializeGlobalVariables ()
{
	bool result;

	size_t count = m_globalStaticVariableArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		Variable* variable = m_globalStaticVariableArray [i];
		ASSERT (!variable->m_llvmValue);

		variable->m_llvmValue = createLlvmGlobalVariable (variable->m_type, variable->m_qualifiedName);

		result = initializeVariable (variable);
		if (!result)
			return false;
	}

	return true;
}

/*

bool
VariableMgr::allocatePrimeStaticVariables ()
{
	bool result;

	size_t count = m_staticVariableArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		Variable* variable = m_staticVariableArray [i];

		result = allocatePrimeStaticVariable (variable);
		if (!result)
			return false;
	}

	return true;
}

bool
VariableMgr::allocatePrimeStaticVariable (Variable* variable)
{
	ASSERT (variable->m_storageKind == StorageKind_Static);
	ASSERT (m_module->m_controlFlowMgr.getCurrentBlock () == m_module->getConstructor ()->getEntryBlock ());

	Type* type = variable->getType ();

	variable->m_llvmAllocValue = createLlvmGlobalVariable (type, variable->getQualifiedName ());

	Value ptrValue (variable->m_llvmAllocValue, type->getDataPtrType_c ());
	bool result = m_module->m_operatorMgr.prime (StorageKind_Static, ptrValue, type, &ptrValue);
	if (!result)
		return false;

	variable->m_llvmValue = ptrValue.getLlvmValue ();

	if (variable->m_type->getFlags () & TypeFlag_GcRoot)
		m_staticGcRootArray.append (variable);

	if (m_module->getFlags () & ModuleFlag_DebugInfo)
		variable->m_llvmDiDescriptor = m_module->m_llvmDiBuilder.createGlobalVariable (variable);

	return true;
}

bool
VariableMgr::allocatePrimeInitializeVariable (Variable* variable)
{
	Type* type = variable->getType ();

	if ((type->getTypeKindFlags () & TypeKindFlag_Ptr) && 
		(type->getFlags () & PtrTypeFlag_Safe) &&
		variable->getInitializer ().isEmpty ())
	{
		err::setFormatStringError (
			"missing initalizer for '%s' variable '%s'", 
			type->getTypeString ().cc (),
			variable->getQualifiedName ().cc ()
			);

		return false;
	}

	StorageKind storageKind = variable->m_storageKind;
	switch (storageKind)
	{
	case StorageKind_Static:
		return allocatePrimeInitializeStaticVariable (variable);

	case StorageKind_Thread:
		return allocatePrimeInitializeTlsVariable (variable);

	default:
		return allocatePrimeInitializeNonStaticVariable (variable);
	}
}

bool
VariableMgr::allocatePrimeInitializeStaticVariable (Variable* variable)
{
	bool result;

	// allocate and prime in module constructor

	BasicBlock* block = m_module->m_controlFlowMgr.getCurrentBlock ();
	m_module->m_controlFlowMgr.setCurrentBlock (m_module->getConstructor ()->getEntryBlock ());

	allocatePrimeStaticVariable (variable);

	m_module->m_controlFlowMgr.setCurrentBlock (block);


	return true;
}

bool
VariableMgr::allocatePrimeInitializeTlsVariable (Variable* variable)
{
	bool result;

	allocateTlsVariable (variable);

	// initialize within 'once' block

	Token::Pos pos = *variable->getItemDecl ()->getPos ();

	OnceStmt stmt;
	m_module->m_controlFlowMgr.onceStmt_Create (&stmt, pos, StorageKind_Thread);

	result =
		m_module->m_controlFlowMgr.onceStmt_PreBody (&stmt, pos) &&
		m_module->m_operatorMgr.parseInitializer (
			variable,
			variable->m_itemDecl->getParentUnit (),
			variable->m_constructor,
			variable->m_initializer
			);

	if (!result)
		return false;

	if (!variable->m_initializer.isEmpty ())
		pos = variable->m_initializer.getTail ()->m_pos;
	else if (!variable->m_constructor.isEmpty ())
		pos = variable->m_constructor.getTail ()->m_pos;

	m_module->m_controlFlowMgr.onceStmt_PostBody (&stmt, pos);

	return true;
}

bool
VariableMgr::allocatePrimeInitializeNonStaticVariable (Variable* variable)
{
	bool result;

	Value ptrValue;
	result = m_module->m_operatorMgr.allocate (
		variable->m_storageKind,
		variable->m_type,
		variable->m_tag,
		&ptrValue
		);

	if (!result)
		return false;
	
	if (variable->m_storageKind == StorageKind_Heap) // local heap variable
		m_module->m_operatorMgr.markStackGcRoot (
			ptrValue, 
			variable->m_type->getDataPtrType_c ()
			);

	variable->m_llvmAllocValue = ptrValue.getLlvmValue ();

	if (variable->m_type->getTypeKind () == TypeKind_Class)
	{
		result = m_module->m_operatorMgr.prime (variable->m_storageKind, ptrValue, variable->m_type, &ptrValue);
		if (!result)
			return false;

		variable->m_llvmValue = ptrValue.getLlvmValue ();
	}
	else
	{
		variable->m_llvmValue = variable->m_llvmAllocValue;

		if (variable->m_initializer.isEmpty () ||
			variable->m_initializer.getHead ()->m_token == '{' ||
			(variable->getType ()->getFlags () & TypeFlag_GcRoot))
		{
			m_module->m_operatorMgr.zeroInitialize (ptrValue, variable->m_type);
		}
	}

	if ((m_module->getFlags () & ModuleFlag_DebugInfo) &&
		(variable->getFlags () & ModuleItemFlag_User))
	{
		variable->m_llvmDiDescriptor = m_module->m_llvmDiBuilder.createLocalVariable (variable);
		m_module->m_llvmDiBuilder.createDeclare (variable);
	}

	result = m_module->m_operatorMgr.parseInitializer (
		variable,
		variable->m_itemDecl->getParentUnit (),
		variable->m_constructor,
		variable->m_initializer
		);

	if (!result)
		return false;

	return true;
}

*/
//.............................................................................

} // namespace jnc {


#if 0

void
VariableMgr::allocateVariableBox (Variable* variable)
{
	ASSERT (!variable->m_llvmBoxValue);

	StorageKind storageKind = variable->m_storageKind;
	switch (storageKind)
	{
	case StorageKind_Static:
		allocateStaticVariableBox (variable);
		break;

	case StorageKind_Thread:
		allocateTlsVariableBox (variable);
		break;

	case StorageKind_Heap:
		getHeapVariableBox (variable);
		break;

	case StorageKind_Stack:
		allocateStackVariableBox (variable);
		break;

	default:
		ASSERT (false);
	}
}

void
VariableMgr::initializeVariableBox (
	const Value& objHdrValue,
	Type* type,
	uint_t flags,
	const Value& ptrValue
	)
{
	Value typeValue (&type, m_module->m_typeMgr.getStdType (StdType_BytePtr));

	// obj hdr

	Value dstValue0, dstValue;
	m_module->m_llvmIrBuilder.createGep2 (objHdrValue, 0, NULL, &dstValue0);
	m_module->m_llvmIrBuilder.createGep2 (dstValue0, 0, NULL, &dstValue);
	m_module->m_llvmIrBuilder.createStore (dstValue0, dstValue);
	m_module->m_llvmIrBuilder.createGep2 (dstValue0, 1, NULL, &dstValue);
	m_module->m_llvmIrBuilder.createStore (typeValue, dstValue);
	m_module->m_llvmIrBuilder.createGep2 (dstValue0, 2, NULL, &dstValue);
	m_module->m_llvmIrBuilder.createStore (
		Value (flags, m_module->m_typeMgr.getPrimitiveType (TypeKind_IntPtr_u)), 
		dstValue
		);

	// variable ptr

	Value srcPtrValue;
	m_module->m_llvmIrBuilder.createBitCast (
		ptrValue, 
		m_module->m_typeMgr.getStdType (StdType_BytePtr),
		&srcPtrValue
		);

	m_module->m_llvmIrBuilder.createGep2 (objHdrValue, 1, NULL, &dstValue);
	m_module->m_llvmIrBuilder.createStore (srcPtrValue, dstValue);
}

void
VariableMgr::allocateStaticVariableBox (Variable* variable)
{
	BasicBlock* block = m_module->m_controlFlowMgr.getCurrentBlock ();
	m_module->m_controlFlowMgr.setCurrentBlock (m_module->getConstructor ()->getEntryBlock ());

	llvm::Value* llvmValue;

	if (variable->m_type->getTypeKind () == TypeKind_Class)
	{
		llvmValue = variable->m_llvmAllocValue;
	}
	else
	{
		Type* type = m_module->m_typeMgr.getStdType (StdType_VariableBox);
		llvmValue = createLlvmGlobalVariable (type, variable->m_tag + ":objHdr");
		initializeVariableBox (
			llvmValue, 
			type, 
			BoxFlag_Static | 
			BoxFlag_GcMark | 
			BoxFlag_GcWeakMark | 
			BoxFlag_GcRootsAdded,
			variable->getLlvmValue ()
			);
	}

	Value ptrValue;
	m_module->m_llvmIrBuilder.createGep2 (llvmValue, 0, NULL, &ptrValue);
	variable->m_llvmBoxValue = ptrValue.getLlvmValue ();
	m_module->m_controlFlowMgr.setCurrentBlock (block);
}

void
VariableMgr::allocateTlsVariableBox (Variable* variable)
{
	ASSERT (variable->m_type->getTypeKind () != TypeKind_Class);
	if (m_llvmTlsBoxValue)
	{
		variable->m_llvmBoxValue = m_llvmTlsBoxValue;
		return;
	}

	static void* null = NULL;
	Value nullPtrValue (&null, m_module->m_typeMgr.getStdType (StdType_BytePtr));

	Type* type = m_module->m_typeMgr.getStdType (StdType_VariableBox);
	llvm::Value* llvmValue = createLlvmGlobalVariable (type, "jnc.g_tlsBox");
	initializeVariableBox (
		llvmValue, 
		NULL, 
		BoxFlag_Static | 
		BoxFlag_GcMark | 
		BoxFlag_GcWeakMark | 
		BoxFlag_GcRootsAdded,
		nullPtrValue
		);

	Value ptrValue;
	m_module->m_llvmIrBuilder.createGep2 (llvmValue, 0, NULL, &ptrValue);
	m_llvmTlsBoxValue = ptrValue.getLlvmValue ();
	variable->m_llvmBoxValue = m_llvmTlsBoxValue;
}

void
VariableMgr::getHeapVariableBox (Variable* variable)
{
	Type* type = m_module->m_typeMgr.getStdType (StdType_BoxPtr);

	Value objHdrValue;
	m_module->m_llvmIrBuilder.createBitCast (variable->m_llvmValue, type, &objHdrValue);
	m_module->m_llvmIrBuilder.createGep (objHdrValue, -1, NULL, &objHdrValue);
	variable->m_llvmBoxValue = objHdrValue.getLlvmValue ();
}

void
VariableMgr::allocateStackVariableBox (Variable* variable)
{
	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	ASSERT (function);

	BasicBlock* prevBlock = m_module->m_controlFlowMgr.setCurrentBlock (function->getEntryBlock ());
	
	Value objHdrValue;
	Type* type = m_module->m_typeMgr.getStdType (StdType_VariableBox);
	m_module->m_llvmIrBuilder.createAlloca (type, variable->m_tag + ":objHdr", NULL, &objHdrValue);

	initializeVariableBox (
		objHdrValue, 
		variable->m_type, 
		BoxFlag_Stack | 
		BoxFlag_GcMark | 
		BoxFlag_GcWeakMark | 
		BoxFlag_GcRootsAdded,
		variable->getLlvmValue ()
		);

	Value ptrValue;
	m_module->m_llvmIrBuilder.createGep2 (objHdrValue, 0, NULL, &ptrValue);
	variable->m_llvmBoxValue = ptrValue.getLlvmValue ();
	m_module->m_controlFlowMgr.setCurrentBlock (prevBlock);
}

#endif
