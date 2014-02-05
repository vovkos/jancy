#include "pch.h"
#include "jnc_MulticastClassType.h"
#include "jnc_Module.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

CMulticastClassType::CMulticastClassType ()
{
	m_ClassTypeKind = EClassType_Multicast;
	m_pTargetType = NULL;
	m_pSnapshotType = NULL;
	m_pEventClassPtrTypeTuple = NULL;
	memset (m_FieldArray, 0, sizeof (m_FieldArray));
	memset (m_MethodArray, 0, sizeof (m_MethodArray));
}

void
CMulticastClassType::PrepareTypeString ()
{
	m_TypeString = m_pTargetType->GetTypeModifierString ();
	m_TypeString.AppendFormat ("multicast %s", m_pTargetType->GetTargetType ()->GetArgString ().cc ());
}

bool
CMulticastClassType::CompileCallMethod ()
{
	bool Result;

	CFunction* pFunction = m_MethodArray [EMulticastMethod_Call];

	size_t ArgCount = pFunction->GetType ()->GetArgArray ().GetCount ();

	char Buffer [256];
	rtl::CArrayT <CValue> ArgValueArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	ArgValueArray.SetCount (ArgCount);

	m_pModule->m_FunctionMgr.InternalPrologue (pFunction, ArgValueArray, ArgCount);

	CFunction* pGetSnapshot = m_MethodArray [EMulticastMethod_GetSnapshot];

	CValue SnapshotValue;
	Result = m_pModule->m_OperatorMgr.CallOperator (pGetSnapshot, ArgValueArray [0], &SnapshotValue);
	if (!Result)
		return false;

	rtl::CBoxListT <CValue> ArgList;
	for (size_t i = 1; i < ArgCount; i++)
		ArgList.InsertTail (ArgValueArray [i]);

	m_pModule->m_OperatorMgr.CallOperator (SnapshotValue, &ArgList);

	m_pModule->m_ControlFlowMgr.Return ();

	m_pModule->m_FunctionMgr.InternalEpilogue ();

	return true;
}

void
CMulticastClassType::GcMark (
	CRuntime* pRuntime,
	void* _p
	)
{
	TObjHdr* pObject = (TObjHdr*) _p;
	ASSERT (pObject->m_pType == this);

	TMulticast* pMulticast = (TMulticast*) (pObject + 1);
	if (!(m_pTargetType->GetFlags () & ETypeFlag_GcRoot) || !pMulticast->m_Count)
		return;

	char* p = (char*) pMulticast->m_pPtrArray;
	size_t Size = m_pTargetType->GetSize ();

	for (size_t i = 0; i < pMulticast->m_Count; i++, p += Size)
		pRuntime->AddGcRoot (p, m_pTargetType);
}

//.............................................................................

void
TMulticast::Call ()
{
	jnc::CFunction* pMethod = GetMethod (jnc::EMulticastMethod_Call);
	
	typedef 
	void 
	FCall (jnc::TMulticast*);

	FCall* pf = (FCall*) pMethod->GetMachineCode ();
	pf (this);
}

void
TMulticast::Call (intptr_t a)
{
	jnc::CFunction* pMethod = GetMethod (jnc::EMulticastMethod_Call);
	
	typedef 
	void 
	FCall (
		jnc::TMulticast*, 
		intptr_t
		);

	FCall* pf = (FCall*) pMethod->GetMachineCode ();
	pf (this, a);
}

void
TMulticast::Call (
	intptr_t a1,
	intptr_t a2
	)
{
	jnc::CFunction* pMethod = GetMethod (jnc::EMulticastMethod_Call);
	
	typedef 
	void 
	FCall (
		jnc::TMulticast*, 
		intptr_t,
		intptr_t
		);

	FCall* pf = (FCall*) pMethod->GetMachineCode ();
	pf (this, a1, a2);	
}

//.............................................................................
	
} // namespace jnc {
