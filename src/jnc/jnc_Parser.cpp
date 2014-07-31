#include "pch.h"
#include "jnc_Parser.llk.h"
#include "jnc_Closure.h"
#include "jnc_DeclTypeCalc.h"

namespace jnc {

//.............................................................................

CParser::CParser ()
{
	m_pModule = GetCurrentThreadModule ();
	m_Stage = EStage_Pass1;
	m_Flags = 0;
	m_StructPackFactor = 8;
	m_DefaultStructPackFactor = 8;
	m_StorageKind = EStorage_Undefined;
	m_AccessKind = EAccess_Undefined;
	m_pAttributeBlock = NULL;
	m_pLastDeclaredItem = NULL;
	m_pLastPropertyGetterType = NULL;
	m_pReactorType = NULL;
	m_ReactorBindableTypeCount = 0;
	m_ReactorBindSiteTotalCount = 0;
	m_pConstructorType = NULL;
	m_pConstructorProperty = NULL;
}

bool
CParser::ParseTokenList (
	ESymbol Symbol,
	const rtl::CConstBoxListT <CToken>& TokenList,
	bool IsBuildingAst
	)
{
	CUnit* pUnit = m_pModule->m_UnitMgr.GetCurrentUnit ();
	ASSERT (pUnit);

	ASSERT (!TokenList.IsEmpty ());

	bool Result;

	Create (Symbol, IsBuildingAst);

	rtl::CBoxIteratorT <CToken> Token = TokenList.GetHead ();
	for (; Token; Token++)
	{
		Result = ParseToken (&*Token);
		if (!Result)
		{
			err::EnsureSrcPosError (pUnit->GetFilePath (), Token->m_Pos.m_Line, Token->m_Pos.m_Col);
			return false;
		}
	}

	// important: process EOF token, it might actually trigger actions!

	CToken::CPos Pos = TokenList.GetTail ()->m_Pos;

	CToken EofToken;
	EofToken.m_Token = 0;
	EofToken.m_Pos = Pos;
	EofToken.m_Pos.m_p += Pos.m_Length;
	EofToken.m_Pos.m_Offset += Pos.m_Length;
	EofToken.m_Pos.m_Col += Pos.m_Length;
	EofToken.m_Pos.m_Length = 0;

	Result = ParseToken (&EofToken);
	if (!Result)
	{
		err::EnsureSrcPosError (pUnit->GetFilePath (), EofToken.m_Pos.m_Line, EofToken.m_Pos.m_Col);
		return false;
	}

	return true;
}

bool
CParser::PreCreateLandingPads (uint_t Flags)
{
	if (!Flags)
		return true;

	CScope* pScope = m_pModule->m_NamespaceMgr.GetCurrentScope ();

	if (Flags & ELandingPadFlag_Catch)
	{
		ASSERT (!pScope->m_pCatchBlock);
		pScope->m_pCatchBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("catch_block");
		pScope->m_Flags |= EScopeFlag_CanThrow;
	}

	if (Flags & ELandingPadFlag_Finally)
	{
		ASSERT (!pScope->m_pFinallyBlock);
		pScope->m_pFinallyBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("finally_block");
		pScope->m_Flags |= EScopeFlag_HasFinally;

		rtl::CString Name = "finally_return_addr";
		CType* pType = GetSimpleType (m_pModule, EType_Int);
		CVariable* pVariable  = m_pModule->m_VariableMgr.CreateStackVariable (Name, pType);
		pVariable->m_pScope = pScope;
		pScope->m_pFinallyReturnAddress = pVariable;

		bool Result = m_pModule->m_VariableMgr.AllocatePrimeInitializeVariable (pVariable);
		if (!Result)
			return false;
	}

	return true;
}

bool
CParser::IsTypeSpecified ()
{
	if (m_TypeSpecifierStack.IsEmpty ())
		return false;

	// if we've seen 'unsigned', assume 'int' is implied.
	// checking for 'property' is required for full property syntax e.g.:
	// property foo { int get (); }
	// here 'foo' should be a declarator, not import-type-specifier

	CTypeSpecifier* pTypeSpecifier = m_TypeSpecifierStack.GetBack ();
	return
		pTypeSpecifier->GetType () != NULL ||
		pTypeSpecifier->GetTypeModifiers () & (ETypeModifier_Unsigned | ETypeModifier_Property);
}

CNamedImportType*
CParser::GetNamedImportType (
	const CQualifiedName& Name,
	const CToken::CPos& Pos
	)
{
	CNamespace* pNamespace = m_pModule->m_NamespaceMgr.GetCurrentNamespace ();
	CNamedImportType* pType = m_pModule->m_TypeMgr.GetNamedImportType (Name, pNamespace);

	if (!pType->m_pParentUnit)
	{
		pType->m_pParentUnit = m_pModule->m_UnitMgr.GetCurrentUnit ();
		pType->m_Pos = Pos;
	}

	return pType;
}

CType*
CParser::FindType (
	size_t BaseTypeIdx,
	const CQualifiedName& Name,
	const CToken::CPos& Pos
	)
{
	CNamespace* pNamespace = m_pModule->m_NamespaceMgr.GetCurrentNamespace ();

	CModuleItem* pItem;

	if (m_Stage == EStage_Pass1)
	{
		if (BaseTypeIdx != -1)
			return NULL;

		if (!Name.IsSimple ())
			return GetNamedImportType (Name, Pos);

		rtl::CString ShortName = Name.GetShortName ();
		pItem = pNamespace->FindItem (ShortName);
		if (!pItem)
			return GetNamedImportType (Name, Pos);
	}
	else
	{
		if (BaseTypeIdx != -1)
		{
			CDerivableType* pBaseType = FindBaseType (BaseTypeIdx);
			if (!pBaseType)
				return NULL;

			pNamespace = pBaseType;

			if (Name.IsEmpty ())
				return pBaseType;
		}

		pItem = pNamespace->FindItemTraverse (Name);
		if (!pItem)
			return NULL;
	}

	EModuleItem ItemKind = pItem->GetItemKind ();
	switch (ItemKind)
	{
	case EModuleItem_Type:
		return (CType*) pItem;

	case EModuleItem_Typedef:
		return ((CTypedef*) pItem)->GetType ();

	default:
		return NULL;
	}
}

CType*
CParser::GetType (
	size_t BaseTypeIdx,
	const CQualifiedName& Name,
	const CToken::CPos& Pos
	)
{
	CType* pType = FindType (BaseTypeIdx, Name, Pos);
	if (!pType)
	{
		if (BaseTypeIdx == -1)
			err::SetFormatStringError ("'%s' is not found or not a type", Name.GetFullName ().cc ());
		else if (Name.IsEmpty ())
			err::SetFormatStringError ("'basetype%d' is not found", BaseTypeIdx + 1);
		else
			err::SetFormatStringError ("'basetype%d.%s' is not found or not a type", BaseTypeIdx + 1, Name.GetFullName ().cc ());

		return NULL;
	}

	return pType;
}

bool
CParser::IsEmptyDeclarationTerminatorAllowed (CTypeSpecifier* pTypeSpecifier)
{
	if (!m_pLastDeclaredItem)
	{
		ASSERT (pTypeSpecifier);
		CType* pType = pTypeSpecifier->GetType ();
		if (!pType || !(pType->GetFlags () & ETypeFlag_Named))
		{
			err::SetFormatStringError ("invalid declaration (no declarator, no named type)");
			return false;
		}

		if (pTypeSpecifier->GetTypeModifiers ())
		{
			err::SetFormatStringError ("unused modifier '%s'", GetTypeModifierString (pTypeSpecifier->GetTypeModifiers ()).cc ());
			return false;
		}

		return true;
	}

	EModuleItem ItemKind = m_pLastDeclaredItem->GetItemKind ();
	switch (ItemKind)
	{
	case EModuleItem_Property:
		return FinalizeLastProperty (false);

	case EModuleItem_Orphan:
		err::SetFormatStringError ("orphan '%s' without a body", m_pLastDeclaredItem->m_Tag.cc ());
		return false;
	}

	return true;
}

bool
CParser::SetDeclarationBody (rtl::CBoxListT <CToken>* pTokenList)
{
	if (!m_pLastDeclaredItem)
	{
		err::SetFormatStringError ("declaration without declarator cannot have a body");
		return false;
	}

	CType* pType;

	EModuleItem ItemKind = m_pLastDeclaredItem->GetItemKind ();
	switch (ItemKind)
	{
	case EModuleItem_Function:
		return ((CFunction*) m_pLastDeclaredItem)->SetBody (pTokenList);

	case EModuleItem_Property:
		return ParseLastPropertyBody (*pTokenList);

	case EModuleItem_Typedef:
		pType = ((CTypedef*) m_pLastDeclaredItem)->GetType ();
		break;

	case EModuleItem_Type:
		pType = (CType*) m_pLastDeclaredItem;
		break;

	case EModuleItem_Variable:
		pType = ((CVariable*) m_pLastDeclaredItem)->GetType ();
		break;

	case EModuleItem_StructField:
		pType = ((CStructField*) m_pLastDeclaredItem)->GetType ();
		break;

	case EModuleItem_Orphan:
		return ((COrphan*) m_pLastDeclaredItem)->SetBody (pTokenList);

	default:
		err::SetFormatStringError ("'%s' cannot have a body", GetModuleItemKindString (m_pLastDeclaredItem->GetItemKind ()));
		return false;
	}

	if (!IsClassType (pType, EClassType_Reactor))
	{
		err::SetFormatStringError ("only functions and reactors can have bodies, not '%s'", pType->GetTypeString ().cc ());
		return false;
	}

	return ((CReactorClassType*) pType)->SetBody (pTokenList);
}

bool
CParser::SetStorageKind (EStorage StorageKind)
{
	if (m_StorageKind)
	{
		err::SetFormatStringError (
			"more than one storage specifier specifiers ('%s' and '%s')",
			GetStorageKindString (m_StorageKind),
			GetStorageKindString (StorageKind)
			);
		return false;
	}

	m_StorageKind = StorageKind;
	return true;
}

bool
CParser::SetAccessKind (EAccess AccessKind)
{
	if (m_AccessKind)
	{
		err::SetFormatStringError (
			"more than one access specifiers ('%s' and '%s')",
			GetAccessKindString (m_AccessKind),
			GetAccessKindString (AccessKind)
			);
		return false;
	}

	m_AccessKind = AccessKind;
	return true;
}

CGlobalNamespace*
CParser::OpenGlobalNamespace (
	const CQualifiedName& Name,
	const CToken::CPos& Pos
	)
{
	CNamespace* pCurrentNamespace = m_pModule->m_NamespaceMgr.GetCurrentNamespace ();
	if (pCurrentNamespace->GetNamespaceKind () != ENamespace_Global)
	{
		err::SetFormatStringError ("cannot open global namespace in '%s'", GetNamespaceKindString (pCurrentNamespace->GetNamespaceKind ()));
		return NULL;
	}

	CGlobalNamespace* pNamespace = GetGlobalNamespace ((CGlobalNamespace*) pCurrentNamespace, Name.GetFirstName (), Pos);
	if (!pNamespace)
		return NULL;

	if (pNamespace->GetFlags () & EGlobalNamespaceFlag_Sealed)
	{
		err::SetFormatStringError ("cannot extend sealed namespace '%s'", pNamespace->GetQualifiedName ().cc ());
		return NULL;
	}

	rtl::CBoxIteratorT <rtl::CString> It = Name.GetNameList ().GetHead ();
	for (; It; It++)
	{
		pNamespace = GetGlobalNamespace (pNamespace, *It, Pos);
		if (!pNamespace)
			return NULL;
	}

	m_pModule->m_NamespaceMgr.OpenNamespace (pNamespace);
	return pNamespace;
}

CGlobalNamespace*
CParser::GetGlobalNamespace (
	CGlobalNamespace* pParentNamespace,
	const rtl::CString& Name,
	const CToken::CPos& Pos
	)
{
	CGlobalNamespace* pNamespace;

	CModuleItem* pItem = pParentNamespace->FindItem (Name);
	if (!pItem)
	{
		pNamespace = m_pModule->m_NamespaceMgr.CreateGlobalNamespace (Name, pParentNamespace);
		pNamespace->m_Pos = Pos;
		pParentNamespace->AddItem (pNamespace);
	}
	else
	{
		if (pItem->GetItemKind () != EModuleItem_Namespace)
		{
			err::SetFormatStringError ("'%s' exists and is not a namespace", pParentNamespace->CreateQualifiedName (Name).cc ());
			return NULL;
		}

		pNamespace = (CGlobalNamespace*) pItem;
	}

	return pNamespace;
}

bool
CParser::OpenTypeExtension (
	const CQualifiedName& Name,
	const CToken::CPos& Pos
	)
{
	CType* pType = FindType (-1, Name, Pos);
	if (!pType)
	{
		err::SetFormatStringError ("'%s' is not a type", Name.GetFullName ().cc ());
		return false;
	}

	CNamedType* pNamedType;

	EType TypeKind = pType->GetTypeKind ();
	switch (TypeKind)
	{
	case EType_Enum:
	case EType_Struct:
	case EType_Union:
	case EType_Class:
		pNamedType = (CNamedType*) pType;
		break;

	case EType_NamedImport:
		err::SetFormatStringError ("extension namespaces for '%s' is not supported yet", pType->GetTypeString ().cc ());
		return false;

	default:
		err::SetFormatStringError ("'%s' could not have an extension namespace", pType->GetTypeString ().cc ());
		return false;
	}

	if (pNamedType->m_pExtensionNamespace)
	{
		m_pModule->m_NamespaceMgr.OpenNamespace (pNamedType->m_pExtensionNamespace);
		return true;
	}

	CGlobalNamespace* pNamespace = m_pModule->m_NamespaceMgr.CreateGlobalNamespace ("extension", pNamedType);
	pNamespace->m_NamespaceKind = ENamespace_TypeExtension;
	pNamespace->m_Pos = Pos;
	pNamespace->m_pParentNamespace = pNamedType;
	pNamedType->m_pExtensionNamespace = pNamespace;

	m_pModule->m_NamespaceMgr.OpenNamespace (pNamespace);
	return true;
}

bool
CParser::PreDeclare ()
{
	if (!m_pLastDeclaredItem || m_pLastDeclaredItem->GetItemKind () != EModuleItem_Property)
		return true;

	CProperty* pProperty = (CProperty*) m_pLastDeclaredItem;
	return true;
}

bool
CParser::Declare (CDeclarator* pDeclarator)
{
	m_pLastDeclaredItem = NULL;

	if ((pDeclarator->GetTypeModifiers () & ETypeModifier_Property) && m_StorageKind != EStorage_Typedef)
	{
		// too early to calctype cause maybe this property has a body
		// declare a typeless property for now

		return DeclareProperty (pDeclarator, NULL, 0);
	}

	uint_t DeclFlags;
	CType* pType = pDeclarator->CalcType (&DeclFlags);
	if (!pType)
		return false;

	EDeclarator DeclaratorKind = pDeclarator->GetDeclaratorKind ();
	uint_t PostModifiers = pDeclarator->GetPostDeclaratorModifiers ();
	EType TypeKind = pType->GetTypeKind ();

	if (PostModifiers != 0 && TypeKind != EType_Function)
	{
		err::SetFormatStringError ("unused post-declarator modifier '%s'", GetPostDeclaratorModifierString (PostModifiers).cc ());
		return false;
	}

	switch (m_StorageKind)
	{
	case EStorage_Typedef:
		return DeclareTypedef (pDeclarator, pType);

	case EStorage_Alias:
		return DeclareAlias (pDeclarator, pType, DeclFlags);

	default:
		switch (TypeKind)
		{
		case EType_Function:
			return DeclareFunction (pDeclarator, (CFunctionType*) pType);

		case EType_Property:
			return DeclareProperty (pDeclarator, (CPropertyType*) pType, DeclFlags);

		default:
			return IsClassType (pType, EClassType_ReactorIface) ?
				DeclareReactor (pDeclarator, (CClassType*) pType, DeclFlags) :
				DeclareData (pDeclarator, pType, DeclFlags);
		}
	}
}

void
CParser::AssignDeclarationAttributes (
	CModuleItem* pItem,
	const CToken::CPos& Pos
	)
{
	CModuleItemDecl* pDecl = pItem->GetItemDecl ();
	ASSERT (pDecl);

	pDecl->m_AccessKind = m_AccessKind ?
		m_AccessKind :
		m_pModule->m_NamespaceMgr.GetCurrentAccessKind ();

	// don't overwrite storage unless explicit

	if (m_StorageKind)
		pDecl->m_StorageKind = m_StorageKind;

	pDecl->m_Pos = Pos;
	pDecl->m_pParentUnit = m_pModule->m_UnitMgr.GetCurrentUnit ();
	pDecl->m_pParentNamespace = m_pModule->m_NamespaceMgr.GetCurrentNamespace ();
	pDecl->m_pAttributeBlock = m_pAttributeBlock;

	pItem->m_Flags |= EModuleItemFlag_User;

	m_pAttributeBlock = NULL;
	m_pLastDeclaredItem = pItem;
}

bool
CParser::DeclareTypedef (
	CDeclarator* pDeclarator,
	CType* pType
	)
{
	ASSERT (m_StorageKind == EStorage_Typedef);

	bool Result;

	if (!pDeclarator->IsSimple ())
	{
		err::SetFormatStringError ("invalid typedef declarator");
		return false;
	}

	CNamespace* pNamespace = m_pModule->m_NamespaceMgr.GetCurrentNamespace ();

	rtl::CString Name = pDeclarator->GetName ()->GetShortName ();
	rtl::CString QualifiedName = pNamespace->CreateQualifiedName (Name);

	CModuleItem* pItem;

	if (IsClassType (pType, EClassType_ReactorIface))
	{
		pType = m_pModule->m_TypeMgr.CreateReactorType (Name, QualifiedName, (CClassType*) pType, NULL);
		pItem = pType;
	}
	else
	{
		CTypedef* pTypedef = m_pModule->m_TypeMgr.CreateTypedef (Name, QualifiedName, pType);
		pItem = pTypedef;
	}

	if (!pItem)
		return false;

	AssignDeclarationAttributes (pItem, pDeclarator->GetPos ());

	Result = pNamespace->AddItem (pItem, pItem->GetItemDecl ());
	if (!Result)
		return false;

	return true;
}

bool
CParser::DeclareAlias (
	CDeclarator* pDeclarator,
	CType* pType,
	uint_t PtrTypeFlags
	)
{
	bool Result;

	if (!pDeclarator->m_Constructor.IsEmpty ())
	{
		err::SetFormatStringError ("alias cannot have constructor");
		return false;
	}

	if (pDeclarator->m_Initializer.IsEmpty ())
	{
		err::SetFormatStringError ("missing alias initializer");
		return false;
	}

	if (!pDeclarator->IsSimple ())
	{
		err::SetFormatStringError ("invalid alias declarator");
		return false;
	}

	CNamespace* pNamespace = m_pModule->m_NamespaceMgr.GetCurrentNamespace ();

	rtl::CString Name = pDeclarator->GetName ()->GetShortName ();
	rtl::CString QualifiedName = pNamespace->CreateQualifiedName (Name);
	rtl::CBoxListT <CToken>* pInitializer = &pDeclarator->m_Initializer;

	CAlias* pAlias = m_pModule->m_VariableMgr.CreateAlias (Name, QualifiedName, pType, pInitializer);
	AssignDeclarationAttributes (pAlias, pDeclarator->GetPos ());

	Result = pNamespace->AddItem (pAlias);
	if (!Result)
		return false;

	if (pNamespace->GetNamespaceKind () == ENamespace_Property)
	{
		CProperty* pProperty = (CProperty*) pNamespace;

		if (PtrTypeFlags & EPtrTypeFlag_Bindable)
		{
			Result = pProperty->SetOnChanged (pAlias);
			if (!Result)
				return false;
		}
		else if (PtrTypeFlags & EPtrTypeFlag_AutoGet)
		{
			Result = pProperty->SetAutoGetValue (pAlias);
			if (!Result)
				return false;
		}
	}

	return true;
}

bool
CParser::DeclareFunction (
	CDeclarator* pDeclarator,
	CFunctionType* pType
	)
{
	CNamespace* pNamespace = m_pModule->m_NamespaceMgr.GetCurrentNamespace ();
	ENamespace NamespaceKind = pNamespace->GetNamespaceKind ();
	EDeclarator DeclaratorKind = pDeclarator->GetDeclaratorKind ();
	uint_t PostModifiers = pDeclarator->GetPostDeclaratorModifiers ();
	EFunction FunctionKind = pDeclarator->GetFunctionKind ();
	bool HasArgs = !pType->GetArgArray ().IsEmpty ();

	if (DeclaratorKind == EDeclarator_UnaryBinaryOperator)
	{
		ASSERT (FunctionKind == EFunction_UnaryOperator || FunctionKind == EFunction_BinaryOperator);
		FunctionKind = HasArgs ? EFunction_BinaryOperator : EFunction_UnaryOperator;
	}

	ASSERT (FunctionKind);
	uint_t FunctionKindFlags = GetFunctionKindFlags (FunctionKind);

	if ((FunctionKindFlags & EFunctionKindFlag_NoStorage) && m_StorageKind)
	{
		err::SetFormatStringError ("'%s' cannot have storage specifier", GetFunctionKindString (FunctionKind));
		return false;
	}

	if ((FunctionKindFlags & EFunctionKindFlag_NoArgs) && HasArgs)
	{
		err::SetFormatStringError ("'%s' cannot have arguments", GetFunctionKindString (FunctionKind));
		return false;
	}

	if (!m_StorageKind)
	{
		m_StorageKind =
			FunctionKind == EFunction_StaticConstructor || FunctionKind == EFunction_StaticDestructor ? EStorage_Static :
			NamespaceKind == ENamespace_Property ? ((CProperty*) pNamespace)->GetStorageKind () : EStorage_Undefined;
	}

	if (NamespaceKind == ENamespace_PropertyTemplate)
	{
		if (m_StorageKind)
		{
			err::SetFormatStringError ("invalid storage '%s' in property template", GetStorageKindString (m_StorageKind));
			return false;
		}

		if (PostModifiers)
		{
			err::SetFormatStringError ("unused post-declarator modifier '%s'", GetPostDeclaratorModifierString (PostModifiers).cc ());
			return false;
		}

		bool Result = ((CPropertyTemplate*) pNamespace)->AddMethod (FunctionKind, pType);
		if (!Result)
			return false;

		m_pLastDeclaredItem = pType;
		return true;
	}

	CUserModuleItem* pFunctionItem;
	CFunctionName* pFunctionName;

	if (pDeclarator->IsQualified ())
	{
		COrphan* pOrphan = m_pModule->m_NamespaceMgr.CreateOrphan (EOrphan_Function, pType);
		pOrphan->m_FunctionKind = FunctionKind;
		pFunctionItem = pOrphan;
		pFunctionName = pOrphan;
	}
	else
	{
		CFunction* pFunction = m_pModule->m_FunctionMgr.CreateFunction (FunctionKind, pType);
		pFunctionItem = pFunction;
		pFunctionName = pFunction;
	}

	pFunctionName->m_DeclaratorName = *pDeclarator->GetName ();
	pFunctionItem->m_Tag = pNamespace->CreateQualifiedName (pFunctionName->m_DeclaratorName);

	AssignDeclarationAttributes (pFunctionItem, pDeclarator->GetPos ());

	if (PostModifiers & EPostDeclaratorModifier_Const)
		pFunctionName->m_ThisArgTypeFlags = EPtrTypeFlag_Const;

	switch (FunctionKind)
	{
	case EFunction_Named:
		pFunctionItem->m_Name = pDeclarator->GetName ()->GetShortName ();
		pFunctionItem->m_QualifiedName = pNamespace->CreateQualifiedName (pFunctionItem->m_Name);
		pFunctionItem->m_Tag = pFunctionItem->m_QualifiedName;
		break;

	case EFunction_UnaryOperator:
		pFunctionName->m_UnOpKind = pDeclarator->GetUnOpKind ();
		pFunctionItem->m_Tag.AppendFormat (".unary operator %s", GetUnOpKindString (pFunctionName->m_UnOpKind));
		break;

	case EFunction_BinaryOperator:
		pFunctionName->m_BinOpKind = pDeclarator->GetBinOpKind ();
		pFunctionItem->m_Tag.AppendFormat (".binary operator %s", GetBinOpKindString (pFunctionName->m_BinOpKind));
		break;

	case EFunction_CastOperator:
		pFunctionName->m_pCastOpType = pDeclarator->GetCastOpType ();
		pFunctionItem->m_Tag.AppendFormat (".cast operator %s", pFunctionName->m_pCastOpType);
		break;

	default:
		pFunctionItem->m_Tag.AppendFormat (".%s", GetFunctionKindString (FunctionKind));
	}

	if (pFunctionItem->GetItemKind () == EModuleItem_Orphan)
		return true;

	ASSERT (pFunctionItem->GetItemKind () == EModuleItem_Function);
	CFunction* pFunction = (CFunction*) pFunctionItem;

	switch (NamespaceKind)
	{
	case ENamespace_TypeExtension:
		if (pFunction->IsVirtual ())
		{
			err::SetFormatStringError ("invalid storage '%s' in type extension", GetStorageKindString (pFunction->m_StorageKind));
			return false;
		}

		break;

	case ENamespace_Type:
		EType TypeKind;
		TypeKind = ((CNamedType*) pNamespace)->GetTypeKind ();
		switch (TypeKind)
		{
		case EType_Struct:
			return ((CStructType*) pNamespace)->AddMethod (pFunction);

		case EType_Union:
			return ((CUnionType*) pNamespace)->AddMethod (pFunction);

		case EType_Class:
			return ((CClassType*) pNamespace)->AddMethod (pFunction);

		default:
			err::SetFormatStringError ("method members are not allowed in '%s'", ((CNamedType*) pNamespace)->GetTypeString ().cc ());
			return false;
		}

	case ENamespace_Property:
		return ((CProperty*) pNamespace)->AddMethod (pFunction);

	default:
		if (PostModifiers)
		{
			err::SetFormatStringError ("unused post-declarator modifier '%s'", GetPostDeclaratorModifierString (PostModifiers).cc ());
			return false;
		}

		if (m_StorageKind && m_StorageKind != EStorage_Static)
		{
			err::SetFormatStringError ("invalid storage specifier '%s' for a global function", GetStorageKindString (m_StorageKind));
			return false;
		}
	}

	if (!pNamespace->GetParentNamespace ()) // module constructor / destructor
		switch (FunctionKind)
		{
		case EFunction_Constructor:
			return pFunction->GetModule ()->SetConstructor (pFunction);

		case EFunction_Destructor:
			return pFunction->GetModule ()->SetDestructor (pFunction);
		}

	if (FunctionKind != EFunction_Named)
	{
		err::SetFormatStringError (
			"invalid '%s' at '%s' namespace",
			GetFunctionKindString (FunctionKind),
			GetNamespaceKindString (NamespaceKind)
			);
		return false;
	}

	return pNamespace->AddFunction (pFunction);
}

bool
CParser::DeclareProperty (
	CDeclarator* pDeclarator,
	CPropertyType* pType,
	uint_t Flags
	)
{
	if (!pDeclarator->IsSimple ())
	{
		err::SetFormatStringError ("invalid property declarator");
		return false;
	}

	CProperty* pProperty = CreateProperty (
		pDeclarator->GetName ()->GetShortName (),
		pDeclarator->GetPos ()
		);

	if (!pProperty)
		return false;

	if (pType)
	{
		pProperty->m_Flags |= Flags;
		return pProperty->Create (pType);
	}

	CDeclThrowSuffix* pThrowSuffix = pDeclarator->GetThrowSuffix ();
	if (pThrowSuffix)
	{
		pProperty->m_Flags |= EPropertyFlag_Throws;
		if (!pThrowSuffix->GetThrowCondition ()->IsEmpty ())
		{
			err::SetFormatStringError ("property cannot have a throw condtion");
			return false;
		}

		pDeclarator->DeleteSuffix (pThrowSuffix);
	}

	if (pDeclarator->GetBaseType ()->GetTypeKind () != EType_Void ||
		!pDeclarator->GetPointerPrefixList ().IsEmpty () ||
		!pDeclarator->GetSuffixList ().IsEmpty ())
	{
		CDeclTypeCalc TypeCalc;
		m_pLastPropertyGetterType = TypeCalc.CalcPropertyGetterType (pDeclarator);
		if (!m_pLastPropertyGetterType)
			return false;
	}
	else
	{
		m_pLastPropertyGetterType = NULL;
	}

	if (pDeclarator->GetTypeModifiers () & ETypeModifier_Const)
	{
		if (pProperty->m_Flags & EPropertyFlag_Throws)
		{
			err::SetFormatStringError ("const property cannot throw");
			return false;
		}

		pProperty->m_Flags |= EPropertyFlag_Const;
	}

	m_LastPropertyTypeModifiers.TakeOver (pDeclarator);
	return true;
}

CPropertyTemplate*
CParser::CreatePropertyTemplate ()
{
	CPropertyTemplate* pPropertyTemplate = m_pModule->m_FunctionMgr.CreatePropertyTemplate ();
	uint_t Modifiers = GetTypeSpecifier ()->ClearTypeModifiers (ETypeModifier_Property | ETypeModifier_Bindable);

	if (Modifiers & ETypeModifier_Bindable)
		pPropertyTemplate->m_TypeFlags = EPropertyTypeFlag_Bindable;

	return pPropertyTemplate;
}

CProperty*
CParser::CreateProperty (
	const rtl::CString& Name,
	const CToken::CPos& Pos
	)
{
	bool Result;

	m_pLastDeclaredItem = NULL;

	CNamespace* pNamespace = m_pModule->m_NamespaceMgr.GetCurrentNamespace ();
	ENamespace NamespaceKind = pNamespace->GetNamespaceKind ();

	if (NamespaceKind == ENamespace_PropertyTemplate)
	{
		err::SetFormatStringError ("property templates cannot have property members");
		return NULL;
	}

	rtl::CString QualifiedName = pNamespace->CreateQualifiedName (Name);
	CProperty* pProperty = m_pModule->m_FunctionMgr.CreateProperty (Name, QualifiedName);

	AssignDeclarationAttributes (pProperty, Pos);

	EType TypeKind;

	switch (NamespaceKind)
	{
	case ENamespace_Type:
		TypeKind = ((CNamedType*) pNamespace)->GetTypeKind ();
		switch (TypeKind)
		{
		case EType_Struct:
			Result = ((CStructType*) pNamespace)->AddProperty (pProperty);
			break;

		case EType_Union:
			Result = ((CUnionType*) pNamespace)->AddProperty (pProperty);
			break;

		case EType_Class:
			Result = ((CClassType*) pNamespace)->AddProperty (pProperty);
			break;

		default:
			err::SetFormatStringError ("property members are not allowed in '%s'", ((CNamedType*) pNamespace)->GetTypeString ().cc ());
			return NULL;
		}

		if (!Result)
			return NULL;

		break;

	case ENamespace_Property:
		Result = ((CProperty*) pNamespace)->AddProperty (pProperty);
		if (!Result)
			return NULL;

		break;

	default:
		if (m_StorageKind && m_StorageKind != EStorage_Static)
		{
			err::SetFormatStringError ("invalid storage specifier '%s' for a global property", GetStorageKindString (m_StorageKind));
			return NULL;
		}

		Result = pNamespace->AddItem (pProperty);
		if (!Result)
			return NULL;

		pProperty->m_StorageKind = EStorage_Static;
	}

	return pProperty;
}

bool
CParser::ParseLastPropertyBody (const rtl::CConstBoxListT <CToken>& Body)
{
	ASSERT (m_pLastDeclaredItem->GetItemKind () == EModuleItem_Property);

	bool Result;

	CProperty* pProperty = (CProperty*) m_pLastDeclaredItem;

	CParser Parser;
	Parser.m_pModule = m_pModule;
	Parser.m_Stage = CParser::EStage_Pass1;

	m_pModule->m_NamespaceMgr.OpenNamespace (pProperty);

	Result = Parser.ParseTokenList (ESymbol_named_type_block_impl, Body);
	if (!Result)
		return false;

	m_pModule->m_NamespaceMgr.CloseNamespace ();

	return FinalizeLastProperty (true);
}

bool
CParser::FinalizeLastProperty (bool HasBody)
{
	ASSERT (m_pLastDeclaredItem->GetItemKind () == EModuleItem_Property);

	bool Result;

	CProperty* pProperty = (CProperty*) m_pLastDeclaredItem;
	if (pProperty->GetType ())
		return true;

	// finalize getter

	if (!pProperty->m_pGetter)
	{
		if (!m_pLastPropertyGetterType)
		{
			err::SetFormatStringError ("incomplete property: no 'get' method or autoget field");
			return NULL;
		}

		CFunction* pGetter = m_pModule->m_FunctionMgr.CreateFunction (EFunction_Getter, m_pLastPropertyGetterType);
		pGetter->m_Flags |= EModuleItemFlag_User;

		Result = pProperty->AddMethod (pGetter);
		if (!Result)
			return false;
	}
	else if (m_pLastPropertyGetterType && m_pLastPropertyGetterType->Cmp (pProperty->m_pGetter->GetType ()) != 0)
	{
		err::SetFormatStringError ("getter type '%s' does not match property declaration", pProperty->m_pGetter->GetType ()->GetTypeString ().cc ());
		return NULL;
	}

	// finalize setter

	if (!(m_LastPropertyTypeModifiers.GetTypeModifiers () & ETypeModifier_Const) && !HasBody)
	{
		CFunctionType* pGetterType = pProperty->m_pGetter->GetType ()->GetShortType ();
		CType* pReturnType = pGetterType->GetReturnType ();
		rtl::CArrayT <CFunctionArg*> ArgArray = pGetterType->GetArgArray ();
		ArgArray.Append (pReturnType->GetSimpleFunctionArg ());

		CFunctionType* pSetterType = m_pModule->m_TypeMgr.GetFunctionType (ArgArray);
		CFunction* pSetter = m_pModule->m_FunctionMgr.CreateFunction (EFunction_Setter, pSetterType);
		pSetter->m_Flags |= EModuleItemFlag_User;

		Result = pProperty->AddMethod (pSetter);
		if (!Result)
			return false;
	}

	// finalize binder

	if (m_LastPropertyTypeModifiers.GetTypeModifiers () & ETypeModifier_Bindable)
	{
		if (!pProperty->m_pOnChanged)
		{
			Result = pProperty->CreateOnChanged ();
			if (!Result)
				return false;
		}
	}

	// finalize auto-get value

	if (m_LastPropertyTypeModifiers.GetTypeModifiers () & ETypeModifier_AutoGet)
	{
		if (!pProperty->m_pAutoGetValue)
		{
			Result = pProperty->CreateAutoGetValue (pProperty->m_pGetter->GetType ()->GetReturnType ());

			if (!Result)
				return false;
		}
	}

	uint_t TypeFlags = 0;
	if (pProperty->m_pOnChanged)
		TypeFlags |= EPropertyTypeFlag_Bindable;

	pProperty->m_pType = pProperty->m_pSetter ?
		m_pModule->m_TypeMgr.GetPropertyType (
			pProperty->m_pGetter->GetType (),
			*pProperty->m_pSetter->GetTypeOverload (),
			TypeFlags
			) :
		m_pModule->m_TypeMgr.GetPropertyType (
			pProperty->m_pGetter->GetType (),
			NULL,
			TypeFlags
			);

	if (pProperty->m_Flags & (EPropertyFlag_AutoGet | EPropertyFlag_AutoSet))
		m_pModule->MarkForCompile (pProperty);

	return true;
}

bool
CParser::DeclareReactor (
	CDeclarator* pDeclarator,
	CClassType* pType,
	uint_t PtrTypeFlags
	)
{
	bool Result;

	if (pDeclarator->GetDeclaratorKind () != EDeclarator_Name)
	{
		err::SetFormatStringError ("invalid reactor declarator");
		return false;
	}

	CNamespace* pNamespace = m_pModule->m_NamespaceMgr.GetCurrentNamespace ();
	ENamespace NamespaceKind = pNamespace->GetNamespaceKind ();

	CNamedType* pParentType = NULL;

	switch (NamespaceKind)
	{
	case ENamespace_Property:
		pParentType = ((CProperty*) pNamespace)->GetParentType ();
		break;

	case ENamespace_Type:
		pParentType = (CNamedType*) pNamespace;
		break;
	}

	if (pParentType && pParentType->GetTypeKind () != EType_Class)
	{
		err::SetFormatStringError ("'%s' cannot contain reactor members", pParentType->GetTypeString ().cc ());
		return false;
	}

	rtl::CString Name = pDeclarator->GetName ()->GetShortName ();
	rtl::CString QualifiedName = pNamespace->CreateQualifiedName (Name);

	if (pDeclarator->IsQualified ())
	{
		CFunction* pStart = pType->GetVirtualMethodArray () [0];
		ASSERT (pStart->GetName () == "start");

		COrphan* pOprhan = m_pModule->m_NamespaceMgr.CreateOrphan (EOrphan_Reactor, pStart->GetType ());
		pOprhan->m_DeclaratorName = *pDeclarator->GetName ();
		AssignDeclarationAttributes (pOprhan, pDeclarator->GetPos ());
	}
	else
	{
		pType = m_pModule->m_TypeMgr.CreateReactorType (Name, QualifiedName, (CReactorClassType*) pType, (CClassType*) pParentType);
		AssignDeclarationAttributes (pType, pDeclarator->GetPos ());
		Result = DeclareData (pDeclarator, pType, PtrTypeFlags);
		if (!Result)
			return false;
	}

	return true;
}

bool
CParser::DeclareData (
	CDeclarator* pDeclarator,
	CType* pType,
	uint_t PtrTypeFlags
	)
{
	bool Result;

	if (!pDeclarator->IsSimple ())
	{
		err::SetFormatStringError ("invalid data declarator");
		return false;
	}

	CNamespace* pNamespace = m_pModule->m_NamespaceMgr.GetCurrentNamespace ();
	ENamespace NamespaceKind = pNamespace->GetNamespaceKind ();

	switch (NamespaceKind)
	{
	case ENamespace_TypeExtension:
	case ENamespace_PropertyTemplate:
		err::SetFormatStringError ("'%s' cannot have data fields", GetNamespaceKindString (NamespaceKind));
		return false;
	}

	rtl::CString Name = pDeclarator->GetName ()->GetShortName ();
	size_t BitCount = pDeclarator->GetBitCount ();
	rtl::CBoxListT <CToken>* pConstructor = &pDeclarator->m_Constructor;
	rtl::CBoxListT <CToken>* pInitializer = &pDeclarator->m_Initializer;

	if (IsAutoSizeArrayType (pType))
	{
		if (pInitializer->IsEmpty ())
		{
			err::SetFormatStringError ("auto-size array '%s' should have initializer", pType->GetTypeString ().cc ());
			return false;
		}

		CArrayType* pArrayType = (CArrayType*) pType;
		Result = m_pModule->m_OperatorMgr.ParseAutoSizeArrayInitializer (*pInitializer, &pArrayType->m_ElementCount);
		if (!Result)
			return false;

		if (m_Stage == EStage_Pass2)
		{
			Result = pArrayType->EnsureLayout ();
			if (!Result)
				return false;
		}
	}

	CModuleItem* pDataItem = NULL;

	if (NamespaceKind != ENamespace_Property && (PtrTypeFlags & (EPtrTypeFlag_AutoGet | EPtrTypeFlag_Bindable)))
	{
		err::SetFormatStringError ("'%s' can only be used on property field", GetPtrTypeFlagString (PtrTypeFlags & (EPtrTypeFlag_AutoGet | EPtrTypeFlag_Bindable)).cc ());
		return false;
	}

	CScope* pScope = m_pModule->m_NamespaceMgr.GetCurrentScope ();
	switch (m_StorageKind)
	{
	case EStorage_Undefined:
		switch (NamespaceKind)
		{
		case ENamespace_Scope:
			m_StorageKind = pType->GetTypeKind () == EType_Class ? EStorage_Heap : EStorage_Stack;
			break;

		case ENamespace_Type:
			m_StorageKind = EStorage_Member;
			break;

		case ENamespace_Property:
			m_StorageKind = ((CProperty*) pNamespace)->GetParentType () ? EStorage_Member : EStorage_Static;
			break;

		default:
			m_StorageKind = EStorage_Static;
		}

		break;

	case EStorage_Static:
		break;

	case EStorage_Thread:
		if (!pScope && (!pConstructor->IsEmpty () || !pInitializer->IsEmpty ()))
		{
			err::SetFormatStringError ("global 'thread' variables cannot have initializers");
			return false;
		}

		break;

	case EStorage_Heap:
	case EStorage_Stack:
		if (!pScope)
		{
			err::SetFormatStringError ("can only use '%s' storage specifier for local variables", GetStorageKindString (m_StorageKind));
			return false;
		}

		break;

	case EStorage_Mutable:
		switch (NamespaceKind)
		{
		case ENamespace_Type:
			break;

		case ENamespace_Property:
			if (((CProperty*) pNamespace)->GetParentType ())
				break;

		default:
			err::SetFormatStringError ("'mutable' can only be applied to member fields");
			return false;
		}

		break;

	default:
		err::SetFormatStringError ("invalid storage specifier '%s' for variable", GetStorageKindString (m_StorageKind));
		return false;
	}

	if (NamespaceKind == ENamespace_Property)
	{
		CProperty* pProperty = (CProperty*) pNamespace;

		if (m_StorageKind == EStorage_Member)
		{
			pDataItem = pProperty->CreateField (Name, pType, BitCount, PtrTypeFlags, pConstructor, pInitializer);
			AssignDeclarationAttributes (pDataItem, pDeclarator->GetPos ());
		}
		else
		{
			CVariable* pVariable = m_pModule->m_VariableMgr.CreateVariable (
				m_StorageKind,
				Name,
				pNamespace->CreateQualifiedName (Name),
				pType,
				PtrTypeFlags,
				pConstructor,
				pInitializer
				);

			AssignDeclarationAttributes (pVariable, pDeclarator->GetPos ());

			Result = pNamespace->AddItem (pVariable);
			if (!Result)
				return false;

			pDataItem = pVariable;
		}

		if (PtrTypeFlags & EPtrTypeFlag_Bindable)
		{
			Result = pProperty->SetOnChanged (pDataItem);
			if (!Result)
				return false;
		}
		else if (PtrTypeFlags & EPtrTypeFlag_AutoGet)
		{
			Result = pProperty->SetAutoGetValue (pDataItem);
			if (!Result)
				return false;
		}

	}
	else if (m_StorageKind != EStorage_Member && m_StorageKind != EStorage_Mutable)
	{
		CVariable* pVariable = m_pModule->m_VariableMgr.CreateVariable (
			m_StorageKind,
			Name,
			pNamespace->CreateQualifiedName (Name),
			pType,
			PtrTypeFlags,
			pConstructor,
			pInitializer
			);

		AssignDeclarationAttributes (pVariable, pDeclarator->GetPos ());

		Result = pNamespace->AddItem (pVariable);
		if (!Result)
			return false;

		if (pScope)
		{
			pVariable->m_pScope = pScope;

			Result = m_pModule->m_VariableMgr.AllocatePrimeInitializeVariable (pVariable);
			if (!Result)
				return false;
		}

		if (pNamespace->GetNamespaceKind () == ENamespace_Type && 
			(!pVariable->GetConstructor ().IsEmpty () || !pVariable->GetInitializer ().IsEmpty ()))
		{
			CNamedType* pNamedType = (CNamedType*) pNamespace;
			EType NamedTypeKind = pNamedType->GetTypeKind ();

			switch (NamedTypeKind)
			{
			case EType_Class:
			case EType_Struct:
			case EType_Union:
				((CDerivableType*) pNamedType)->m_InitializedStaticFieldArray.Append (pVariable);
				break;

			default:
				err::SetFormatStringError ("field members are not allowed in '%s'", pNamedType->GetTypeString ().cc ());
				return false;
			}
		}
	}
	else
	{
		ASSERT (pNamespace->GetNamespaceKind () == ENamespace_Type);

		CNamedType* pNamedType = (CNamedType*) pNamespace;
		EType NamedTypeKind = pNamedType->GetTypeKind ();

		CStructField* pField;

		switch (NamedTypeKind)
		{
		case EType_Class:
			pField = ((CClassType*) pNamedType)->CreateField (Name, pType, BitCount, PtrTypeFlags, pConstructor, pInitializer);
			break;

		case EType_Struct:
			pField = ((CStructType*) pNamedType)->CreateField (Name, pType, BitCount, PtrTypeFlags, pConstructor, pInitializer);
			break;

		case EType_Union:
			pField = ((CUnionType*) pNamedType)->CreateField (Name, pType, BitCount, PtrTypeFlags, pConstructor, pInitializer);
			break;

		default:
			err::SetFormatStringError ("field members are not allowed in '%s'", pNamedType->GetTypeString ().cc ());
			return false;
		}

		if (!pField)
			return false;

		AssignDeclarationAttributes (pField, pDeclarator->GetPos ());
	}

	return true;
}

bool
CParser::DeclareUnnamedStructOrUnion (CDerivableType* pType)
{
	m_StorageKind = EStorage_Undefined;
	m_AccessKind = EAccess_Undefined;

	CDeclarator Declarator;
	Declarator.m_DeclaratorKind = EDeclarator_Name;
	Declarator.m_Pos = *pType->GetPos ();
	return DeclareData (&Declarator, pType, 0);
}

CFunctionArg*
CParser::CreateFormalArg (
	CDeclFunctionSuffix* pArgSuffix,
	CDeclarator* pDeclarator
	)
{
	CNamespace* pNamespace = m_pModule->m_NamespaceMgr.GetCurrentNamespace ();

	uint_t PtrTypeFlags = 0;
	CType* pType = pDeclarator->CalcType (&PtrTypeFlags);
	if (!pType)
		return NULL;

	if (m_StorageKind)
	{
		err::SetFormatStringError ("invalid storage '%s' for argument", GetStorageKindString (m_StorageKind));
		return NULL;
	}

	m_StorageKind = EStorage_Stack;

	rtl::CString Name;

	if (pDeclarator->IsSimple ())
	{
		Name = pDeclarator->GetName ()->GetShortName ();
	}
	else if (pDeclarator->GetDeclaratorKind () != EDeclarator_Undefined)
	{
		err::SetFormatStringError ("invalid formal argument declarator");
		return NULL;
	}

	rtl::CBoxListT <CToken>* pInitializer = &pDeclarator->m_Initializer;

	CFunctionArg* pArg = m_pModule->m_TypeMgr.CreateFunctionArg (Name, pType, PtrTypeFlags, pInitializer);
	AssignDeclarationAttributes (pArg, pDeclarator->GetPos ());

	pArgSuffix->m_ArgArray.Append (pArg);

	return pArg;
}

CEnumType*
CParser::CreateEnumType (
	EEnumType EnumTypeKind,
	const rtl::CString& Name,
	CType* pBaseType,
	uint_t Flags
	)
{
	CNamespace* pNamespace = m_pModule->m_NamespaceMgr.GetCurrentNamespace ();
	CEnumType* pEnumType = NULL;

	if (Name.IsEmpty ())
	{
		Flags |= EEnumTypeFlag_Exposed;
		pEnumType = m_pModule->m_TypeMgr.CreateUnnamedEnumType (EnumTypeKind, pBaseType, Flags);
	}
	else
	{
		rtl::CString QualifiedName = pNamespace->CreateQualifiedName (Name);
		pEnumType = m_pModule->m_TypeMgr.CreateEnumType (EnumTypeKind, Name, QualifiedName, pBaseType, Flags);
		if (!pEnumType)
			return NULL;

		bool Result = pNamespace->AddItem (pEnumType);
		if (!Result)
			return NULL;
	}

	AssignDeclarationAttributes (pEnumType, m_LastMatchedToken.m_Pos);
	return pEnumType;
}

CStructType*
CParser::CreateStructType (
	const rtl::CString& Name,
	rtl::CBoxListT <CType*>* pBaseTypeList,
	size_t PackFactor
	)
{
	bool Result;

	CNamespace* pNamespace = m_pModule->m_NamespaceMgr.GetCurrentNamespace ();
	CStructType* pStructType = NULL;

	if (Name.IsEmpty ())
	{
		pStructType = m_pModule->m_TypeMgr.CreateUnnamedStructType (PackFactor);
	}
	else
	{
		rtl::CString QualifiedName = pNamespace->CreateQualifiedName (Name);
		pStructType = m_pModule->m_TypeMgr.CreateStructType (Name, QualifiedName, PackFactor);
		if (!pStructType)
			return NULL;
	}

	if (pBaseTypeList)
	{
		rtl::CBoxIteratorT <CType*> BaseType = pBaseTypeList->GetHead ();
		for (; BaseType; BaseType++)
		{
			Result = pStructType->AddBaseType (*BaseType) != NULL;
			if (!Result)
				return NULL;
		}
	}

	if (!Name.IsEmpty ())
	{
		Result = pNamespace->AddItem (pStructType);
		if (!Result)
			return NULL;
	}

	AssignDeclarationAttributes (pStructType, m_LastMatchedToken.m_Pos);
	return pStructType;
}

CUnionType*
CParser::CreateUnionType (
	const rtl::CString& Name,
	size_t PackFactor
	)
{
	bool Result;

	CNamespace* pNamespace = m_pModule->m_NamespaceMgr.GetCurrentNamespace ();
	CUnionType* pUnionType = NULL;

	if (Name.IsEmpty ())
	{
		pUnionType = m_pModule->m_TypeMgr.CreateUnnamedUnionType (PackFactor);
	}
	else
	{
		rtl::CString QualifiedName = pNamespace->CreateQualifiedName (Name);
		pUnionType = m_pModule->m_TypeMgr.CreateUnionType (Name, QualifiedName, PackFactor);
		if (!pUnionType)
			return NULL;

		Result = pNamespace->AddItem (pUnionType);
		if (!Result)
			return NULL;
	}

	AssignDeclarationAttributes (pUnionType, m_LastMatchedToken.m_Pos);
	return pUnionType;
}

CClassType*
CParser::CreateClassType (
	const rtl::CString& Name,
	rtl::CBoxListT <CType*>* pBaseTypeList,
	size_t PackFactor,
	uint_t Flags
	)
{
	bool Result;

	CNamespace* pNamespace = m_pModule->m_NamespaceMgr.GetCurrentNamespace ();
	CClassType* pClassType;

	if (Name.IsEmpty ())
	{
		pClassType = m_pModule->m_TypeMgr.CreateUnnamedClassType (PackFactor, Flags);
	}
	else
	{
		rtl::CString QualifiedName = pNamespace->CreateQualifiedName (Name);
		pClassType = m_pModule->m_TypeMgr.CreateClassType (Name, QualifiedName, PackFactor, Flags);
	}

	if (pBaseTypeList)
	{
		rtl::CBoxIteratorT <CType*> BaseType = pBaseTypeList->GetHead ();
		for (; BaseType; BaseType++)
		{
			Result = pClassType->AddBaseType (*BaseType) != NULL;
			if (!Result)
				return NULL;
		}
	}

	if (!Name.IsEmpty ())
	{
		Result = pNamespace->AddItem (pClassType);
		if (!Result)
			return NULL;
	}

	AssignDeclarationAttributes (pClassType, m_LastMatchedToken.m_Pos);
	return pClassType;
}

bool
CParser::CountReactorBindableTypes ()
{
	if (!m_ReactorBindableTypeCount)
	{
		err::SetFormatStringError ("no bindable properties found");
		return false;
	}

	m_ReactorBindSiteTotalCount += m_ReactorBindableTypeCount;
	m_ReactorBindableTypeCount = 0;
	return true;
}

bool 
CParser::AddReactorBindSite (const CValue& Value)
{
	ASSERT (m_Stage == EStage_ReactorStarter);

	bool Result;

	CValue OnChangedValue;
	Result = m_pModule->m_OperatorMgr.GetPropertyOnChanged (Value, &OnChangedValue);
	if (!Result)
		return false;
	
	CType* pType = m_pModule->m_TypeMgr.GetStdType (EStdType_SimpleEventPtr);
	CVariable* pVariable = m_pModule->m_VariableMgr.CreateStackVariable ("onChanged", pType);
	
	Result = 
		m_pModule->m_VariableMgr.AllocatePrimeInitializeVariable (pVariable) &&
		m_pModule->m_OperatorMgr.StoreDataRef (pVariable, OnChangedValue);

	m_ReactorBindSiteList.InsertTail (pVariable);
	return true;
}

bool
CParser::FinalizeReactor ()
{
	ASSERT (m_pReactorType);
	ASSERT (!m_ReactionList.IsEmpty ());

	bool Result = m_pReactorType->BindHandlers (m_ReactionList);
	if (!Result)
		return false;

	m_ReactionList.Clear ();
	return true;
}

bool
CParser::FinalizeReactorOnEventDeclaration (
	rtl::CBoxListT <CValue>* pValueList,
	CDeclarator* pDeclarator
	)
{
	ASSERT (m_pReactorType);

	CDeclFunctionSuffix* pSuffix = pDeclarator->GetFunctionSuffix ();
	ASSERT (pSuffix);

	TReaction* pHandler = AXL_MEM_NEW (TReaction);
	pHandler->m_pFunction = m_pReactorType->CreateHandler (pSuffix->GetArgArray ());
	pHandler->m_BindSiteList.TakeOver (pValueList);
	m_ReactionList.InsertTail (pHandler);

	return m_pModule->m_FunctionMgr.Prologue (pHandler->m_pFunction, m_LastMatchedToken.m_Pos);
}

bool
CParser::ReactorExpressionStmt (const rtl::CConstBoxListT <CToken>& TokenList)
{
	ASSERT (m_pReactorType);
	ASSERT (!TokenList.IsEmpty ());

	bool Result;

	ASSERT (m_pReactorType);

	CParser Parser;
	Parser.m_pModule = m_pModule;
	Parser.m_Stage = EStage_ReactorStarter;
	Parser.m_pReactorType = m_pReactorType;

	Result = Parser.ParseTokenList (ESymbol_expression, TokenList);
	if (!Result)
		return false;

	if (Parser.m_ReactorBindSiteList.IsEmpty ())
	{
		err::SetFormatStringError ("no bindable properties found");
		return false;
	}

	TReaction* pHandler = AXL_MEM_NEW (TReaction);
	pHandler->m_pFunction = m_pReactorType->CreateHandler ();
	pHandler->m_BindSiteList.TakeOver (&Parser.m_ReactorBindSiteList);
	m_ReactionList.InsertTail (pHandler);

	Result = m_pModule->m_FunctionMgr.Prologue (pHandler->m_pFunction, TokenList.GetHead ()->m_Pos);
	if (!Result)
		return false;

	Parser.m_Stage = EStage_Pass2;
	Parser.m_pReactorType = NULL;

	Result = Parser.ParseTokenList (ESymbol_expression, TokenList);
	if (!Result)
		return false;

	m_pModule->m_FunctionMgr.Epilogue ();
	return true;
}

bool
CParser::CallBaseTypeMemberConstructor (
	const CQualifiedName& Name,
	rtl::CBoxListT <CValue>* pArgList
	)
{
	ASSERT (m_pConstructorType || m_pConstructorProperty);

	CNamespace* pNamespace = m_pModule->m_FunctionMgr.GetCurrentFunction ()->GetParentNamespace ();
	CModuleItem* pItem = pNamespace->FindItemTraverse (Name, NULL, ETraverse_NoExtensionNamespace);
	if (!pItem)
	{
		err::SetFormatStringError ("name '%s' is not found", Name.GetFullName ().cc ());
		return false;
	}

	CType* pType = NULL;

	EModuleItem ItemKind = pItem->GetItemKind ();
	switch (ItemKind)
	{
	case EModuleItem_Type:
		return CallBaseTypeConstructor ((CType*) pItem, pArgList);

	case EModuleItem_Typedef:
		return CallBaseTypeConstructor (((CTypedef*) pItem)->GetType (), pArgList);

	case EModuleItem_Property:
		err::SetFormatStringError ("property construction is not yet implemented");
		return false;

	case EModuleItem_StructField:
		return CallFieldConstructor ((CStructField*) pItem, pArgList);

	case EModuleItem_Variable:
		err::SetFormatStringError ("static field construction is not yet implemented");
		return false;

	default:
		err::SetFormatStringError ("'%s' cannot be used in base-type-member construct list");
		return false;
	}
}

CDerivableType*
CParser::FindBaseType (size_t BaseTypeIdx)
{
	CFunction* pFunction = m_pModule->m_FunctionMgr.GetCurrentFunction ();
	ASSERT (pFunction); // should not be called at pass

	CDerivableType* pParentType = pFunction->GetParentType ();
	if (!pParentType)
		return NULL;
	
	CBaseTypeSlot* pSlot = pParentType->GetBaseTypeByIndex (BaseTypeIdx);
	if (!pSlot)
		return NULL;

	return pSlot->GetType ();
}

CDerivableType*
CParser::GetBaseType (size_t BaseTypeIdx)
{
	CDerivableType* pType = FindBaseType (BaseTypeIdx);
	if (!pType)
	{
		err::SetFormatStringError ("'basetype%d' is not found", BaseTypeIdx + 1);
		return NULL;
	}

	return pType;
}

bool
CParser::GetBaseType (
	size_t BaseTypeIdx,
	CValue* pResultValue
	)
{
	CDerivableType* pType = GetBaseType (BaseTypeIdx);
	if (!pType)
		return false;

	pResultValue->SetNamespace (pType);
	return true;
}

bool
CParser::CallBaseTypeConstructor (
	size_t BaseTypeIdx,
	rtl::CBoxListT <CValue>* pArgList
	)
{
	ASSERT (m_pConstructorType || m_pConstructorProperty);

	if (m_pConstructorProperty)
	{
		err::SetFormatStringError ("'%s.construct' cannot have base-type constructor calls", m_pConstructorProperty->m_Tag.cc ());
		return false;
	}

	CBaseTypeSlot* pBaseTypeSlot = m_pConstructorType->GetBaseTypeByIndex (BaseTypeIdx);
	if (!pBaseTypeSlot)
		return false;

	return CallBaseTypeConstructorImpl (pBaseTypeSlot, pArgList);
}

bool
CParser::CallBaseTypeConstructor (
	CType* pType,
	rtl::CBoxListT <CValue>* pArgList
	)
{
	ASSERT (m_pConstructorType || m_pConstructorProperty);

	if (m_pConstructorProperty)
	{
		err::SetFormatStringError ("'%s.construct' cannot have base-type constructor calls", m_pConstructorProperty->m_Tag.cc ());
		return false;
	}

	CBaseTypeSlot* pBaseTypeSlot = m_pConstructorType->FindBaseType (pType);
	if (!pBaseTypeSlot)
	{
		err::SetFormatStringError (
			"'%s' is not a base type of '%s'",
			pType->GetTypeString ().cc (),
			m_pConstructorType->GetTypeString ().cc ()
			);
		return false;
	}

	return CallBaseTypeConstructorImpl (pBaseTypeSlot, pArgList);
}

bool
CParser::CallBaseTypeConstructorImpl (
	CBaseTypeSlot* pBaseTypeSlot,
	rtl::CBoxListT <CValue>* pArgList
	)
{
	CDerivableType* pType = pBaseTypeSlot->GetType ();

	if (pBaseTypeSlot->m_Flags & EModuleItemFlag_Constructed)
	{
		err::SetFormatStringError ("'%s' is already constructed", pType->GetTypeString ().cc ());
		return false;
	}

	CFunction* pConstructor = pType->GetConstructor ();
	if (!pConstructor)
	{
		err::SetFormatStringError ("'%s' has no constructor", pType->GetTypeString ().cc ());
		return false;
	}

	CValue ThisValue = m_pModule->m_FunctionMgr.GetThisValue ();
	ASSERT (ThisValue);

	pArgList->InsertHead (ThisValue);

	bool Result = m_pModule->m_OperatorMgr.CallOperator (pConstructor, pArgList);
	if (!Result)
		return false;

	pBaseTypeSlot->m_Flags |= EModuleItemFlag_Constructed;
	return true;
}

bool
CParser::CallFieldConstructor (
	CStructField* pField,
	rtl::CBoxListT <CValue>* pArgList
	)
{
	ASSERT (m_pConstructorType || m_pConstructorProperty);

	CValue ThisValue = m_pModule->m_FunctionMgr.GetThisValue ();
	ASSERT (ThisValue);

	bool Result;

	if (m_pConstructorProperty)
	{
		err::SetFormatStringError ("property field construction is not yet implemented");
		return false;
	}

	if (pField->GetParentNamespace () != m_pConstructorType)
	{
		err::SetFormatStringError (
			"'%s' is not an immediate field of '%s'",
			pField->GetName ().cc (),
			m_pConstructorType->GetTypeString ().cc ()
			);
		return false;
	}

	if (pField->GetFlags () & EModuleItemFlag_Constructed)
	{
		err::SetFormatStringError ("'%s' is already constructed", pField->GetName ().cc ());
		return false;
	}

	if (!(pField->GetType ()->GetTypeKindFlags () & ETypeKindFlag_Derivable) ||
		!((CDerivableType*) pField->GetType ())->GetConstructor ())
	{
		err::SetFormatStringError ("'%s' has no constructor", pField->GetName ().cc ());
		return false;
	}

	CFunction* pConstructor = ((CDerivableType*) pField->GetType ())->GetConstructor ();

	CValue FieldValue;
	Result =
		m_pModule->m_OperatorMgr.GetField (ThisValue, pField, NULL, &FieldValue) &&
		m_pModule->m_OperatorMgr.UnaryOperator (EUnOp_Addr, &FieldValue);

	if (!Result)
		return false;

	pArgList->InsertHead (FieldValue);

	Result = m_pModule->m_OperatorMgr.CallOperator (pConstructor, pArgList);
	if (!Result)
		return false;

	pField->m_Flags |= EModuleItemFlag_Constructed;
	return true;
}

bool
CParser::FinalizeBaseTypeMemberConstructBlock ()
{
	ASSERT (m_pConstructorType || m_pConstructorProperty);

	CValue ThisValue = m_pModule->m_FunctionMgr.GetThisValue ();

	bool Result;

	if (m_pConstructorProperty)
		return
			m_pConstructorProperty->CallMemberPropertyConstructors (ThisValue) &&
			m_pConstructorProperty->CallMemberFieldConstructors (ThisValue);

	ASSERT (ThisValue);

	Result =
		m_pConstructorType->CallBaseTypeConstructors (ThisValue) &&
		m_pConstructorType->CallMemberPropertyConstructors (ThisValue) &&
		m_pConstructorType->CallMemberFieldConstructors (ThisValue);

	if (!Result)
		return false;

	CFunction* pPreConstructor = m_pConstructorType->GetPreConstructor ();
	if (!pPreConstructor)
		return true;

	return m_pModule->m_OperatorMgr.CallOperator (pPreConstructor, ThisValue);
}

bool
CParser::NewOperator_0 (
	EStorage StorageKind,
	CType* pType,
	CValue* pResultValue
	)
{
	pResultValue->SetType (m_pModule->m_OperatorMgr.GetNewOperatorResultType (pType));
	return true;
}

bool
CParser::LookupIdentifier (
	const rtl::CString& Name,
	const CToken::CPos& Pos,
	CValue* pValue
	)
{
	bool Result;

	CNamespace* pNamespace = m_pModule->m_NamespaceMgr.GetCurrentNamespace ();
	CModuleItem* pItem = NULL;

	CMemberCoord Coord;
	pItem = pNamespace->FindItemTraverse (Name, &Coord);
	if (!pItem)
	{
		err::SetFormatStringError ("undeclared identifier '%s'", Name.cc ());
		err::PushSrcPosError (m_pModule->m_UnitMgr.GetCurrentUnit ()->GetFilePath (), Pos);
		return false;
	}

	CValue ThisValue;

	EModuleItem ItemKind = pItem->GetItemKind ();
	switch (ItemKind)
	{
	case EModuleItem_Namespace:
		pValue->SetNamespace ((CGlobalNamespace*) pItem);
		break;

	case EModuleItem_Typedef:
		pItem = ((CTypedef*) pItem)->GetType ();
		// and fall through

	case EModuleItem_Type:
		if (!(((CType*) pItem)->GetTypeKindFlags () & ETypeKindFlag_Named))
		{
			err::SetFormatStringError ("'%s' cannot be used as expression", ((CType*) pItem)->GetTypeString ().cc ());
			return false;
		}

		pValue->SetNamespace ((CNamedType*) pItem);
		break;

	case EModuleItem_Alias:
		return m_pModule->m_OperatorMgr.EvaluateAlias (
			pItem->GetItemDecl (),
			((CAlias*) pItem)->GetInitializer (),
			pValue
			);

	case EModuleItem_Variable:
		if (m_Flags & EFlag_ConstExpression)
		{
			err::SetFormatStringError ("variable '%s' cannot be used in const expression", Name.cc ());
			return false;
		}

		pValue->SetVariable ((CVariable*) pItem);
		break;

	case EModuleItem_Function:
		if (m_Flags & EFlag_ConstExpression)
		{
			err::SetFormatStringError ("function '%s' cannot be used in const expression", Name.cc ());
			return false;
		}

		pValue->SetFunction ((CFunction*) pItem);

		if (((CFunction*) pItem)->IsMember ())
		{
			Result = m_pModule->m_OperatorMgr.CreateMemberClosure (pValue);
			if (!Result)
				return false;
		}

		break;

	case EModuleItem_Property:
		if (m_Flags & EFlag_ConstExpression)
		{
			err::SetFormatStringError ("property '%s' cannot be used in const expression", Name.cc ());
			return false;
		}

		pValue->SetProperty ((CProperty*) pItem);

		if (((CProperty*) pItem)->IsMember ())
		{
			Result = m_pModule->m_OperatorMgr.CreateMemberClosure (pValue);
			if (!Result)
				return false;
		}

		break;

	case EModuleItem_EnumConst:
		Result = ((CEnumConst*) pItem)->GetParentEnumType ()->EnsureLayout ();
		if (!Result)
			return false;

		pValue->SetConstInt64 (
			((CEnumConst*) pItem)->GetValue (),
			((CEnumConst*) pItem)->GetParentEnumType ()
			);
		break;

	case EModuleItem_StructField:
		if (m_Flags & EFlag_ConstExpression)
		{
			err::SetFormatStringError ("field '%s' cannot be used in const expression", Name.cc ());
			return false;
		}

		Result =
			m_pModule->m_OperatorMgr.GetThisValue (&ThisValue) &&
			m_pModule->m_OperatorMgr.GetField (ThisValue, (CStructField*) pItem, &Coord, pValue);

		if (!Result)
			return false;

		break;

	default:
		err::SetFormatStringError (
			"%s '%s' cannot be used as expression",
			GetModuleItemKindString (pItem->GetItemKind ()),
			Name.cc ()
			);
		return false;
	};

	return true;
}

bool
CParser::LookupIdentifierType (
	const rtl::CString& Name,
	const CToken::CPos& Pos,
	CValue* pValue
	)
{
	bool Result;

	CNamespace* pNamespace = m_pModule->m_NamespaceMgr.GetCurrentNamespace ();
	CModuleItem* pItem = NULL;

	pItem = pNamespace->FindItemTraverse (Name);
	if (!pItem)
	{
		err::SetFormatStringError ("undeclared identifier '%s'", Name.cc ());
		err::PushSrcPosError (m_pModule->m_UnitMgr.GetCurrentUnit ()->GetFilePath (), Pos);
		return false;
	}

	EModuleItem ItemKind = pItem->GetItemKind ();
	switch (ItemKind)
	{
	case EModuleItem_Namespace:
		pValue->SetNamespace ((CGlobalNamespace*) pItem);
		break;

	case EModuleItem_Typedef:
		pItem = ((CTypedef*) pItem)->GetType ();
		// and fall through

	case EModuleItem_Type:
		if (!(((CType*) pItem)->GetTypeKindFlags () & ETypeKindFlag_Named))
		{
			err::SetFormatStringError ("'%s' cannot be used as expression", ((CType*) pItem)->GetTypeString ().cc ());
			return false;
		}

		pValue->SetNamespace ((CNamedType*) pItem);
		break;

	case EModuleItem_Variable:
		pValue->SetType (((CVariable*) pItem)->GetType ()->GetDataPtrType (EType_DataRef, EDataPtrType_Lean));
		break;

	case EModuleItem_Alias:
		pValue->SetType (((CAlias*) pItem)->GetType ());
		break;

	case EModuleItem_Function:
		{
		CFunction* pFunction = (CFunction*) pItem;
		pValue->SetFunctionTypeOverload (pFunction->GetTypeOverload ());

		if (((CFunction*) pItem)->IsMember ())
		{
			Result = m_pModule->m_OperatorMgr.CreateMemberClosure (pValue);
			if (!Result)
				return false;
		}
		}
		break;

	case EModuleItem_Property:
		pValue->SetType (((CProperty*) pItem)->GetType ()->GetPropertyPtrType (EType_PropertyRef, EPropertyPtrType_Thin));

		if (((CProperty*) pItem)->IsMember ())
		{
			Result = m_pModule->m_OperatorMgr.CreateMemberClosure (pValue);
			if (!Result)
				return false;
		}

		break;

	case EModuleItem_EnumConst:
		pValue->SetType (((CEnumConst*) pItem)->GetParentEnumType ()->GetBaseType ());
		break;

	case EModuleItem_StructField:
		pValue->SetType (((CStructField*) pItem)->GetType ()->GetDataPtrType (EType_DataRef, EDataPtrType_Lean));
		break;

	default:
		err::SetFormatStringError ("'%s' cannot be used as expression", Name.cc ());
		return false;
	};

	return true;
}

bool
CParser::GetCountOf (
	CType* pType,
	CValue* pValue
	)
{
	if (pType->GetTypeKind () != EType_Array)
	{
		err::SetFormatStringError ("'countof' operator is only applicable to arrays, not to '%s'", pType->GetTypeString ().cc ());
		return false;
	}

	pValue->SetConstSizeT (((CArrayType*) pType)->GetElementCount ());
	return true;
}

bool
CParser::GetThrowReturnValue (CValue* pValue)
{
	if (m_ThrowReturnValue.IsEmpty ())
	{
		err::SetFormatStringError ("'retval' can only be used in throw condition");
		return false;
	}

	*pValue = m_ThrowReturnValue;
	return true;
}

bool
CParser::GetThrowReturnValueType (CValue* pValue)
{
	if (m_ThrowReturnValue.IsEmpty ())
	{
		err::SetFormatStringError ("'retval' can only be used in throw condition");
		return false;
	}

	pValue->SetType (m_ThrowReturnValue.GetType ());
	return true;
}

bool
CParser::PrepareCurlyInitializerNamedItem (
	TCurlyInitializer* pInitializer,
	const char* pName
	)
{
	CValue MemberValue;

	bool Result = m_pModule->m_OperatorMgr.MemberOperator (
		pInitializer->m_TargetValue,
		pName,
		&pInitializer->m_MemberValue
		);

	if (!Result)
		return false;

	pInitializer->m_Index = -1;
	pInitializer->m_Count++;
	m_CurlyInitializerTargetValue = pInitializer->m_MemberValue;
	return true;
}

bool
CParser::PrepareCurlyInitializerIndexedItem (TCurlyInitializer* pInitializer)
{
	if (pInitializer->m_Index == -1)
	{
		err::SetFormatStringError ("indexed-baded initializer cannot be used after named-based initializer");
		return false;
	}

	bool Result = m_pModule->m_OperatorMgr.MemberOperator (
		pInitializer->m_TargetValue,
		pInitializer->m_Index,
		&pInitializer->m_MemberValue
		);

	if (!Result)
		return false;

	pInitializer->m_Index++;
	pInitializer->m_Count++;
	m_CurlyInitializerTargetValue = pInitializer->m_MemberValue;
	return true;
}

bool
CParser::SkipCurlyInitializerItem (TCurlyInitializer* pInitializer)
{
	if (pInitializer->m_Index == -1)
	{
		err::SetFormatStringError ("indexed-baded initializer cannot be used after named-based initializer");
		return false;
	}

	pInitializer->m_Index++;
	return true;
}

bool
CParser::AppendFmtLiteral (
	TLiteral* pLiteral,
	const void* p,
	size_t Length
	)
{
	bool Result;

	if (!pLiteral->m_FmtLiteralValue)
	{
		Result = m_pModule->m_OperatorMgr.NewOperator (
			EStorage_Stack,
			GetSimpleType (m_pModule, EStdType_FmtLiteral),
			NULL,
			&pLiteral->m_FmtLiteralValue
			);

		if (!Result)
			return false;
	}

	CFunction* pAppend = m_pModule->m_FunctionMgr.GetStdFunction (EStdFunc_AppendFmtLiteral_a);

	pLiteral->m_BinData.Append ((uchar_t*) p, Length);
	Length = pLiteral->m_BinData.GetCount ();

	CValue LiteralValue;
	LiteralValue.SetCharArray (pLiteral->m_BinData, Length);
	Result = m_pModule->m_OperatorMgr.CastOperator (&LiteralValue, GetSimpleType (m_pModule, EType_Char)->GetDataPtrType_c ());
	if (!Result)
		return false;

	pLiteral->m_BinData.Clear ();

	CValue LengthValue;
	LengthValue.SetConstSizeT (Length);

	CValue ResultValue;
	m_pModule->m_LlvmIrBuilder.CreateCall3 (
		pAppend,
		pAppend->GetType (),
		pLiteral->m_FmtLiteralValue,
		LiteralValue,
		LengthValue,
		&ResultValue
		);

	return true;
}

bool
CParser::AppendFmtLiteralValue (
	TLiteral* pLiteral,
	const CValue& RawSrcValue,
	const rtl::CString& FmtSpecifierString
	)
{
	ASSERT (pLiteral->m_FmtLiteralValue);

	if (FmtSpecifierString == 'B') // binary format
		return AppendFmtLiteralBinValue (pLiteral, RawSrcValue);

	CValue SrcValue;
	bool Result = m_pModule->m_OperatorMgr.PrepareOperand (RawSrcValue, &SrcValue);
	if (!Result)
		return false;

	EStdFunc AppendFunc;

	CType* pType = SrcValue.GetType ();
	if (pType->GetTypeKindFlags () & ETypeKindFlag_Integer)
	{
		static EStdFunc FuncTable [2] [2] =
		{
			{ EStdFunc_AppendFmtLiteral_i32, EStdFunc_AppendFmtLiteral_ui32 },
			{ EStdFunc_AppendFmtLiteral_i64, EStdFunc_AppendFmtLiteral_ui64 },
		};

		size_t i1 = pType->GetSize () > 4;
		size_t i2 = (pType->GetTypeKindFlags () & ETypeKindFlag_Unsigned) != 0;

		AppendFunc = FuncTable [i1] [i2];
	}
	else if (pType->GetTypeKindFlags () & ETypeKindFlag_Fp)
	{
		AppendFunc = EStdFunc_AppendFmtLiteral_f;
	}
	else if (IsCharArrayType (pType) || IsCharArrayRefType (pType) || IsCharPtrType (pType))
	{
		AppendFunc = EStdFunc_AppendFmtLiteral_p;
	}
	else
	{
		err::SetFormatStringError ("don't know how to format '%s'", pType->GetTypeString ().cc ());
		return false;
	}

	CFunction* pAppend = m_pModule->m_FunctionMgr.GetStdFunction (AppendFunc);
	CType* pArgType = pAppend->GetType ()->GetArgArray () [2]->GetType ();

	CValue ArgValue;
	Result = m_pModule->m_OperatorMgr.CastOperator (SrcValue, pArgType, &ArgValue);
	if (!Result)
		return false;

	CValue FmtSpecifierValue;
	if (!FmtSpecifierString.IsEmpty ())
	{
		FmtSpecifierValue.SetCharArray (FmtSpecifierString, FmtSpecifierString.GetLength () + 1);
		m_pModule->m_OperatorMgr.CastOperator (&FmtSpecifierValue, GetSimpleType (m_pModule, EType_Char)->GetDataPtrType_c ());
	}
	else
	{
		FmtSpecifierValue = GetSimpleType (m_pModule, EType_Char)->GetDataPtrType_c ()->GetZeroValue ();
	}

	CValue ResultValue;
	m_pModule->m_LlvmIrBuilder.CreateCall3 (
		pAppend,
		pAppend->GetType (),
		pLiteral->m_FmtLiteralValue,
		FmtSpecifierValue,
		ArgValue,
		&ResultValue
		);

	return true;
}

bool
CParser::AppendFmtLiteralBinValue (
	TLiteral* pLiteral,
	const CValue& RawSrcValue
	)
{
	CValue SrcValue;
	bool Result = m_pModule->m_OperatorMgr.PrepareOperand (RawSrcValue, &SrcValue);
	if (!Result)
		return false;

	CType* pType = SrcValue.GetType ();
	CFunction* pAppend = m_pModule->m_FunctionMgr.GetStdFunction (EStdFunc_AppendFmtLiteral_a);
	CType* pArgType = GetSimpleType (m_pModule, EStdType_BytePtr);

	CValue SizeValue (pType->GetSize (), EType_SizeT);

	CValue TmpValue;
	CValue ResultValue;
	m_pModule->m_LlvmIrBuilder.CreateAlloca (pType, "tmpFmtValue", NULL, &TmpValue);
	m_pModule->m_LlvmIrBuilder.CreateStore (SrcValue, TmpValue);
	m_pModule->m_LlvmIrBuilder.CreateBitCast (TmpValue, pArgType, &TmpValue);

	m_pModule->m_LlvmIrBuilder.CreateCall3 (
		pAppend,
		pAppend->GetType (),
		pLiteral->m_FmtLiteralValue,
		TmpValue,
		SizeValue,
		&ResultValue
		);

	return true;
}

bool
CParser::FinalizeLiteral (
	TLiteral* pLiteral,
	CValue* pResultValue
	)
{
	if (!pLiteral->m_FmtLiteralValue)
	{
		if (pLiteral->m_LastToken == EToken_Literal)
			pLiteral->m_BinData.Append (0);

		pResultValue->SetCharArray (pLiteral->m_BinData, pLiteral->m_BinData.GetCount ());
		return true;
	}

	AppendFmtLiteral (pLiteral, NULL, 0);

	CValue PtrValue;
	CValue SizeValue;
	CValue ObjHdrValue;

	m_pModule->m_LlvmIrBuilder.CreateGep2 (pLiteral->m_FmtLiteralValue, 0, NULL, &PtrValue);
	m_pModule->m_LlvmIrBuilder.CreateLoad (PtrValue, NULL, &PtrValue);

	m_pModule->m_LlvmIrBuilder.CreateGep2 (pLiteral->m_FmtLiteralValue, 2, NULL, &SizeValue);
	m_pModule->m_LlvmIrBuilder.CreateLoad (SizeValue, NULL, &SizeValue);
	m_pModule->m_LlvmIrBuilder.CreateAdd_i (SizeValue, CValue (1, EType_SizeT), NULL, &SizeValue);

	CType* pObjHdrPtrType = m_pModule->m_TypeMgr.GetStdType (EStdType_ObjHdrPtr);
	m_pModule->m_LlvmIrBuilder.CreateBitCast (PtrValue, pObjHdrPtrType, &ObjHdrValue);
	m_pModule->m_LlvmIrBuilder.CreateGep (ObjHdrValue, -1, pObjHdrPtrType, &ObjHdrValue);

	pResultValue->SetLeanDataPtr (
		PtrValue.GetLlvmValue (),
		GetSimpleType (m_pModule, EType_Char)->GetDataPtrType (EDataPtrType_Lean),
		ObjHdrValue,
		PtrValue,
		SizeValue
		);

	return true;
}

bool
CParser::FinalizeLiteral_0 (
	TLiteral* pLiteral,
	CValue* pResultValue
	)
{
	CType* pType;

	if (!pLiteral->m_FmtLiteralValue)
	{
		size_t Count = pLiteral->m_BinData.GetCount ();

		if (pLiteral->m_LastToken == EToken_Literal)
			Count++;

		pType = m_pModule->m_TypeMgr.GetArrayType (EType_Char, Count);
	}
	else
	{
		pType = GetSimpleType (m_pModule, EType_Char)->GetDataPtrType ();
	}

	pResultValue->SetType (pType);
	return true;
}

//.............................................................................

} // namespace jnc {

