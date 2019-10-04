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

Function::Function()
{
	m_itemKind = ModuleItemKind_Function;
	m_functionKind = FunctionKind_Normal;
	m_type = NULL;
	m_castOpType = NULL;
	m_thisArgType = NULL;
	m_thisType = NULL;
	m_thisArgDelta = 0;
	m_thisArgTypeFlags = 0;
	m_virtualOriginClassType = NULL;
	m_property = NULL;
	m_extensionNamespace = NULL;
	m_classVtableIndex = -1;
	m_allocaBlock = NULL;
	m_prologueBlock = NULL;
	m_scope = NULL;
	m_llvmFunction = NULL;
	m_machineCode = NULL;
}

void
Function::addUsingSet(Namespace* anchorNamespace)
{
	for (Namespace* nspace = anchorNamespace; nspace; nspace = nspace->getParentNamespace())
		m_usingSet.append(nspace->getUsingSet());
}

void
Function::addUsingSet(UsingSet* usingSet)
{
	m_usingSet.append(usingSet);
}

void
Function::prepareLlvmFunction()
{
	ASSERT(!m_llvmFunction);

	sl::String llvmName;
	if (m_module->getCompileFlags() & ModuleCompileFlag_McJit)
	{
		llvmName = "?"; // as to avoid linking conflicts
		llvmName += getQualifiedName();
	}
	else
	{
		llvmName = getQualifiedName();
	}

	m_llvmFunction = m_type->getCallConv()->createLlvmFunction(m_type, llvmName);

	if (canCompile())
		m_module->markForCompile(this);
	else if (m_type->getReturnType()->getTypeKind() == TypeKind_ClassPtr)
		m_module->m_typeMgr.addExternalReturnType(((ClassPtrType*)m_type->getReturnType())->getTargetType());
	else if (isDerivableTypePtrType(m_type->getReturnType()))
		m_module->m_typeMgr.addExternalReturnType((DerivableType*)((DataPtrType*)m_type->getReturnType())->getTargetType());
}

void
Function::prepareLlvmDiSubprogram()
{
	ASSERT(!m_llvmDiSubprogram);
	m_llvmDiSubprogram = m_module->m_llvmDiBuilder.createFunction(this);
}

void
Function::convertToMemberMethod(DerivableType* parentType)
{
	ASSERT(m_typeOverload.getOverloadCount() == 1);

	m_parentNamespace = parentType;
	m_type = parentType->getMemberMethodType(m_type, m_thisArgTypeFlags);
	m_typeOverload = m_type;

	ASSERT(!m_type->getArgArray().isEmpty());
	m_thisArgType = m_type->getArgArray() [0]->getType();
	m_thisType = m_thisArgType;
}

size_t
Function::addOverload(Function* function)
{
	size_t overloadIdx = m_typeOverload.addOverload(function->m_type);
	if (overloadIdx == -1)
		return -1;

	m_overloadArray.append(function);
	ASSERT(overloadIdx == m_overloadArray.getCount());
	return overloadIdx;
}

void
Function::addTlsVariable(Variable* variable)
{
	llvm::AllocaInst* llvmAlloca = (llvm::AllocaInst*)variable->getLlvmValue();
	ASSERT(llvmAlloca && llvm::isa<llvm::AllocaInst> (*llvmAlloca));

	TlsVariable tlsVariable;
	tlsVariable.m_variable = variable;
	tlsVariable.m_llvmAlloca = llvmAlloca;
	m_tlsVariableArray.append(tlsVariable);
}

bool
Function::require()
{
	if (!canCompile())
	{
		err::setFormatStringError("required '%s' is external", getQualifiedName().sz());
		return false;
	}

	m_module->markForCompile(this);

	size_t count = m_overloadArray.getCount();
	for (size_t i = 0; i < count; i++)
	{
		Function* overload = m_overloadArray[i];
		if (!overload->canCompile())
		{
			err::setFormatStringError("required '%s' (overload #%d) is external", getQualifiedName().sz(), i);
			return false;
		}

		m_module->markForCompile(overload);
	}

	return true;
}

