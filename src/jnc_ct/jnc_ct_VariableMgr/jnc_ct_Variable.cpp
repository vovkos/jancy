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

namespace jnc {
namespace ct {

//..............................................................................

Variable::Variable()
{
	m_itemKind = ModuleItemKind_Variable;
	m_type = NULL;
	m_ptrTypeFlags = 0;
	m_scope = NULL;
	m_tlsField = NULL;
	m_staticData = NULL;
	m_llvmGlobalVariable = NULL;
	m_llvmValue = NULL;
	m_llvmPreLiftValue = NULL;
}

LeanDataPtrValidator*
Variable::getLeanDataPtrValidator()
{
	if (m_leanDataPtrValidator)
		return m_leanDataPtrValidator;

	Value originValue(this);
	m_leanDataPtrValidator = AXL_REF_NEW(LeanDataPtrValidator);
	m_leanDataPtrValidator->m_originValue = originValue;
	m_leanDataPtrValidator->m_rangeBeginValue = originValue;
	m_leanDataPtrValidator->m_rangeLength = m_type->getSize();
	return m_leanDataPtrValidator;
}

void*
Variable::getStaticData()
{
	ASSERT(m_storageKind == StorageKind_Static);

	if (m_staticData)
		return m_staticData;

	llvm::ExecutionEngine* llvmExecutionEngine = m_module->getLlvmExecutionEngine();

	m_staticData = (m_module->getCompileFlags() & ModuleCompileFlag_McJit) ?
		(void*)llvmExecutionEngine->getGlobalValueAddress(m_llvmGlobalVariable->getName()) :
		(void*)llvmExecutionEngine->getPointerToGlobal(m_llvmGlobalVariable);

	return m_staticData;
}

llvm::Value*
Variable::getLlvmValue()
{
	if (m_llvmValue)
		return m_llvmValue;

	ASSERT(m_storageKind == StorageKind_Tls);

	Function* function = m_module->m_functionMgr.getCurrentFunction();
	BasicBlock* prologueBlock = function->getPrologueBlock();
	BasicBlock* prevBlock = m_module->m_controlFlowMgr.setCurrentBlock(prologueBlock);

	Value ptrValue;
	m_llvmValue = m_module->m_llvmIrBuilder.createAlloca(
		m_type,
		m_qualifiedName,
		NULL,
		&ptrValue
		);

	m_module->m_controlFlowMgr.setCurrentBlock(prevBlock);
	function->addTlsVariable(this);
	return m_llvmValue;
}

bool
Variable::isInitializationNeeded()
{
	return
		!m_constructor.isEmpty() ||
		!m_initializer.isEmpty() ||
		m_type->getTypeKind() == TypeKind_Class; // static class variable
}

bool
Variable::generateDocumentation(
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
	)
{
	DoxyBlock* doxyBlock = getDoxyBlock();

	bool isMulticast = isClassType(m_type, ClassTypeKind_Multicast);
	const char* kind = isMulticast ? "event" : "variable";

	itemXml->format("<memberdef kind='%s' id='%s'", kind, doxyBlock->getRefId ().sz ());

	if (m_accessKind != AccessKind_Public)
		itemXml->appendFormat(" prot='%s'", getAccessKindString (m_accessKind));

	if (m_storageKind == StorageKind_Static)
		itemXml->append(" static='yes'");
	else if (m_storageKind == StorageKind_Tls)
		itemXml->append(" tls='yes'");

	if (m_ptrTypeFlags & PtrTypeFlag_Const)
		itemXml->append(" const='yes'");

	itemXml->appendFormat(">\n<name>%s</name>\n", m_name.sz ());
	itemXml->append(m_type->getDoxyTypeString());

	sl::String ptrTypeFlagString = getPtrTypeFlagString(m_ptrTypeFlags & ~PtrTypeFlag_DualEvent);
	if (!ptrTypeFlagString.isEmpty())
		itemXml->appendFormat("<modifiers>%s</modifiers>\n", ptrTypeFlagString.sz ());

	if (!m_initializer.isEmpty())
		itemXml->appendFormat("<initializer>= %s</initializer>\n", getInitializerString ().sz ());

	itemXml->append(doxyBlock->getImportString());
	itemXml->append(doxyBlock->getDescriptionString());
	itemXml->append(getDoxyLocationString());
	itemXml->append("</memberdef>\n");

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
