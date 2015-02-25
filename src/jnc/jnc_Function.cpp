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
		"undefined-function-kind",  // FunctionKind_Undefined,
		"named-function",           // FunctionKind_Named,
		"get",                      // FunctionKind_Getter,
		"set",                      // FunctionKind_Setter,
		"bind",                     // FunctionKind_Binder,
		"prime",                    // FunctionKind_Primer,
		"preconstruct",             // FunctionKind_Preconstructor,
		"construct",                // FunctionKind_Constructor,
		"destruct",                 // FunctionKind_Destructor,
		"static construct",         // FunctionKind_StaticConstructor,
		"static destruct",          // FunctionKind_StaticDestructor,
		"module construct",         // FunctionKind_ModuleConstructor,
		"module destruct",          // FunctionKind_ModuleDestructor,
		"call-operator",            // FunctionKind_CallOperator,
		"cast-operator",            // FunctionKind_CastOperator,
		"unary-operator",           // FunctionKind_UnaryOperator,
		"binary-operator",          // FunctionKind_BinaryOperator,
		"operator_new",             // FunctionKind_OperatorNew,
		"operator_vararg",          // FunctionKind_OperatorVararg,
		"operator_cdecl_vararg",    // FunctionKind_OperatorCdeclVararg,
		"internal",                 // FunctionKind_Internal,
		"thunk",                    // FunctionKind_Thunk,
		"reaction",                 // FunctionKind_Reaction,
		"schedule-launcher",        // FunctionKind_ScheduleLauncher,
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
		0,                              // FunctionKind_Undefined,
		0,                              // FunctionKind_Named,
		FunctionKindFlag_NoOverloads,   // FunctionKind_Getter,
		0,                              // FunctionKind_Setter,
		0,                              // FunctionKind_Binder,
		0,                              // FunctionKind_Primer,
		FunctionKindFlag_NoStorage   |  // FunctionKind_Preconstructor,
		FunctionKindFlag_NoOverloads |
		FunctionKindFlag_NoArgs,
		FunctionKindFlag_NoStorage,     // FunctionKind_Constructor,
		FunctionKindFlag_NoStorage   |  // FunctionKind_Destructor,
		FunctionKindFlag_NoOverloads |
		FunctionKindFlag_NoArgs,
		FunctionKindFlag_NoStorage   |  // FunctionKind_StaticConstructor,
		FunctionKindFlag_NoOverloads |
		FunctionKindFlag_NoArgs,
		FunctionKindFlag_NoStorage   |  // FunctionKind_StaticDestructor,
		FunctionKindFlag_NoOverloads |
		FunctionKindFlag_NoArgs,
		FunctionKindFlag_NoStorage   |  // FunctionKind_ModuleConstructor,
		FunctionKindFlag_NoOverloads |
		FunctionKindFlag_NoArgs,
		FunctionKindFlag_NoStorage   |  // FunctionKind_ModuleDestructor,
		FunctionKindFlag_NoOverloads |
		FunctionKindFlag_NoArgs,
		0,                              // FunctionKind_CallOperator,
		FunctionKindFlag_NoOverloads |  // FunctionKind_CastOperator,
		FunctionKindFlag_NoArgs,
		FunctionKindFlag_NoOverloads |  // FunctionKind_UnaryOperator,
		FunctionKindFlag_NoArgs,
		0,                              // FunctionKind_BinaryOperator,
		0,                              // FunctionKind_OperatorNew,
		FunctionKindFlag_NoOverloads |  // FunctionKind_OperatorVararg,
		FunctionKindFlag_NoArgs,
		FunctionKindFlag_NoOverloads |  // FunctionKind_OperatorCdeclVararg,
		FunctionKindFlag_NoArgs,
		0,                              // FunctionKind_Internal,
		0,                              // FunctionKind_Thunk,
		0,                              // FunctionKind_Reaction,
		0,                              // FunctionKind_ScheduleLauncher,
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
	m_extensionNamespace = NULL;
	m_classVTableIndex = -1;
	m_propertyVTableIndex = -1;
	m_entryBlock = NULL;
	m_scope = NULL;
	m_llvmFunction = NULL;
	m_llvmPostTlsPrologueInst = NULL;
	m_machineCode = NULL;
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
Function::convertToMemberMethod (DerivableType* parentType)
{
	ASSERT (m_typeOverload.getOverloadCount () == 1);

	m_parentNamespace = parentType;
	m_type = parentType->getMemberMethodType (m_type, m_thisArgTypeFlags);
	m_typeOverload = m_type;

	ASSERT (!m_type->getArgArray ().isEmpty ());
	m_thisArgType = m_type->getArgArray () [0]->getType ();
	m_thisType = m_thisArgType;
}

void
Function::convertToOperatorNew ()
{
	ASSERT (m_typeOverload.getOverloadCount () == 1);

	m_type = m_module->m_typeMgr.getOperatorNewType (m_type);
	m_typeOverload = m_type;
}

size_t
Function::addOverload (Function* function)
{
	size_t overloadIdx = m_typeOverload.addOverload (function->m_type);
	if (overloadIdx == -1)
		return -1;
	
	m_overloadArray.append (function);
	ASSERT (overloadIdx == m_overloadArray.getCount ());
	return overloadIdx;
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

	m_module->m_namespaceMgr.getCurrentScope ()->getUsingSet ()->append (&m_usingSet);

	// parse body

	Parser parser (m_module);
	parser.m_stage = Parser::StageKind_Pass2;

	SymbolKind startSymbol = SymbolKind_compound_stmt;

	if (m_functionKind == FunctionKind_Constructor)
	{
		startSymbol = SymbolKind_constructor_compound_stmt;

		NamespaceKind namespaceKind = m_parentNamespace->getNamespaceKind ();
		ASSERT (namespaceKind == NamespaceKind_Type || namespaceKind == NamespaceKind_Property);

		if (namespaceKind == NamespaceKind_Type)
			parser.m_constructorType = (DerivableType*) m_parentNamespace;
		else
			parser.m_constructorProperty = (Property*) m_parentNamespace;
	}
	else if (m_type->getFlags () & FunctionTypeFlag_Automaton)
	{
		startSymbol = SymbolKind_automaton_compound_stmt;
	}

	result = parser.parseTokenList (startSymbol, m_body, true);
	if (!result)
		return false;

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