bool
Function::compile()
{
	ASSERT(!m_prologueBlock);
	ASSERT(!m_body.isEmpty() || !m_initializer.isEmpty()); // otherwise what are we doing here?

	bool result = m_type->ensureLayout();
	if (!result)
		return false;

	if (m_parentUnit)
		m_module->m_unitMgr.setCurrentUnit(m_parentUnit);

	if (!m_body.isEmpty())
	{
		// a function with a body

		m_module->m_functionMgr.prologue(this, m_bodyPos);
		m_module->m_namespaceMgr.getCurrentScope()->getUsingSet()->append(&m_usingSet);

		Parser parser(m_module, Parser::Mode_Compile);
		SymbolKind symbolKind = SymbolKind_compound_stmt;

		if (m_functionKind == FunctionKind_Constructor ||
			m_functionKind == FunctionKind_StaticConstructor)
		{
			NamespaceKind namespaceKind = m_parentNamespace->getNamespaceKind();
			switch (namespaceKind)
			{
			case NamespaceKind_Type:
				parser.m_constructorType = (DerivableType*)m_parentNamespace;
				symbolKind = SymbolKind_constructor_compound_stmt;
				break;

			case NamespaceKind_Property:
				parser.m_constructorProperty = (Property*)m_parentNamespace;
				symbolKind = SymbolKind_constructor_compound_stmt;
				break;
			}
		}

		return
			parser.parseBody(symbolKind, m_bodyPos, m_body) &&
			m_module->m_functionMgr.epilogue();
	}

	// redirected function

	Parser parser(m_module);
	result = parser.parseTokenList(SymbolKind_qualified_name_save_name, m_initializer);
	if (!result)
		return false;

	FindModuleItemResult findResult = m_parentNamespace->findItemTraverse(parser.m_qualifiedName);
	if (!findResult.m_result)
		return false;

	if (!findResult.m_item)
	{
		err::setFormatStringError("'%s' not found", parser.m_qualifiedName.getFullName ().sz());
		return false;
	}

	if (findResult.m_item->getItemKind() != ModuleItemKind_Function)
	{
		err::setFormatStringError("'%s' is not function", parser.m_qualifiedName.getFullName ().sz());
		return false;
	}

	Function* targetFunction = (Function*)findResult.m_item;
	if (m_functionKind < FunctionKind_StaticConstructor || m_functionKind > FunctionKind_Destructor)
	{
		Function* targetOverload = targetFunction->findOverload(m_type);
		if (targetOverload) // can re-use target function directly
		{
			llvm::Function* llvmFunction = targetOverload->getLlvmFunction();
			m_llvmFunction->replaceAllUsesWith(llvmFunction);
			m_llvmFunction = llvmFunction;
			return true;
		}
	}

	// have to make a call because of either conversion or extra prologue/epilogue actions (such as in constructor)

	size_t argCount = m_type->getArgArray().getCount();

	char buffer[256];
	sl::Array<Value> argValueArray(ref::BufKind_Stack, buffer, sizeof(buffer));
	argValueArray.setCount(argCount);

	m_module->m_functionMgr.internalPrologue(this, argValueArray, argCount);

	sl::BoxList<Value> argValueList;
	for (size_t i = 0; i < argCount; i++)
		argValueList.insertTail(argValueArray[i]);

	Value resultValue;
	result = m_module->m_operatorMgr.callOperator(targetFunction, &argValueList, &resultValue);
	if (!result)
		return false;

	if (m_type->getTypeKind() != TypeKind_Void)
	{
		result = m_module->m_controlFlowMgr.ret(resultValue);
		if (!result)
			return false;
	}

	m_module->m_functionMgr.internalEpilogue();
	return true;
}

bool
Function::generateDocumentation(
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
	)
{
	dox::Block* doxyBlock = m_module->m_doxyHost.getItemBlock(this);

	itemXml->format("<memberdef kind='function' id='%s'", doxyBlock->getRefId ().sz());

	if (m_accessKind != AccessKind_Public)
		itemXml->appendFormat(" prot='%s'", getAccessKindString(m_accessKind));

	if (m_storageKind == StorageKind_Static)
		itemXml->append(" static='yes'");

	if (isMember() && (m_thisArgTypeFlags & PtrTypeFlag_Const))
		itemXml->append(" const='yes'");

	if (isVirtual())
		itemXml->appendFormat(" virt='%s'", getStorageKindString(m_storageKind));

	itemXml->appendFormat(">\n<functionkind>%s</functionkind>\n", getFunctionKindString(m_functionKind));
	itemXml->appendFormat("<name>%s</name>\n", !m_name.isEmpty () ? m_name.sz() : getFunctionKindString(m_functionKind));

	itemXml->append(m_type->getDoxyTypeString());
	itemXml->append(doxyBlock->getImportString());
	itemXml->append(doxyBlock->getDescriptionString());
	itemXml->append(getDoxyLocationString());
	itemXml->append("</memberdef>\n");

	sl::String overloadXml;

	size_t overloadCount = m_overloadArray.getCount();
	for (size_t i = 0; i < overloadCount; i++)
	{
		Function* overload = m_overloadArray[i];
		overload->generateDocumentation(outputDir, &overloadXml, indexXml);
		itemXml->append('\n');
		itemXml->append(overloadXml);
	}

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
