#include "pch.h"
#include "jnc_ThunkFunction.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CThunkFunction::CThunkFunction ()
{
	m_FunctionKind = EFunction_Thunk;
	m_pTargetFunction = NULL;
}

bool 
CThunkFunction::Compile ()
{
	ASSERT (m_pTargetFunction);

	bool Result;

	rtl::CArrayT <CFunctionArg*> TargetArgArray = m_pTargetFunction->GetType ()->GetArgArray ();
	rtl::CArrayT <CFunctionArg*> ThunkArgArray = m_pType->GetArgArray ();

	size_t TargetArgCount = TargetArgArray.GetCount ();
	size_t ThunkArgCount = ThunkArgArray.GetCount ();

	char Buffer [256];
	rtl::CArrayT <CValue> ArgArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	ArgArray.SetCount (TargetArgCount);

	m_pModule->m_FunctionMgr.InternalPrologue (this);

	llvm::Function::arg_iterator LlvmArg = GetLlvmFunction ()->arg_begin();

	// skip the first thunk argument (if needed)
	
	if (ThunkArgCount != TargetArgCount)
	{
		ASSERT (ThunkArgCount == TargetArgCount + 1);
		ThunkArgArray.Remove (0); 
		LlvmArg++;
	}
		
	for (size_t i = 0; i < TargetArgCount; i++, LlvmArg++)
	{
		CValue ArgValue (LlvmArg, ThunkArgArray [i]->GetType ());
		Result = m_pModule->m_OperatorMgr.CastOperator (&ArgValue, TargetArgArray [i]->GetType ());
		if (!Result)
			return false;

		ArgArray [i] = ArgValue;
	}	

	CValue ReturnValue;
	m_pModule->m_LlvmIrBuilder.CreateCall (
		m_pTargetFunction, 
		m_pTargetFunction->GetType (),
		ArgArray,
		ArgArray.GetCount (),
		&ReturnValue
		);

	if (m_pType->GetReturnType ()->GetTypeKind () != EType_Void)
	{
		Result = m_pModule->m_ControlFlowMgr.Return (ReturnValue);
		if (!Result)
			return false;
	}

	m_pModule->m_FunctionMgr.InternalEpilogue ();
	return true;
}

//.............................................................................

} // namespace jnc {
