#include "pch.h"
#include "jnc_ct_Variable.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

Variable::Variable ()
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
Variable::getLeanDataPtrValidator ()
{
	if (m_leanDataPtrValidator)
		return m_leanDataPtrValidator;

	Value originValue (this);
	m_leanDataPtrValidator = AXL_REF_NEW (LeanDataPtrValidator);
	m_leanDataPtrValidator->m_originValue = originValue;
	m_leanDataPtrValidator->m_rangeBeginValue = originValue;
	m_leanDataPtrValidator->m_rangeLength = m_type->getSize ();
	return m_leanDataPtrValidator;
}

void*
Variable::getStaticData ()
{
	ASSERT (m_storageKind == StorageKind_Static);
	
	if (m_staticData)
		return m_staticData;

	llvm::ExecutionEngine* llvmExecutionEngine = m_module->getLlvmExecutionEngine ();

	m_staticData = (m_module->getCompileFlags () & ModuleCompileFlag_McJit) ?
		(void*) llvmExecutionEngine->getGlobalValueAddress (m_llvmGlobalVariable->getName ()) :
		(void*) llvmExecutionEngine->getPointerToGlobal (m_llvmGlobalVariable);

	return m_staticData;
}

llvm::Value*
Variable::getLlvmValue ()
{
	if (m_llvmValue)
		return m_llvmValue;

	ASSERT (m_storageKind == StorageKind_Tls);

	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	BasicBlock* entryBlock = function->getEntryBlock ();
	BasicBlock* prevBlock = m_module->m_controlFlowMgr.setCurrentBlock (entryBlock);

	Value ptrValue;
	m_llvmValue = m_module->m_llvmIrBuilder.createAlloca (
		m_type,
		m_qualifiedName,
		NULL,
		&ptrValue
		);

	m_module->m_controlFlowMgr.setCurrentBlock (prevBlock);
	function->addTlsVariable (this);
	return m_llvmValue;
}

bool
Variable::isInitializationNeeded ()
{
	return 
		!m_constructor.isEmpty () ||
		!m_initializer.isEmpty () || 
		m_type->getTypeKind () == TypeKind_Class; // static class variable
}

bool
Variable::generateDocumentation (
	const char* outputDir,
	sl::String* itemXml,
	sl::String* indexXml
	)
{
	bool isMulticast = isClassType (m_type, ClassTypeKind_Multicast);
	const char* kind = isMulticast ? "event" : "variable";

	itemXml->format ("<memberdef kind='%s' id='%s'", kind, getDoxyBlock ()->getRefId ().cc ());

	if (m_accessKind != AccessKind_Public)
		itemXml->appendFormat (" prot='%s'", getAccessKindString (m_accessKind));

	if (m_storageKind == StorageKind_Static)
		itemXml->append (" static='yes'");
	else if (m_storageKind == StorageKind_Tls)
		itemXml->append (" tls='yes'");
	 
	if (m_ptrTypeFlags & PtrTypeFlag_Const)
		itemXml->append (" const='yes'");

	itemXml->appendFormat (">\n<name>%s</name>\n", m_name.cc ());
	itemXml->appendFormat ("<type>%s</type>\n", m_type->getDoxyBlock ()->getLinkedText ().cc ());
 
	if (isMulticast)
		((MulticastClassType*) m_type)->getFunctionType ()->generateArgDocumentation (itemXml);

	itemXml->append (getDoxyBlock ()->createDescriptionString ());
	itemXml->append (createDoxyLocationString ());

	itemXml->append ("\n</memberdef>\n");

	return true;
}

//.............................................................................

} // namespace ct
} // namespace jnc
