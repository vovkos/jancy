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
#include "jnc_ct_Function.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_ClassType.h"
#include "jnc_ct_ReactorClassType.h"
#include "jnc_ct_Parser.llk.h"

namespace jnc {
namespace ct {

//..............................................................................

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
	m_reactionIndex = -1;
	m_entryBlock = NULL;
	m_scope = NULL;
	m_llvmFunction = NULL;
	m_machineCode = NULL;
}

bool
Function::setBody (sl::BoxList <Token>* tokenList)
{
	if (!m_body.isEmpty ())
	{
		err::setFormatStringError ("'%s' already has a body", m_tag.sz ());
		return false;
	}

	if (m_storageKind == StorageKind_Abstract)
	{
		err::setFormatStringError ("'%s' is abstract and hence cannot have a body", m_tag.sz ());
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

#if (_JNC_OS_POSIX)
	sl::String llvmName = "."; // as to avoid linking conflicts
	llvmName += m_tag;
#else
	sl::String llvmName = m_tag;
#endif

	m_llvmFunction = m_type->getCallConv ()->createLlvmFunction (m_type, llvmName);
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

	Unit* unit = getParentUnit ();
	if (unit)
		m_module->m_unitMgr.setCurrentUnit (unit);

	// prologue

	Token::Pos beginPos = m_body.getHead ()->m_pos;
	Token::Pos endPos = m_body.getTail ()->m_pos;

	result = m_module->m_functionMgr.prologue (this, beginPos);
	if (!result)
		return false;

	m_module->m_namespaceMgr.getCurrentScope ()->getUsingSet ()->append (&m_usingSet);

	// body

	result =
		m_functionKind == FunctionKind_Constructor ? compileConstructorBody () :
		m_functionKind == FunctionKind_Reaction ? compileReactionBody () :
		(m_type->getFlags () & FunctionTypeFlag_Automaton) ? compileAutomatonBody () :
		compileNormalBody ();

	if (!result)
		return false;

	// epilogue

	m_module->m_namespaceMgr.setSourcePos (endPos);
	return m_module->m_functionMgr.epilogue ();
}

bool
Function::compileConstructorBody ()
{
	Parser parser (m_module);
	parser.m_stage = Parser::StageKind_Pass2;

	NamespaceKind namespaceKind = m_parentNamespace->getNamespaceKind ();
	ASSERT (namespaceKind == NamespaceKind_Type || namespaceKind == NamespaceKind_Property);

	if (namespaceKind == NamespaceKind_Type)
		parser.m_constructorType = (DerivableType*) m_parentNamespace;
	else
		parser.m_constructorProperty = (Property*) m_parentNamespace;

	return parser.parseTokenList (SymbolKind_constructor_compound_stmt, m_body, true);
}

bool
Function::compileAutomatonBody ()
{
	Unit* unit = getParentUnit ();
	ASSERT (unit);

	if (m_type->getReturnType ()->getStdType () != StdType_AutomatonResult)
	{
		err::setFormatStringError ("automaton function must return 'jnc.AutomatonResult'");
		lex::pushSrcPosError (lex::SrcPos (unit->getFilePath (), *getPos ()));
		return false;
	}

	sl::Array <FunctionArg*> argArray = m_type->getArgArray ();
	size_t explicitArgCount = argArray.getCount ();
	size_t recognizerArgIdx = 0;

	if (m_type->isMemberMethodType ())
	{
		explicitArgCount--;
		recognizerArgIdx = 1;
	}

	ASSERT (recognizerArgIdx < explicitArgCount); // automaton at least has 'int state'
	Type* recognizerArgType = argArray [recognizerArgIdx]->getType ();

	if (explicitArgCount != 2 ||
		(m_type->getFlags () & FunctionTypeFlag_VarArg) ||
		(recognizerArgType->getTypeKind () != TypeKind_ClassPtr) ||
		((ClassPtrType*) recognizerArgType)->getTargetType ()->getStdType () != StdType_Recognizer)
	{
		err::setFormatStringError ("automaton function must take one argument of type 'jnc.Recognizer*'");
		lex::pushSrcPosError (lex::SrcPos (unit->getFilePath (), *getPos ()));
		return false;
	}

	ASSERT (recognizerArgIdx + 1 < argArray.getCount ());
	ASSERT (argArray [recognizerArgIdx + 1]->getType ()->getTypeKind () == TypeKind_Int);

	Parser parser (m_module);
	parser.m_stage = Parser::StageKind_Pass2;
	return parser.parseTokenList (SymbolKind_automaton_compound_stmt, m_body, true);
}

bool
Function::compileReactionBody ()
{
	bool result;

	ClassPtrType* thisArgType = (ClassPtrType*) m_type->getThisArgType ();
	ASSERT (thisArgType->getTypeKindFlags () & TypeKindFlag_ClassPtr);

	ReactorClassType* reactorType = (ReactorClassType*) thisArgType->getTargetType ();
	ASSERT (isClassType (reactorType, ClassTypeKind_Reactor));

	StructField* stateField = reactorType->getField (ReactorFieldKind_ReactionStateArray);

	Value thisValue = m_module->m_functionMgr.getThisValue ();
	ASSERT (thisValue);

	BasicBlock* followBlock = m_module->m_controlFlowMgr.createBlock ("follow_block");
	BasicBlock* returnBlock = m_module->m_controlFlowMgr.createBlock ("return_block");

	Value stateValue;
	Value stateCmpValue;

	result =
		m_module->m_operatorMgr.getField (thisValue, stateField, NULL, &stateValue) &&
		m_module->m_operatorMgr.binaryOperator (
			BinOpKind_Idx,
			&stateValue,
			Value (m_reactionIndex, m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT))
			) &&
		m_module->m_controlFlowMgr.conditionalJump (stateValue, returnBlock, followBlock, followBlock) &&
		m_module->m_operatorMgr.storeDataRef (
			stateValue,
			Value (1, m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32))
			);

