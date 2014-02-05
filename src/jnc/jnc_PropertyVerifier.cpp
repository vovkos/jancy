#include "pch.h"
#include "jnc_PropertyVerifier.h"

namespace jnc {

//.............................................................................

bool
CPropertyVerifier::CheckSetter (CFunctionType* pFunctionType)
{
	if (pFunctionType->GetArgArray ().IsEmpty ())
	{
		err::SetFormatStringError ("'set' must have at least one argument");
		return false;
	}

	return CheckIndexSignature (EFunction_Setter, pFunctionType);
}

bool
CPropertyVerifier::CheckIndexSignature (
	EFunction FunctionKind,
	CFunctionType* pFunctionType
	)
{
	ASSERT (FunctionKind == EFunction_Getter || FunctionKind == EFunction_Setter);
	
	rtl::CString IndexArgSignature = CreateIndexArgSignature (FunctionKind, pFunctionType);
	if (m_IndexArgSignature.IsEmpty ())
	{
		m_IndexArgSignature = IndexArgSignature;
	}
	else if (m_IndexArgSignature != IndexArgSignature)
	{
		err::SetFormatStringError ("index arguments mismatch in property accessors");
		return false;
	}

	return true;
}

rtl::CString
CPropertyVerifier::CreateIndexArgSignature (
	EFunction FunctionKind,
	CFunctionType* pFunctionType
	)
{
	ASSERT (FunctionKind == EFunction_Getter || FunctionKind == EFunction_Setter);

	// refine!!!

	if (pFunctionType->IsMemberMethodType ())
		pFunctionType = pFunctionType->GetShortType ();

	if (FunctionKind == EFunction_Getter)
		return pFunctionType->CreateArgSignature ();

	rtl::CArrayT <CFunctionArg*> ArgArray = pFunctionType->GetArgArray ();
	size_t ArgCount = ArgArray.GetCount ();
	ASSERT (ArgCount);

	return pFunctionType->CreateArgSignature (ArgArray, ArgCount - 1, 0);
}

//.............................................................................

} // namespace jnc {
