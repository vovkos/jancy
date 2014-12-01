#include "pch.h"
#include "jnc_VariableMgr.h"
#include "jnc_Parser.llk.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

VariableMgr::VariableMgr ()
{
	m_module = getCurrentThreadModule ();
	ASSERT (m_module);
	
	m_tlsStructType = NULL;
	m_llvmTlsObjHdrValue = NULL;

	memset (m_stdVariableArray, 0, sizeof (m_stdVariableArray));
	createStdVariables ();
}

void
VariableMgr::clear ()
{
	m_variableList.clear ();
	m_aliasList.clear ();

	m_staticVariableArray.clear ();
	m_staticGcRootArray.clear ();
	m_globalStaticVariableArray.clear ();
	m_staticDestructList.clear ();

	m_tlsVariableArray.clear ();
	m_tlsGcRootArray.clear ();
	m_tlsStructType = NULL;
	m_llvmTlsObjHdrValue = NULL;

	memset (m_stdVariableArray, 0, sizeof (m_stdVariableArray));
	createStdVariables ();
}

void
VariableMgr::createStdVariables ()
{
	for (size_t i = 0; i < StdVariable__Count; i++)
		getStdVariable ((StdVariable) i);
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
	case StdVariable_ScopeLevel:
		variable = createVariable (
			StorageKind_Thread,
			"g_scopeLevel",
			"jnc.g_scopeLevel",
			m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT)
			);
		break;

	case StdVariable_GcShadowStackTop:
		variable = createVariable (
			StorageKind_Thread,
			"g_gcShadowStackTop",
			"jnc.g_gcShadowStackTop",
			m_module->m_typeMgr.getStdType (StdType_BytePtr)
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
	Variable* variable = AXL_MEM_NEW (Variable);
	variable->m_module = m_module;
	variable->m_name = name;
	variable->m_qualifiedName = qualifiedName;
	variable->m_tag = qualifiedName;
	variable->m_type = type;
	variable->m_storageKind = storageKind;
	variable->m_ptrTypeFlags = ptrTypeFlags;

	if (storageKind == StorageKind_Stack)
	{
		Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
		ASSERT (scope);

		variable->m_scope = scope;
		variable->m_scopeLevel = scope->getLevel ();
	}

	if (constructor)
		variable->m_constructor.takeOver (constructor);

	if (initializer)
		variable->m_initializer.takeOver (initializer);

	m_variableList.insertTail (variable);

	switch (storageKind)
	{
	case StorageKind_Static:
		m_staticVariableArray.append (variable);

		if (m_module->m_namespaceMgr.getCurrentNamespace ()->getNamespaceKind () == NamespaceKind_Global)
			m_globalStaticVariableArray.append (variable);

		break;

	case StorageKind_Thread:
		m_tlsVariableArray.append (variable);
		variable->m_scopeLevel = 1;
		break;

	case StorageKind_Stack:
	case StorageKind_Heap:
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
VariableMgr::createOnceFlagVariable (StorageKind storageKind)
{
	return createVariable (
		storageKind,
		"once_flag",
		"once_flag",
		m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32),
		storageKind == StorageKind_Static ? PtrTypeFlag_Volatile : 0
		);
}

Variable*
VariableMgr::createArgVariable (
	FunctionArg* arg,
	llvm::Value* llvmArgValue
	)
{
	bool result;

	Variable* variable = createStackVariable (
		arg->getName (),
		arg->getType (),
		arg->getPtrTypeFlags ()
		);

	variable->m_parentUnit = arg->getParentUnit ();
	variable->m_pos = *arg->getPos ();
	variable->m_flags |= ModuleItemFlag_User;

	Value ptrValue;
	result = m_module->m_operatorMgr.allocate (
		StorageKind_Stack,
		arg->getType (),
		arg->getName (),
		&ptrValue
		);

	if (!result)
		return NULL;

	variable->m_llvmAllocValue = ptrValue.getLlvmValue ();
	variable->m_llvmValue = ptrValue.getLlvmValue ();

	if ((m_module->getFlags () & ModuleFlag_DebugInfo) &&
		(variable->getFlags () & ModuleItemFlag_User))
	{
		variable->m_llvmDiDescriptor = m_module->m_llvmDiBuilder.createLocalVariable (
			variable,
			llvm::dwarf::DW_TAG_arg_variable
			);

		m_module->m_llvmDiBuilder.createDeclare (variable);
	}

	return variable;
}

llvm::GlobalVariable*
VariableMgr::createLlvmGlobalVariable (
	Type* type,
	const char* tag
	)
{
	llvm::GlobalVariable* llvmValue = new llvm::GlobalVariable (
		*m_module->getLlvmModule (),
		type->getLlvmType (),
		false,
		llvm::GlobalVariable::InternalLinkage,
		(llvm::Constant*) type->getZeroValue ().getLlvmValue (),
		tag
		);

	m_llvmGlobalVariableArray.append (llvmValue);
	return llvmValue;
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
VariableMgr::initializeGlobalStaticVariables ()
{
	bool result;

	size_t count = m_globalStaticVariableArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		Variable* variable = m_globalStaticVariableArray [i];

		result = m_module->m_operatorMgr.parseInitializer (
			variable,
			variable->m_itemDecl->getParentUnit (),
			variable->m_constructor,
			variable->m_initializer
			);

		if (variable->m_type->getTypeKind () == TypeKind_Class)
		{
			Function* destructor = ((ClassType*) variable->m_type)->getDestructor ();
			if (destructor)
				m_staticDestructList.addDestructor (destructor, variable);
		}

		if (!result)
			return false;
	}

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

void
VariableMgr::allocateVariableObjHdr (Variable* variable)
{
	ASSERT (!variable->m_llvmObjHdrValue);

	StorageKind storageKind = variable->m_storageKind;
	switch (storageKind)
	{
	case StorageKind_Static:
		allocateStaticVariableObjHdr (variable);
		break;

	case StorageKind_Thread:
		allocateTlsVariableObjHdr (variable);
		break;

	case StorageKind_Heap:
		getHeapVariableObjHdr (variable);
		break;

	case StorageKind_Stack:
		allocateStackVariableObjHdr (variable);
		break;

	default:
		ASSERT (false);
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

	// initialize within 'once' block

	Token::Pos pos = *variable->getItemDecl ()->getPos ();

	OnceStmt stmt;
	m_module->m_controlFlowMgr.onceStmt_Create (&stmt, pos);

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

	if (variable->m_type->getTypeKind () == TypeKind_Class)
	{
		Function* destructor = ((ClassType*) variable->m_type)->getDestructor ();
		if (destructor)
			m_staticDestructList.addDestructor (destructor, variable, stmt.m_flagVariable);
	}

	if (!variable->m_initializer.isEmpty ())
		pos = variable->m_initializer.getTail ()->m_pos;
	else if (!variable->m_constructor.isEmpty ())
		pos = variable->m_constructor.getTail ()->m_pos;

	m_module->m_controlFlowMgr.onceStmt_PostBody (&stmt, pos);

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
			variable->m_initializer.getHead ()->m_token == '{')
		{
			m_module->m_llvmIrBuilder.createStore (variable->m_type->getZeroValue (), ptrValue);
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

void
VariableMgr::initializeVariableObjHdr (
	const Value& objHdrValue,
	const Value& scopeLevelValue,
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
	m_module->m_llvmIrBuilder.createStore (scopeLevelValue, dstValue);
	m_module->m_llvmIrBuilder.createGep2 (dstValue0, 1, NULL, &dstValue);
	m_module->m_llvmIrBuilder.createStore (dstValue0, dstValue);
	m_module->m_llvmIrBuilder.createGep2 (dstValue0, 2, NULL, &dstValue);
	m_module->m_llvmIrBuilder.createStore (typeValue, dstValue);
	m_module->m_llvmIrBuilder.createGep2 (dstValue0, 3, NULL, &dstValue);
	m_module->m_llvmIrBuilder.createStore (Value (flags, TypeKind_Int_pu), dstValue);

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
VariableMgr::allocateStaticVariableObjHdr (Variable* variable)
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
		Type* type = m_module->m_typeMgr.getStdType (StdType_VariableObjHdr);
		llvmValue = createLlvmGlobalVariable (type, variable->m_tag + ":objHdr");
		initializeVariableObjHdr (
			llvmValue, 
			0, 
			type, 
			ObjHdrFlag_Static | 
			ObjHdrFlag_GcMark | 
			ObjHdrFlag_GcWeakMark | 
			ObjHdrFlag_GcRootsAdded,
			variable->getLlvmValue ()
			);
	}

	Value ptrValue;
	m_module->m_llvmIrBuilder.createGep2 (llvmValue, 0, NULL, &ptrValue);
	variable->m_llvmObjHdrValue = ptrValue.getLlvmValue ();
	m_module->m_controlFlowMgr.setCurrentBlock (block);
}

void
VariableMgr::allocateTlsVariableObjHdr (Variable* variable)
{
	ASSERT (variable->m_type->getTypeKind () != TypeKind_Class);
	if (m_llvmTlsObjHdrValue)
	{
		variable->m_llvmObjHdrValue = m_llvmTlsObjHdrValue;
		return;
	}

	static void* null = NULL;
	Value nullPtrValue (&null, m_module->m_typeMgr.getStdType (StdType_BytePtr));

	Type* type = m_module->m_typeMgr.getStdType (StdType_VariableObjHdr);
	llvm::Value* llvmValue = createLlvmGlobalVariable (type, "jnc.g_tlsObjHdr");
	initializeVariableObjHdr (
		llvmValue, 
		1, 
		NULL, 
		ObjHdrFlag_Static | 
		ObjHdrFlag_GcMark | 
		ObjHdrFlag_GcWeakMark | 
		ObjHdrFlag_GcRootsAdded,
		nullPtrValue
		);

	Value ptrValue;
	m_module->m_llvmIrBuilder.createGep2 (llvmValue, 0, NULL, &ptrValue);
	m_llvmTlsObjHdrValue = ptrValue.getLlvmValue ();
	variable->m_llvmObjHdrValue = m_llvmTlsObjHdrValue;
}

void
VariableMgr::getHeapVariableObjHdr (Variable* variable)
{
	Type* type = m_module->m_typeMgr.getStdType (StdType_ObjHdrPtr);

	Value objHdrValue;
	m_module->m_llvmIrBuilder.createBitCast (variable->m_llvmValue, type, &objHdrValue);
	m_module->m_llvmIrBuilder.createGep (objHdrValue, -1, NULL, &objHdrValue);
	variable->m_llvmObjHdrValue = objHdrValue.getLlvmValue ();
}

void
VariableMgr::allocateStackVariableObjHdr (Variable* variable)
{
	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	ASSERT (function);

	BasicBlock* prevBlock = m_module->m_controlFlowMgr.setCurrentBlock (function->getEntryBlock ());
	
	Value objHdrValue;
	Type* type = m_module->m_typeMgr.getStdType (StdType_VariableObjHdr);
	m_module->m_llvmIrBuilder.createAlloca (type, variable->m_tag + ":objHdr", NULL, &objHdrValue);

	initializeVariableObjHdr (
		objHdrValue, 
		m_module->m_namespaceMgr.getScopeLevel (variable->m_scopeLevel), 
		variable->m_type, 
		ObjHdrFlag_Stack | 
		ObjHdrFlag_GcMark | 
		ObjHdrFlag_GcWeakMark | 
		ObjHdrFlag_GcRootsAdded,
		variable->getLlvmValue ()
		);

	Value ptrValue;
	m_module->m_llvmIrBuilder.createGep2 (objHdrValue, 0, NULL, &ptrValue);
	variable->m_llvmObjHdrValue = ptrValue.getLlvmValue ();
	m_module->m_controlFlowMgr.setCurrentBlock (prevBlock);
}

void
VariableMgr::allocateTlsVariable (Variable* variable)
{
	Value ptrValue;
	llvm::AllocaInst* llvmAlloca = m_module->m_llvmIrBuilder.createAlloca (
		variable->m_type,
		variable->m_qualifiedName,
		NULL,
		&ptrValue
		);

	variable->m_llvmAllocValue = llvmAlloca;
	variable->m_llvmValue = llvmAlloca;

	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	ASSERT (function);

	function->addTlsVariable (variable);
}

void
VariableMgr::deallocateTlsVariableArray (
	const TlsVariable* array,
	size_t count
	)
{
	for (size_t i = 0; i < count; i++)
	{
		Variable* variable = array [i].m_variable;
		ASSERT (variable->m_llvmValue == array [i].m_llvmAlloca);

		variable->m_llvmValue = NULL;
		variable->m_llvmAllocValue = NULL;
	}
}

void
VariableMgr::restoreTlsVariableArray (
	const TlsVariable* array,
	size_t count
	)
{
	for (size_t i = 0; i < count; i++)
	{
		Variable* variable = array [i].m_variable;
		llvm::AllocaInst* llvmAlloca = array [i].m_llvmAlloca;

		variable->m_llvmValue = llvmAlloca;
		variable->m_llvmAllocValue = llvmAlloca;
	}
}

//.............................................................................

} // namespace jnc {