	Parser parser (m_module);
	parser.m_stage = Parser::StageKind_Pass2;

	result =
		parser.parseTokenList (SymbolKind_expression, m_body) &&
		m_module->m_operatorMgr.storeDataRef (
			stateValue,
			Value ((int64_t) 0, m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32))
			);

	if (!result)
		return false;

	m_module->m_controlFlowMgr.follow (returnBlock);
	return true;
}

bool
Function::compileNormalBody ()
{
	Parser parser (m_module);
	parser.m_stage = Parser::StageKind_Pass2;

	return parser.parseTokenList (SymbolKind_compound_stmt, m_body, true);
}

bool
Function::generateDocumentation (
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
	)
{
	itemXml->format ("<memberdef kind='function' id='%s'", getDoxyBlock ()->getRefId ().sz ());

	if (m_accessKind != AccessKind_Public)
		itemXml->appendFormat (" prot='%s'", getAccessKindString (m_accessKind));

	if (m_storageKind == StorageKind_Static)
		itemXml->append (" static='yes'");

	if (isMember () && (m_thisArgTypeFlags & PtrTypeFlag_Const))
		itemXml->append (" const='yes'");

	if (isVirtual ())
		itemXml->appendFormat (" virt='%s'", getStorageKindString (m_storageKind));

	itemXml->appendFormat (">\n<functionkind>%s</functionkind>\n", getFunctionKindString (m_functionKind));
	itemXml->appendFormat ("<name>%s</name>\n", m_name.sz ());
	itemXml->append (m_type->getDoxyTypeString ());
	itemXml->append (getDoxyBlock ()->getDescriptionString ());
	itemXml->append (getDoxyLocationString ());
	itemXml->append ("</memberdef>\n");

	sl::String overloadXml;

	size_t overloadCount = m_overloadArray.getCount ();
	for (size_t i = 0; i < overloadCount; i++)
	{
		Function* overload = m_overloadArray [i];
		overload->generateDocumentation (outputDir, &overloadXml, indexXml);
		itemXml->append ('\n');
		itemXml->append (overloadXml);
	}

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
