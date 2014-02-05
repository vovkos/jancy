#include "pch.h"
#include "jnc_FunctionType.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CFunctionType::CFunctionType ()
{
	m_TypeKind = EType_Function;
	m_pCallConv = NULL;
	m_pReturnType = NULL;
	m_pReturnType_i = NULL;
	m_pShortType = this;
	m_pStdObjectMemberMethodType = NULL;
	m_pFunctionPtrTypeTuple = NULL;
	m_pReactorInterfaceType = NULL;
}

CNamedType*
CFunctionType::GetThisTargetType ()
{
	CType* pThisArgType = GetThisArgType ();
	if (!pThisArgType)
		return NULL;

	EType ThisArgTypeKind = pThisArgType->GetTypeKind ();
	switch (ThisArgTypeKind)
	{
	case EType_ClassPtr:
		return ((CClassPtrType*) pThisArgType)->GetTargetType ();

	case EType_DataPtr:
		return (CNamedType*) ((CDataPtrType*) pThisArgType)->GetTargetType ();

	default:
		ASSERT (false);
		return NULL;
	}
}

bool
CFunctionType::IsThrowConditionMatch (CFunctionType* pType)
{
	if (m_pReturnType->Cmp (pType->m_pReturnType) != 0 ||
		(m_Flags & EFunctionTypeFlag_Throws) != (pType->m_Flags & EFunctionTypeFlag_Throws) ||
		m_ThrowCondition.GetCount () != pType->m_ThrowCondition.GetCount ())
		return false;

	rtl::CBoxIteratorT <CToken> Token1 = m_ThrowCondition.GetHead ();
	rtl::CBoxIteratorT <CToken> Token2 = pType->m_ThrowCondition.GetHead ();

	for (; Token1 && Token2; Token1++, Token2++)
	{
		if (Token1->m_Token != Token2->m_Token)
			return false;

		#pragma AXL_TODO ("check token data also. in fact, need to come up with something smarter than token cmp")
	}

	return true;
}

rtl::CString
CFunctionType::GetArgSignature ()
{
	if (m_ArgSignature.IsEmpty ())
		m_ArgSignature = CreateArgSignature ();

	return m_ArgSignature;
}

CFunctionPtrType*
CFunctionType::GetFunctionPtrType (
	EType TypeKind,
	EFunctionPtrType PtrTypeKind,
	uint_t Flags
	)
{
	return m_pModule->m_TypeMgr.GetFunctionPtrType (this, TypeKind, PtrTypeKind, Flags);
}

CClassType*
CFunctionType::GetMulticastType ()
{
	return m_pModule->m_TypeMgr.GetMulticastType (this);
}

CFunctionType*
CFunctionType::GetMemberMethodType (
	CNamedType* pParentType,
	uint_t ThisArgTypeFlags
	)
{
	return m_pModule->m_TypeMgr.GetMemberMethodType (pParentType, this, ThisArgTypeFlags);
}

CFunctionType*
CFunctionType::GetStdObjectMemberMethodType ()
{
	return m_pModule->m_TypeMgr.GetStdObjectMemberMethodType (this);
}

CFunction*
CFunctionType::GetAbstractFunction ()
{
	if (m_pAbstractFunction)
		return m_pAbstractFunction;

	CFunction* pFunction = m_pModule->m_FunctionMgr.CreateFunction (EFunction_Internal, "abstractFunction", this);
	m_pAbstractFunction = pFunction;
	m_pModule->MarkForCompile (this);
	return pFunction;
}

bool
CFunctionType::Compile ()
{
	ASSERT (m_pAbstractFunction);

	m_pModule->m_FunctionMgr.InternalPrologue (m_pAbstractFunction);
	m_pModule->m_LlvmIrBuilder.RuntimeError (ERuntimeError_AbstractFunction);
	m_pModule->m_FunctionMgr.InternalEpilogue ();

	return true;
}

rtl::CString
CFunctionType::CreateArgSignature (
	CType* const* pArgTypeArray,
	size_t ArgCount,
	uint_t Flags
	)
{
	rtl::CString String = "(";

	for (size_t i = 0; i < ArgCount; i++)
	{
		CType* pType = pArgTypeArray [i];
		String += pType->GetSignature ();
		String += ",";
	}

	String += (Flags & EFunctionTypeFlag_VarArg) ? ".)" : ")";
	return String;
}

rtl::CString
CFunctionType::CreateArgSignature (
	CFunctionArg* const* pArgArray,
	size_t ArgCount,
	uint_t Flags
	)
{
	rtl::CString String = "(";

	for (size_t i = 0; i < ArgCount; i++)
	{
		CFunctionArg* pArg = pArgArray [i];

		String += pArg->GetType ()->GetSignature ();
		String += ",";
	}

	String += (Flags & EFunctionTypeFlag_VarArg) ? ".)" : ")";
	return String;
}

