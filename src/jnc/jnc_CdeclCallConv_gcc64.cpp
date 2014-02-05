#include "pch.h"
#include "jnc_CdeclCallConv_gcc64.h"

namespace jnc {

//.............................................................................

llvm::FunctionType*
CCdeclCallConv_gcc64::GetLlvmFunctionType (CFunctionType* pFunctionType)
{
	return CCallConv::GetLlvmFunctionType (pFunctionType);
}

llvm::Function*
CCdeclCallConv_gcc64::CreateLlvmFunction (
	CFunctionType* pFunctionType,
	const char* pTag
	)
{
	return CCallConv::CreateLlvmFunction (pFunctionType, pTag);
}

void
CCdeclCallConv_gcc64::Call (
	const CValue& CalleeValue,
	CFunctionType* pFunctionType,
	rtl::CBoxListT <CValue>* pArgValueList,
	CValue* pResultValue
	)
{
	CCallConv::Call (CalleeValue, pFunctionType, pArgValueList, pResultValue);
}

void
CCdeclCallConv_gcc64::Return (
	CFunction* pFunction,
	const CValue& Value
	)
{
	CCallConv::Return (pFunction, Value);
}

CValue
CCdeclCallConv_gcc64::GetThisArgValue (CFunction* pFunction)
{
	return CCallConv::GetThisArgValue (pFunction);
}

void
CCdeclCallConv_gcc64::CreateArgVariables (CFunction* pFunction)
{
	CCallConv::CreateArgVariables (pFunction);
}

//.............................................................................

} // namespace jnc {
