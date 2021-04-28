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
#include "jnc_ct_Variable.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_LeanDataPtrValidator.h"

namespace jnc {
namespace ct {

//..............................................................................

Variable::Variable()
{
	m_itemKind = ModuleItemKind_Variable;
	m_type = NULL;
	m_ptrTypeFlags = 0;
	m_stdVariable = (StdVariable)-1;
	m_scope = NULL;
	m_tlsField = NULL;
	m_staticData = NULL;
	m_llvmGlobalVariable = NULL;
	m_llvmValue = NULL;
	m_llvmPreLiftValue = NULL;
}

void
Variable::prepareLlvmValue()
{
	ASSERT(!m_llvmValue && m_storageKind == StorageKind_Tls);

	Function* function = m_module->m_functionMgr.getCurrentFunction();
	BasicBlock* prologueBlock = function->getPrologueBlock();
	BasicBlock* prevBlock = m_module->m_controlFlowMgr.setCurrentBlock(prologueBlock);

	Value ptrValue;
	m_llvmValue = m_module->m_llvmIrBuilder.createAlloca(
		m_type,
		getQualifiedName(),
		NULL,
		&ptrValue
		);

	m_module->m_controlFlowMgr.setCurrentBlock(prevBlock);
	function->addTlsVariable(this);
}

void
Variable::prepareLeanDataPtrValidator()
{
	ASSERT(!m_leanDataPtrValidator);

	Value originValue(this);
	m_leanDataPtrValidator = AXL_RC_NEW(LeanDataPtrValidator);
	m_leanDataPtrValidator->m_originValue = originValue;
	m_leanDataPtrValidator->m_rangeBeginValue = originValue;
	m_leanDataPtrValidator->m_rangeLength = m_type->getSize();
}

void
Variable::prepareStaticData()
{
	ASSERT(!m_staticData && m_storageKind == StorageKind_Static);

	llvm::GlobalVariable* llvmGlobalVariable = !m_llvmGlobalVariableName.isEmpty() ?
		m_module->getLlvmModule()->getGlobalVariable(m_llvmGlobalVariableName >> toLlvm) :
		m_llvmGlobalVariable;

	if (!llvmGlobalVariable) // optimized out
	{
		Value value((void*) NULL, m_type);
		m_module->m_constMgr.saveValue(value);
		m_staticData = value.getConstData();
		return;
	}

	llvm::ExecutionEngine* llvmExecutionEngine = m_module->getLlvmExecutionEngine();

	m_staticData = (m_module->getCompileFlags() & ModuleCompileFlag_McJit) ?
		(void*)llvmExecutionEngine->getGlobalValueAddress(llvmGlobalVariable->getName().str()) :
		(void*)llvmExecutionEngine->getPointerToGlobal(llvmGlobalVariable);
}

bool
Variable::generateDocumentation(
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
	)
{
	bool result = m_type->ensureNoImports();
	if (!result)
		return false;

	dox::Block* doxyBlock = m_module->m_doxyHost.getItemBlock(this);

	bool isMulticast = isClassType(m_type, ClassTypeKind_Multicast);
	const char* kind = isMulticast ? "event" : "variable";

	itemXml->format("<memberdef kind='%s' id='%s'", kind, doxyBlock->getRefId ().sz());

	if (m_accessKind != AccessKind_Public)
		itemXml->appendFormat(" prot='%s'", getAccessKindString(m_accessKind));

	if (m_storageKind == StorageKind_Tls)
		itemXml->append(" tls='yes'");
	else if (m_storageKind == StorageKind_Static && m_parentNamespace && m_parentNamespace->getNamespaceKind() == NamespaceKind_Type)
		itemXml->append(" static='yes'");

	if (m_ptrTypeFlags & PtrTypeFlag_Const)
		itemXml->append(" const='yes'");

	itemXml->appendFormat(">\n<name>%s</name>\n", m_name.sz());
	itemXml->append(m_type->getDoxyTypeString());

	sl::String ptrTypeFlagString = getPtrTypeFlagString(m_ptrTypeFlags & ~PtrTypeFlag_DualEvent);
	if (!ptrTypeFlagString.isEmpty())
		itemXml->appendFormat("<modifiers>%s</modifiers>\n", ptrTypeFlagString.sz());

	if (!m_initializer.isEmpty())
		itemXml->appendFormat("<initializer>= %s</initializer>\n", getInitializerString().sz());

	itemXml->append(doxyBlock->getImportString());
	itemXml->append(doxyBlock->getDescriptionString());
	itemXml->append(getDoxyLocationString());
	itemXml->append("</memberdef>\n");

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
