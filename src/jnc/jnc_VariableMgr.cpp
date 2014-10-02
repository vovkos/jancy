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

	memset (m_stdVariableArray, 0, sizeof (m_stdVariableArray));
	createStdVariables ();
}

void
VariableMgr::createStdVariables ()
{
	for (size_t i = 0; i < StdVariableKind__Count; i++)
		getStdVariable ((StdVariableKind) i);
}

Variable*
VariableMgr::getStdVariable (StdVariableKind variableKind)
{
	ASSERT ((size_t) variableKind < StdVariableKind__Count);

	if (m_stdVariableArray [variableKind])
		return m_stdVariableArray [variableKind];

	Variable* variable;

	switch (variableKind)
	{
	case StdVariableKind_ScopeLevel:
		variable = createVariable (
			StorageKind_Thread,
			"g_scopeLevel",
			"jnc.g_scopeLevel",
			m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT)
			);
		break;

	case StdVariableKind_GcShadowStackTop:
		variable = createVariable (
			StorageKind_Thread,
			"g_gcShadowStackTop",
			"jnc.g_gcShadowStackTop",
			m_module->m_typeMgr.getStdType (StdTypeKind_BytePtr)
			);
		break;

	default:
		ASSERT (false);
		variable = NULL;
	}

	m_stdVariableArray [variableKind] = variable;
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
		break;

	case StorageKind_Stack:
	case StorageKind_Heap:
		break;

	default:
		ASSERT (false);
	}

	if (type->getTypeKindFlags () & TypeKindFlagKind_Import)
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
		storageKind == StorageKind_Static ? PtrTypeFlagKind_Volatile : 0
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
	variable->m_flags |= ModuleItemFlagKind_User;

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

	if ((m_module->getFlags () & ModuleFlagKind_DebugInfo) &&
		(variable->getFlags () & ModuleItemFlagKind_User))
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

	if (type->getTypeKindFlags () & TypeKindFlagKind_Import)
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

		if (variable->m_type->getTypeKindFlags () & TypeKindFlagKind_Aggregate)
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

	if (variable->m_type->getFlags () & TypeFlagKind_GcRoot)
		m_staticGcRootArray.append (variable);

	if (m_module->getFlags () & ModuleFlagKind_DebugInfo)
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

	if ((type->getTypeKindFlags () & TypeKindFlagKind_Ptr) && 
		(type->getFlags () & PtrTypeFlagKind_Safe) &&
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

	if ((m_module->getFlags () & ModuleFlagKind_DebugInfo) &&
		(variable->getFlags () & ModuleItemFlagKind_User))
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
