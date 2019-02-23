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
	m_allocaBlock = NULL;
	m_prologueBlock = NULL;
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

	sl::takeOver (&m_body, tokenList);
	m_module->markForCompile (this);
	return true;
}

void
Function::addUsingSet (Namespace* anchorNamespace)
{
	NamespaceMgr* importNamespaceMgr = m_module->getCompileState () < ModuleCompileState_Linked ?
		&m_module->m_namespaceMgr :
		NULL;

	for (Namespace* nspace = anchorNamespace; nspace; nspace = nspace->getParentNamespace ())
		m_usingSet.append (importNamespaceMgr, nspace->getUsingSet ());
}

void
Function::addUsingSet (UsingSet* usingSet)
{
	NamespaceMgr* importNamespaceMgr = m_module->getCompileState () < ModuleCompileState_Linked ?
		&m_module->m_namespaceMgr :
		NULL;

	m_usingSet.append (importNamespaceMgr, usingSet);
}

llvm::Function*
Function::getLlvmFunction ()
{
	if (m_llvmFunction)
		return m_llvmFunction;

	sl::String llvmName;
	if (m_module->getCompileFlags () & ModuleCompileFlag_McJit)
	{
		llvmName = "?"; // as to avoid linking conflicts
		llvmName += m_tag;
	}
	else
	{
		llvmName = m_tag;
	}

	m_llvmFunction = m_type->getCallConv ()->createLlvmFunction (m_type, llvmName);
	return m_llvmFunction;
}

llvm::DISubprogram_vn
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
	bool result;

	ASSERT (!m_body.isEmpty () || !m_initializer.isEmpty ()); // otherwise what are we doing here?
	ASSERT (!m_prologueBlock);

	if (m_parentUnit)
		m_module->m_unitMgr.setCurrentUnit (m_parentUnit);

	if (!m_body.isEmpty ())
	{
		// a function with a body

		Token::Pos beginPos = m_body.getHead ()->m_pos;
		Token::Pos endPos = m_body.getTail ()->m_pos;

		m_module->m_functionMgr.prologue (this, beginPos);
		m_module->m_namespaceMgr.getCurrentScope ()->getUsingSet ()->append (NULL, &m_usingSet);

		result =
			(m_type->getFlags () & FunctionTypeFlag_Async) ? compileAsyncLauncher () :
			m_functionKind == FunctionKind_Constructor ? compileConstructorBody () :
			compileNormalBody ();

		if (!result)
			return false;

		m_module->m_namespaceMgr.setSourcePos (endPos);
		return m_module->m_functionMgr.epilogue ();
	}

	// redirected function

	Parser parser (m_module);
	result = parser.parseTokenList (SymbolKind_qualified_name_save_name, m_initializer);
	if (!result)
		return false;

	ModuleItem* item = m_parentNamespace->findItemTraverse (parser.m_qualifiedName);
	if (!item)
	{
		err::setFormatStringError ("name '%s' is not found", parser.m_qualifiedName.getFullName ().sz ());
		return false;
	}

	if (item->getItemKind () != ModuleItemKind_Function)
	{
		err::setFormatStringError ("'%s' is not function", parser.m_qualifiedName.getFullName ().sz ());
		return false;
	}

	Function* targetFunction = (Function*) item;

	if (m_functionKind < FunctionKind_PreConstructor || m_functionKind > FunctionKind_StaticDestructor)
	{
		Function* targetOverload = targetFunction->findOverload (m_type);
		if (targetOverload)
		{
			// can re-use the same function directly

			result = targetOverload->m_llvmFunction != NULL || targetOverload->compile ();
			if (!result)
				return false;

			m_llvmFunction = targetOverload->m_llvmFunction;
			return true;
		}
	}

	// have to make a call because of either conversion or extra prologue/epilogue actions (such as in constructor)

	size_t argCount = m_type->getArgArray ().getCount ();

	char buffer [256];
	sl::Array <Value> argValueArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	argValueArray.setCount (argCount);

	m_module->m_functionMgr.internalPrologue (this, argValueArray, argCount);

	sl::BoxList <Value> argValueList;
	for (size_t i = 0; i < argCount; i++)
		argValueList.insertTail (argValueArray [i]);

	Value resultValue;
	result = m_module->m_operatorMgr.callOperator (targetFunction, &argValueList, &resultValue);
	if (!result)
		return false;

	if (m_type->getTypeKind () != TypeKind_Void)
	{
		result = m_module->m_controlFlowMgr.ret (resultValue);
		if (!result)
			return false;
	}

	m_module->m_functionMgr.internalEpilogue ();
	return true;
}

