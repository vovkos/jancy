#include "pch.h"
#include "jnc_McSnapshotClassType.h"
#include "jnc_Module.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

CMcSnapshotClassType::CMcSnapshotClassType ()
{
	m_ClassTypeKind = EClassType_McSnapshot;
	m_pTargetType = NULL;
	memset (m_FieldArray, 0, sizeof (m_FieldArray));
	memset (m_MethodArray, 0, sizeof (m_MethodArray));
}

void
CMcSnapshotClassType::PrepareTypeString ()
{
	m_TypeString = m_pTargetType->GetTypeModifierString ();
	m_TypeString.AppendFormat ("mcsnapshot %s", m_pTargetType->GetTargetType ()->GetArgString ().cc ());
}

bool
CMcSnapshotClassType::CompileCallMethod ()
{
	CFunction* pFunction = m_MethodArray [EMcSnapshotMethod_Call];

	rtl::CArrayT <CFunctionArg*> ArgArray = pFunction->GetType ()->GetArgArray ();
	size_t ArgCount = ArgArray.GetCount ();

	char Buffer [256];
	rtl::CArrayT <CValue> ArgValueArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	ArgValueArray.SetCount (ArgCount);

	m_pModule->m_FunctionMgr.InternalPrologue (pFunction, ArgValueArray, ArgCount);

	rtl::CBoxListT <CValue> ArgValueList;
	for (size_t i = 1; i < ArgCount; i++)
		ArgValueList.InsertTail (ArgValueArray [i]);

	CValue CountValue;
	CValue PtrPfnValue;
	CValue PtrPfnEndValue;
	CValue PtrPfnVariable;

	CType* pPtrPfnType = m_pTargetType->GetDataPtrType_c ();
	CType* pSizeType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_SizeT);

	m_pModule->m_OperatorMgr.GetClassField (ArgValueArray [0], m_FieldArray [EMcSnapshotField_Count], NULL, &CountValue);
	m_pModule->m_OperatorMgr.GetClassField (ArgValueArray [0], m_FieldArray [EMcSnapshotField_PtrArray], NULL, &PtrPfnValue);

	m_pModule->m_LlvmIrBuilder.CreateAlloca (pPtrPfnType, "ppf", NULL, &PtrPfnVariable);
	m_pModule->m_LlvmIrBuilder.CreateLoad (PtrPfnValue, PtrPfnValue.GetType (), &PtrPfnValue);
	m_pModule->m_LlvmIrBuilder.CreateLoad (CountValue, CountValue.GetType (), &CountValue);
	m_pModule->m_LlvmIrBuilder.CreateStore (PtrPfnValue, PtrPfnVariable);
	m_pModule->m_LlvmIrBuilder.CreateGep (PtrPfnValue, CountValue, pPtrPfnType, &PtrPfnEndValue);

	CBasicBlock* pConditionBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("mccall_cond");
	CBasicBlock* pBodyBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("mccall_loop");
	CBasicBlock* pFollowBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("mccall_follow");

	m_pModule->m_ControlFlowMgr.Follow (pConditionBlock);

	CValue IdxValue;
	CValue CmpValue;
	m_pModule->m_LlvmIrBuilder.CreateLoad (PtrPfnVariable, NULL, &PtrPfnValue);
	m_pModule->m_LlvmIrBuilder.CreateGe_u (PtrPfnValue, PtrPfnEndValue, &CmpValue);
	m_pModule->m_ControlFlowMgr.ConditionalJump (CmpValue, pFollowBlock, pBodyBlock, pBodyBlock);

	CValue PfnValue;
	m_pModule->m_LlvmIrBuilder.CreateLoad (PtrPfnValue, m_pTargetType, &PfnValue);
	m_pModule->m_OperatorMgr.CallOperator (PfnValue, &ArgValueList);

	m_pModule->m_LlvmIrBuilder.CreateGep (PtrPfnValue, 1, pPtrPfnType, &PtrPfnValue);
	m_pModule->m_LlvmIrBuilder.CreateStore (PtrPfnValue, PtrPfnVariable);
	m_pModule->m_ControlFlowMgr.Jump (pConditionBlock, pFollowBlock);

	m_pModule->m_FunctionMgr.InternalEpilogue ();

	return true;
}

void
CMcSnapshotClassType::GcMark (
	CRuntime* pRuntime,
	void* _p
	)
{
	TObjHdr* pObject = (TObjHdr*) _p;
	ASSERT (pObject->m_pType == this);

	TMcSnapshot* pSnapshot = (TMcSnapshot*) (pObject + 1);
	if (!(m_pTargetType->GetFlags () & ETypeFlag_GcRoot) || !pSnapshot->m_Count)
		return;

	char* p = (char*) pSnapshot->m_pPtrArray;
	size_t Size = m_pTargetType->GetSize ();

	for (size_t i = 0; i < pSnapshot->m_Count; i++, p += Size)
		pRuntime->AddGcRoot (p, m_pTargetType);
}

//.............................................................................

} // namespace jnc {
