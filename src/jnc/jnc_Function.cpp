#include "pch.h"
#include "jnc_Function.h"
#include "jnc_Module.h"
#include "jnc_ClassType.h"
#include "jnc_Parser.llk.h"

namespace jnc {

//.............................................................................

const char*
getFunctionKindString (FunctionKind functionKind)
{
	static const char* stringTable [FunctionKind__Count] =
	{
		"undefined-function-kind",  // EFunction_Undefined,
		"named-function",           // EFunction_Named,
		"get",                      // EFunction_Getter,
		"set",                      // EFunction_Setter,
		"bind",                     // EFunction_Binder,
		"prime",                    // EFunction_Primer,
		"preconstruct",             // EFunction_PreConstructor,
		"construct",                // EFunction_Constructor,
		"destruct",                 // EFunction_Destructor,
		"static construct",         // EFunction_StaticConstructor,
		"static destruct",          // EFunction_StaticDestructor,
		"module construct",         // EFunction_ModuleConstructor,
		"module destruct",          // EFunction_ModuleDestructor,
		"call-operator",            // EFunction_CallOperator,
		"cast-operator",            // EFunction_CastOperator,
		"unary-operator",           // EFunction_UnaryOperator,
		"binary-operator",          // EFunction_BinaryOperator,
		"operator_new",             // EFunction_OperatorNew,
		"internal",                 // EFunction_Internal,
		"thunk",                    // EFunction_Thunk,
		"reaction",                 // EFunction_Reaction,
		"schedule-launcher",        // EFunction_ScheduleLauncher,
	};

	return (size_t) functionKind < FunctionKind__Count ?
		stringTable [functionKind] :
		stringTable [FunctionKind_Undefined];
}

//.............................................................................

int
getFunctionKindFlags (FunctionKind functionKind)
{
	static int flagTable [FunctionKind__Count] =
	{
		0,                              // EFunction_Undefined,
		0,                              // EFunction_Named,
		FunctionKindFlag_NoOverloads,  // EFunction_Getter,
		0,                              // EFunction_Setter,
		0,                              // EFunction_Binder,
		0,                              // EFunction_Primer,
		FunctionKindFlag_NoStorage   | // EFunction_PreConstructor,
		FunctionKindFlag_NoOverloads |
		FunctionKindFlag_NoArgs,
		FunctionKindFlag_NoStorage,    // EFunction_Constructor,
		FunctionKindFlag_NoStorage   | // EFunction_Destructor,
		FunctionKindFlag_NoOverloads |
		FunctionKindFlag_NoArgs,
		FunctionKindFlag_NoStorage   | // EFunction_StaticConstructor,
		FunctionKindFlag_NoOverloads |
		FunctionKindFlag_NoArgs,
		FunctionKindFlag_NoStorage   | // EFunction_StaticDestructor,
		FunctionKindFlag_NoOverloads |
		FunctionKindFlag_NoArgs,
		FunctionKindFlag_NoStorage   | // EFunction_ModuleConstructor,
		FunctionKindFlag_NoOverloads |
		FunctionKindFlag_NoArgs,
		FunctionKindFlag_NoStorage   | // EFunction_ModuleDestructor,
		FunctionKindFlag_NoOverloads |
		FunctionKindFlag_NoArgs,
		0,                              // EFunction_CallOperator,
		FunctionKindFlag_NoOverloads | // EFunction_CastOperator,
		FunctionKindFlag_NoArgs,
		FunctionKindFlag_NoOverloads | // EFunction_UnaryOperator,
		FunctionKindFlag_NoArgs,
		0,                              // EFunction_BinaryOperator,
		0,                              // EFunction_OperatorNew,
		0,                              // EFunction_Internal,
		0,                              // EFunction_Thunk,
		0,                              // EFunction_Reaction,
		0,                              // EFunction_ScheduleLauncher,
	};

	return functionKind >= 0 && functionKind < FunctionKind__Count ? flagTable [functionKind] : 0;
}

//.............................................................................

Function::Function ()
{
	m_itemKind = ModuleItemKind_Function;
	m_functionKind = FunctionKind_Undefined;
	m_type = NULL;
	m_castOpType = NULL;
	m_thisArgType = NULL;
	m_thisType = NULL;
	m_thisArgDelta = 0;
	m_thisArgTypeFlags = 0;
	m_virtualOriginClassType = NULL;
	m_property = NULL;
	m_classVTableIndex = -1;
	m_propertyVTableIndex = -1;
	m_entryBlock = NULL;
	m_scope = NULL;
	m_llvmFunction = NULL;
	m_llvmPostTlsPrologueInst = NULL;
	m_pfMachineCode = NULL;
	m_machineCodeSize = 0;
}

void
Function::markGc ()
{
	llvm::Function* llvmFunction = getLlvmFunction ();
	if (!llvmFunction->hasGC ())
		llvmFunction->setGC ("jnc-shadow-stack");
}

bool
Function::setBody (rtl::BoxList <Token>* tokenList)
{
	if (!m_body.isEmpty ())
	{
		err::setFormatStringError ("'%s' already has a body", m_tag.cc ());
		return false;
	}

	if (m_storageKind == StorageKind_Abstract)
	{
		err::setFormatStringError ("'%s' is abstract and hence cannot have a body", m_tag.cc ());
		return false;
	}

	m_body.takeOver (tokenList);
	m_module->markForCompile (this);
	return true;
}

llvm::Function*
Function::getLlvmFunction ()
{
	if (m_llvmFunction)
		return m_llvmFunction;

	m_llvmFunction = m_type->getCallConv ()->createLlvmFunction (m_type, m_tag);
	m_module->m_functionMgr.m_llvmFunctionMap [m_llvmFunction] = this;
	return m_llvmFunction;
}

llvm::DISubprogram
Function::getLlvmDiSubprogram ()
{
	if (m_llvmDiSubprogram)
		return m_llvmDiSubprogram;

	m_llvmDiSubprogram = m_module->m_llvmDiBuilder.createFunction (this);
	return m_llvmDiSubprogram;
}

void
Function::convertToMemberMethod (NamedType* parentType)
{
	ASSERT (m_typeOverload.getOverloadCount () == 1);

	m_parentNamespace = parentType;
	m_type = parentType->getMemberMethodType (m_type, m_thisArgTypeFlags);
	m_typeOverload = m_type;

	ASSERT (!m_type->getArgArray ().isEmpty ());
	m_thisArgType = m_type->getArgArray () [0]->getType ();
	m_thisType = m_thisArgType;
}

bool
Function::addOverload (Function* function)
{
	bool result = m_typeOverload.addOverload (function->m_type);
	if (!result)
		return false;

	m_overloadArray.append (function);
	return true;
}

void
Function::addTlsVariable (Variable* variable)
{
	llvm::AllocaInst* llvmAlloca = (llvm::AllocaInst*) variable->getLlvmValue ();
	ASSERT (llvmAlloca && llvm::isa <llvm::AllocaInst> (*llvmAlloca));

	TlsVariable tlsVariable;
	tlsVariable.m_variable = variable;
	tlsVariable.m_llvmAlloca = llvmAlloca;
	m_tlsVariableArray.append (tlsVariable);
}

bool
Function::compile ()
{
	ASSERT (!m_body.isEmpty ()); // otherwise what are we doing here?
	ASSERT (!m_entryBlock || m_functionKind == FunctionKind_ModuleConstructor);

	bool result;

	if (m_entryBlock) // already compiled
		return true;

	m_module->m_unitMgr.setCurrentUnit (m_itemDecl->getParentUnit ());

	// prologue

	Token::Pos beginPos = m_body.getHead ()->m_pos;
	Token::Pos endPos = m_body.getTail ()->m_pos;

	result = m_module->m_functionMgr.prologue (this, beginPos);
	if (!result)
		return false;

	OnceStmt stmt; // for static constructors

	// parse body

	Parser parser;
	parser.m_module = m_module;
	parser.m_stage = Parser::StageKind_Pass2;

	SymbolKind startSymbol = SymbolKind_compound_stmt;

	if (m_functionKind == FunctionKind_StaticConstructor)
	{
		DerivableType* parentType = getParentType ();
		if (!parentType)
		{
			err::setFormatStringError ("static constructors for properties are not yet supported");
			return false;
		}

		m_module->m_controlFlowMgr.onceStmt_Create (&stmt, parentType->getStaticOnceFlagVariable ());

		result = m_module->m_controlFlowMgr.onceStmt_PreBody (&stmt, beginPos);
		if (!result)
			return false;
	}
	else if (m_functionKind == FunctionKind_PreConstructor)
	{
		DerivableType* parentType = getParentType ();
		if (!parentType)
		{
			err::setFormatStringError ("preconstructors for properties are not yet supported");
			return false;
		}

		Function* staticConstructor = parentType->getStaticConstructor ();
		if (staticConstructor)
			m_module->m_operatorMgr.callOperator (staticConstructor);

		Value thisValue = m_module->m_functionMgr.getThisValue ();
		ASSERT (thisValue);

		TypeKind typeKind = parentType->getTypeKind ();
		switch (typeKind)
		{
		case TypeKind_Struct:
			result = ((StructType*) parentType)->initializeFields (thisValue);
			break;

		case TypeKind_Union:
			result = ((UnionType*) parentType)->initializeField (thisValue);
			break;

		case TypeKind_Class:
			result = ((ClassType*) parentType)->getIfaceStructType ()->initializeFields (thisValue);
			break;
		}
	}
	else if (m_functionKind == FunctionKind_Constructor)
	{
		startSymbol = SymbolKind_constructor_compound_stmt;

		NamespaceKind namespaceKind = m_parentNamespace->getNamespaceKind ();
		ASSERT (namespaceKind == NamespaceKind_Type || namespaceKind == NamespaceKind_Property);

		if (namespaceKind == NamespaceKind_Type)
			parser.m_constructorType = (DerivableType*) m_parentNamespace;
		else
			parser.m_constructorProperty = (Property*) m_parentNamespace;
	}

	result = parser.parseTokenList (startSymbol, m_body, true);
	if (!result)
		return false;

	if (m_functionKind == FunctionKind_StaticConstructor)
		m_module->m_controlFlowMgr.onceStmt_PostBody (&stmt, endPos);

	// epilogue

	m_module->m_namespaceMgr.setSourcePos (endPos);

	result = m_module->m_functionMgr.epilogue ();
	if (!result)
		return false;

	return true;
}

//.............................................................................


ModuleItem*
LazyStdFunction::getActualItem ()
{
	return m_module->m_functionMgr.getStdFunction (m_func);
}

//.............................................................................

} // namespace jnc {