bool
Function::compileConstructorBody ()
{
	Parser parser (m_module);
	parser.m_stage = Parser::Stage_Pass2;

	NamespaceKind namespaceKind = m_parentNamespace->getNamespaceKind ();
	ASSERT (namespaceKind == NamespaceKind_Type || namespaceKind == NamespaceKind_Property);

	if (namespaceKind == NamespaceKind_Type)
		parser.m_constructorType = (DerivableType*) m_parentNamespace;
	else
		parser.m_constructorProperty = (Property*) m_parentNamespace;

	return parser.parseTokenList (SymbolKind_constructor_compound_stmt, m_body, true);
}

bool
Function::compileNormalBody ()
{
	Parser parser (m_module);
	parser.m_stage = Parser::Stage_Pass2;

	return parser.parseTokenList (SymbolKind_compound_stmt, m_body, true);
}

bool
Function::compileAsyncLauncher ()
{
	bool result;

	sl::String promiseName = m_tag + "_Promise";
	sl::String sequencerName = m_tag + "_sequencer";

	ClassType* promiseType = m_module->m_typeMgr.createClassType (promiseName, promiseName);
	promiseType->addBaseType (m_module->m_typeMgr.getStdType (StdType_Promise));

	sl::Array <Variable*> argVariableArray = m_module->m_variableMgr.getArgVariableArray ();
	size_t argCount = argVariableArray.getCount ();

	for (size_t i = 0; i < argCount; i++)
	{
		Variable* argVar = argVariableArray [i];
		promiseType->createField (argVar->getName (), argVar->getType ());
	}

	sl::Array <StructField*> argFieldArray = promiseType->getMemberFieldArray ();
	ASSERT (argFieldArray.getCount () == argCount);

	promiseType->ensureLayout ();

	Value promiseValue;
	result = m_module->m_operatorMgr.newOperator (promiseType, &promiseValue);
	if (!result)
		return false;

	ASSERT (argFieldArray.getCount () == argVariableArray.getCount ());
	for (size_t i = 0; i < argCount; i++)
	{
		Variable* argVar = argVariableArray [i];
		StructField* argField = argFieldArray [i];

		Value argFieldValue;

		result =
			m_module->m_operatorMgr.getField (promiseValue, argField, &argFieldValue) &&
			m_module->m_operatorMgr.storeDataRef (argFieldValue, argVar);

		if (!result)
			return false;
	}

	Type* argType = promiseType->getClassPtrType (ClassPtrTypeKind_Normal, PtrTypeFlag_Safe);
	FunctionType* functionType = m_module->m_typeMgr.getFunctionType (&argType, 1);

	Function* sequencerFunc = m_module->m_functionMgr.createFunction (
		FunctionKind_Async,
		sequencerName,
		functionType
		);

	sequencerFunc->m_asyncLauncher = this;
	sequencerFunc->m_parentUnit = m_parentUnit;
	sequencerFunc->m_parentNamespace = m_parentNamespace;
	sequencerFunc->setBody (&m_body);

	m_module->m_operatorMgr.callOperator (sequencerFunc, promiseValue);

	return m_module->m_controlFlowMgr.ret (promiseValue);
}

bool
Function::generateDocumentation (
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
	)
{
	DoxyBlock* doxyBlock = getDoxyBlock ();

	itemXml->format ("<memberdef kind='function' id='%s'", doxyBlock->getRefId ().sz ());

	if (m_accessKind != AccessKind_Public)
		itemXml->appendFormat (" prot='%s'", getAccessKindString (m_accessKind));

	if (m_storageKind == StorageKind_Static)
		itemXml->append (" static='yes'");

	if (isMember () && (m_thisArgTypeFlags & PtrTypeFlag_Const))
		itemXml->append (" const='yes'");

	if (isVirtual ())
		itemXml->appendFormat (" virt='%s'", getStorageKindString (m_storageKind));

	itemXml->appendFormat (">\n<functionkind>%s</functionkind>\n", getFunctionKindString (m_functionKind));
	itemXml->appendFormat ("<name>%s</name>\n", !m_name.isEmpty () ? m_name.sz () : getFunctionKindString (m_functionKind));

	itemXml->append (m_type->getDoxyTypeString ());
	itemXml->append (doxyBlock->getImportString ());
	itemXml->append (doxyBlock->getDescriptionString ());
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