rtl::CString
CFunctionType::CreateSignature (
	CCallConv* pCallConv,
	CType* pReturnType,
	CType* const* pArgTypeArray,
	size_t ArgCount,
	uint_t Flags
	)
{
	rtl::CString String = "F";
	String += GetCallConvSignature (pCallConv->GetCallConvKind ());
	String += pReturnType->GetSignature ();
	String += CreateArgSignature (pArgTypeArray, ArgCount, Flags);
	return String;
}

rtl::CString
CFunctionType::CreateSignature (
	CCallConv* pCallConv,
	CType* pReturnType,
	CFunctionArg* const* pArgArray,
	size_t ArgCount,
	uint_t Flags
	)
{
	rtl::CString String = "F";
	String += GetCallConvSignature (pCallConv->GetCallConvKind ());
	String += pReturnType->GetSignature ();
	String += CreateArgSignature (pArgArray, ArgCount, Flags);
	return String;
}

rtl::CString
CFunctionType::GetArgString ()
{
	if (!m_ArgString.IsEmpty ())
		return m_ArgString;

	bool IsUserType = (m_Flags & EModuleItemFlag_User) != 0;

	m_ArgString = "(";

	if (!m_ArgArray.IsEmpty ())
	{
		CFunctionArg* pArg = m_ArgArray [0];
		m_ArgString.AppendFormat ("%s", pArg->GetType ()->GetTypeString ().cc ()); // thanks a lot gcc

		if (pArg->GetStorageKind () == EStorage_This)
		{
			m_ArgString += " this";
		}
		else if (IsUserType)
		{
				if (!pArg->GetName ().IsEmpty ())
					m_ArgString.AppendFormat (" %s", pArg->GetName ().cc ());

				if (!pArg->GetInitializer ().IsEmpty ())
					m_ArgString.AppendFormat (" = %s", pArg->GetInitializerString ().cc ());
		}

		size_t ArgCount = m_ArgArray.GetCount ();
		for (size_t i = 1; i < ArgCount; i++)
		{
			pArg = m_ArgArray [i];

			m_ArgString.AppendFormat (", %s", pArg->GetType ()->GetTypeString ().cc ());

			if (IsUserType)
			{
				if (!pArg->GetName ().IsEmpty ())
					m_ArgString.AppendFormat (" %s", pArg->GetName ().cc ());

				if (!pArg->GetInitializer ().IsEmpty ())
					m_ArgString.AppendFormat (" = %s", pArg->GetInitializerString ().cc ());
			}
		}

		if (m_Flags & EFunctionTypeFlag_VarArg)
			m_ArgString += ", ";
	}

	if (!(m_Flags & EFunctionTypeFlag_VarArg))
		m_ArgString += ")";
	else
		m_ArgString += "...)";

	if (m_Flags & EFunctionTypeFlag_Throws)
	{
		m_ArgString += " throws";

		if (!m_ThrowCondition.IsEmpty ())
			m_ArgString.AppendFormat (" if (%s)", CToken::GetTokenListString (m_ThrowCondition).cc ());
	}

	return m_ArgString;
}

rtl::CString
CFunctionType::GetTypeModifierString ()
{
	if (!m_TypeModifierString.IsEmpty ())
		return m_TypeModifierString;

	if (!m_pCallConv->IsDefault ())
	{
		m_TypeModifierString = m_pCallConv->GetCallConvDisplayString ();
		m_TypeModifierString += ' ';
	}

	return m_TypeModifierString;
}

void
CFunctionType::PrepareTypeString ()
{
	m_TypeString = GetTypeModifierString ();
	m_TypeString += m_pReturnType->GetTypeString ();
	m_TypeString += ' ';
	m_TypeString += GetArgString ();
}

void
CFunctionType::PrepareLlvmType ()
{
	m_pLlvmType = m_pCallConv->GetLlvmFunctionType (this);
}

void
CFunctionType::PrepareLlvmDiType ()
{
	m_LlvmDiType = m_pModule->m_LlvmDiBuilder.CreateSubroutineType (this);
}

bool
CFunctionType::CalcLayout ()
{
	bool Result;

	if (m_pReturnType_i)
	{
		m_pReturnType = m_pReturnType_i->GetActualType ();
		// TODO: check for valid return type
	}

	Result = m_pReturnType->EnsureLayout ();
	if (!Result)
		return false;

	size_t ArgCount = m_ArgArray.GetCount ();
	for (size_t i = 0; i < ArgCount; i++)
	{
		Result = m_ArgArray [i]->EnsureLayout ();
		if (!Result)
			return false;
	}

	if (m_pShortType != this)
	{
		Result = m_pShortType->EnsureLayout ();
		if (!Result)
			return false;
	}

	// update signature

	rtl::CString Signature = CreateSignature (
		m_pCallConv,
		m_pReturnType,
		m_ArgArray,
		m_ArgArray.GetCount (),
		m_Flags
		);

	m_pModule->m_TypeMgr.UpdateTypeSignature (this, Signature);
	return true;
}

//.............................................................................

} // namespace jnc {
