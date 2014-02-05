#include "pch.h"
#include "jnc_Orphan.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

COrphan::COrphan ()
{
	m_ItemKind = EModuleItem_Orphan;
	m_OrphanKind = EOrphan_Undefined;
	m_pFunctionType = NULL;
}

bool
COrphan::SetBody (rtl::CBoxListT <CToken>* pTokenList)
{
	if (!m_Body.IsEmpty ())
	{
		err::SetFormatStringError ("'%s' already has a body", m_Tag.cc ());
		return false;
	}

	m_Body.TakeOver (pTokenList);
	return true;
}

bool
COrphan::ResolveOrphan ()
{
	CModuleItem* pItem = m_pParentNamespace->FindItemTraverse (m_DeclaratorName);
	if (!pItem)
	{
		err::SetFormatStringError ("unresolved orphan '%s'", m_Tag.cc ()); // thanks a lot gcc
		return false;
	}

	switch (m_OrphanKind)
	{
	case EOrphan_Function:
		return AdoptOrphanFunction (pItem);

	case EOrphan_Reactor:
		return AdoptOrphanReactor (pItem);

	default:
		ASSERT (false);
		return true;
	}
}

CFunction*
COrphan::GetItemUnnamedMethod (CModuleItem* pItem)
{
	if (pItem->GetItemKind () == EModuleItem_Property)
	{
		CProperty* pProperty = (CProperty*) pItem;
		switch (m_FunctionKind)
		{
		case EFunction_Constructor:
			return pProperty->GetConstructor ();

		case EFunction_StaticConstructor:
			return pProperty->GetStaticConstructor ();

		case EFunction_Destructor:
			return pProperty->GetDestructor ();

		case EFunction_Getter:
			return pProperty->GetGetter ();

		case EFunction_Setter:
			return pProperty->GetSetter ();
		}
	}
	else if (
		pItem->GetItemKind () == EModuleItem_Type &&
		(((CType*) pItem)->GetTypeKindFlags () & ETypeKindFlag_Derivable))
	{
		CDerivableType* pType = (CDerivableType*) pItem;
		switch (m_FunctionKind)
		{
		case EFunction_PreConstructor:
			return pType->GetPreConstructor ();

		case EFunction_Constructor:
			return pType->GetConstructor ();

		case EFunction_StaticConstructor:
			return pType->GetStaticConstructor ();

		case EFunction_Destructor:
			return pType->GetTypeKind () == EType_Class ? ((CClassType*) pType)->GetDestructor () : NULL;

		case EFunction_UnaryOperator:
			return pType->GetUnaryOperator (m_UnOpKind);

		case EFunction_BinaryOperator:
			return pType->GetBinaryOperator (m_BinOpKind);

		case EFunction_CallOperator:
			return pType->GetCallOperator ();
		}
	}

	return NULL;
}

bool
COrphan::AdoptOrphanFunction (CModuleItem* pItem)
{
	CFunction* pOriginFunction = NULL;

	EModuleItem ItemKind = pItem->GetItemKind ();

	if (m_FunctionKind == EFunction_Named)
	{
		if (ItemKind != EModuleItem_Function)
		{
			err::SetFormatStringError ("'%s' is not a function", m_Tag.cc ());
			return false;
		}

		pOriginFunction = (CFunction*) pItem;
	}
	else
	{
		pOriginFunction = GetItemUnnamedMethod (pItem);
		if (!pOriginFunction)
		{
			err::SetFormatStringError ("'%s' has no '%s'", pItem->m_Tag.cc (), GetFunctionKindString (m_FunctionKind));
			return false;
		}
	}

	bool Result =
		m_pFunctionType->EnsureLayout () &&
		pOriginFunction->GetTypeOverload ()->EnsureLayout ();

	if (!Result)
		return false;

	pOriginFunction = pOriginFunction->FindShortOverload (m_pFunctionType);
	if (!pOriginFunction)
	{
		err::SetFormatStringError ("'%s': overload not found", m_Tag.cc ());
		return false;
	}

	if (!(pOriginFunction->m_Flags & EModuleItemFlag_User))
	{
		err::SetFormatStringError ("'%s' is a compiler-generated function", m_Tag.cc ());
		return false;
	}

	ASSERT (pOriginFunction->m_FunctionKind == m_FunctionKind);

	CopySrcPos (pOriginFunction);

	return
		CopyArgNames (pOriginFunction->GetType ()) &&
		pOriginFunction->SetBody (&m_Body) &&
		VerifyStorageKind (pOriginFunction);
}

bool
COrphan::AdoptOrphanReactor (CModuleItem* pItem)
{
	CType* pItemType = NULL;

	EModuleItem ItemKind = pItem->GetItemKind ();
	switch (ItemKind)
	{
	case EModuleItem_Variable:
		pItemType = ((CVariable*) pItem)->GetType ();
		break;

	case EModuleItem_StructField:
		pItemType = ((CStructField*) pItem)->GetType ();
		break;
	}

	if (!pItemType || !IsClassType (pItemType, EClassType_Reactor))
	{
		err::SetFormatStringError ("'%s' is not a reactor", m_Tag.cc ());
		return false;
	}

	CReactorClassType* pOriginType = (CReactorClassType*) pItemType ;
	CFunction* pOriginStart = pOriginType->GetMethod (EReactorMethod_Start);

	CopySrcPos (pOriginType);
	CopySrcPos (pOriginStart);

	return
		CopyArgNames (pOriginStart->GetType ()) &&
		pOriginType->SetBody (&m_Body) &&
		VerifyStorageKind (pOriginStart);
}

bool
COrphan::CopyArgNames (CFunctionType* pTargetFunctionType)
{
	// copy arg names and make sure orphan funciton does not override default values

	rtl::CArrayT <CFunctionArg*> DstArgArray = pTargetFunctionType->GetArgArray ();
	rtl::CArrayT <CFunctionArg*> SrcArgArray = m_pFunctionType->GetArgArray ();

	size_t ArgCount = DstArgArray.GetCount ();

	size_t iDst = 0;
	size_t iSrc = 0;

	if (pTargetFunctionType->IsMemberMethodType ())
		iDst++;

	for (; iDst < ArgCount; iDst++, iSrc++)
	{
		CFunctionArg* pDstArg = DstArgArray [iDst];
		CFunctionArg* pSrcArg = SrcArgArray [iSrc];

		if (!pSrcArg->m_Initializer.IsEmpty ())
		{
			err::SetFormatStringError ("redefinition of default value for '%s'", pSrcArg->m_Name.cc ());
			return false;
		}

		pDstArg->m_Name = pSrcArg->m_Name;
		pDstArg->m_QualifiedName = pSrcArg->m_QualifiedName;
		pDstArg->m_Tag = pSrcArg->m_Tag;
	}

	return true;
}

bool
COrphan::VerifyStorageKind (CModuleItemDecl* pTargetDecl)
{
	if (!m_StorageKind || m_StorageKind == pTargetDecl->GetStorageKind ())
		return true;

	err::SetFormatStringError ("storage specifier mismatch for orphan '%s'", m_Tag.cc ());
	return false;
}

void
COrphan::CopySrcPos (CModuleItemDecl* pTargetDecl)
{
	pTargetDecl->m_pParentUnit = m_pParentUnit;
	pTargetDecl->m_Pos = m_Pos;
}

//.............................................................................

} // namespace jnc {
