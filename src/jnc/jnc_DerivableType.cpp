#include "pch.h"
#include "jnc_DerivableType.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CBaseTypeSlot::CBaseTypeSlot ()
{
	m_ItemKind = EModuleItem_BaseTypeSlot;
	m_pType = NULL;
	m_pType_i = NULL;
	m_Offset = 0;
	m_LlvmIndex = -1;
	m_VTableIndex = -1;
}

//.............................................................................

CBaseTypeCoord::CBaseTypeCoord ():
	m_LlvmIndexArray (ref::EBuf_Field, m_Buffer, sizeof (m_Buffer))
{
	m_pType = NULL;
	m_Offset = 0;
	m_VTableIndex = 0;
}

//.............................................................................

CDerivableType::CDerivableType ()
{
	m_pPreConstructor = NULL;
	m_pConstructor = NULL;
	m_pDefaultConstructor = NULL;
	m_pStaticConstructor = NULL;
	m_pStaticDestructor = NULL;
	m_pStaticOnceFlagVariable = NULL;
	m_pCallOperator = NULL;
}

CFunction*
CDerivableType::GetDefaultConstructor ()
{
	ASSERT (m_pConstructor);
	if (m_pDefaultConstructor)
		return m_pDefaultConstructor;

	CType* pThisArgType = GetThisArgType (EPtrTypeFlag_Safe);

	// avoid allocations

	rtl::CBoxListEntryT <CValue> ThisArgValue;
	ThisArgValue.m_Value.SetType (pThisArgType);

	rtl::CAuxListT <rtl::CBoxListEntryT <CValue> > ArgList;
	ArgList.InsertTail (&ThisArgValue);

	m_pDefaultConstructor = m_pConstructor->ChooseOverload (ArgList);
	if (!m_pDefaultConstructor)
	{
		err::SetFormatStringError ("'%s' has no default constructor", GetTypeString ().cc ()); // thanks a lot gcc
		return NULL;
	}

	return m_pDefaultConstructor;
}

CBaseTypeSlot*
CDerivableType::GetBaseTypeByIndex (size_t Index)
{
	size_t Count = m_BaseTypeList.GetCount ();
	if (Index >= Count)
	{
		err::SetFormatStringError ("index '%d' is out of bounds", Index);
		return NULL;
	}

	if (m_BaseTypeArray.GetCount () != Count)
	{
		m_BaseTypeArray.SetCount (Count);
		rtl::CIteratorT <CBaseTypeSlot> Slot = m_BaseTypeList.GetHead ();
		for (size_t i = 0; i < Count; i++, Slot++)
			m_BaseTypeArray [i] = *Slot;
	}

	return m_BaseTypeArray [Index];
}

CBaseTypeSlot*
CDerivableType::AddBaseType (CType* pType)
{
	rtl::CStringHashTableMapIteratorT <CBaseTypeSlot*> It = m_BaseTypeMap.Goto (pType->GetSignature ());
	if (It->m_Value)
	{
		err::SetFormatStringError (
			"'%s' is already a base type",
			pType->GetTypeString ().cc () // thanks a lot gcc
			);
		return NULL;
	}

	CBaseTypeSlot* pSlot = AXL_MEM_NEW (CBaseTypeSlot);
	pSlot->m_pModule = m_pModule;

	EType TypeKind = pType->GetTypeKind ();
	if (TypeKind == EType_NamedImport)
	{
		pSlot->m_pType_i = (CImportType*) pType;
		m_ImportBaseTypeArray.Append (pSlot);
	}
	else if (
		(pType->GetTypeKindFlags () & ETypeKindFlag_Derivable) &&
		(TypeKind != EType_Class || m_TypeKind == EType_Class))
	{
		pSlot->m_pType = (CDerivableType*) pType;
	}
	else
	{
		err::SetFormatStringError (
			"'%s' cannot be inherited from '%s'",
			GetTypeString ().cc (),
			pType->GetTypeString ().cc ()
			);
		return NULL;
	}

	m_BaseTypeList.InsertTail (pSlot);
	It->m_Value = pSlot;
	return pSlot;
}

bool
CDerivableType::ResolveImportBaseType (CBaseTypeSlot* pSlot)
{
	ASSERT (pSlot->m_pType_i);

	CType* pType = pSlot->m_pType_i->GetActualType ();
	rtl::CStringHashTableMapIteratorT <CBaseTypeSlot*> It = m_BaseTypeMap.Goto (pType->GetSignature ());
	if (It->m_Value)
	{
		err::SetFormatStringError (
			"'%s' is already a base type",
			pType->GetTypeString ().cc () // thanks a lot gcc
			);
		return false;
	}

	if (!(pType->GetTypeKindFlags () & ETypeKindFlag_Derivable) ||
		pType->GetTypeKind () == EType_Class && m_TypeKind != EType_Class)
	{
		err::SetFormatStringError (
			"'%s' cannot be inherited from '%s'",
			GetTypeString ().cc (),
			pType->GetTypeString ().cc ()
			);
		return NULL;
	}

	pSlot->m_pType = (CDerivableType*) pType;
	It->m_Value = pSlot;
	return true;
}

bool
CDerivableType::ResolveImportBaseTypes ()
{
	bool Result;

	size_t Count = m_ImportBaseTypeArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		Result = ResolveImportBaseType (m_ImportBaseTypeArray [i]);
		if (!Result)
			return false;
	}

	return true;
}

bool
CDerivableType::ResolveImportFields ()
{
	size_t Count = m_ImportFieldArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CStructField* pField = m_ImportFieldArray [i];
		ASSERT (pField->m_pType_i);

		CType* pType = pField->m_pType_i->GetActualType ();
		if (pField->m_pType->GetTypeKindFlags () & ETypeKindFlag_Code)
		{
			err::SetFormatStringError ("'%s': illegal type for a field", pType->GetTypeString ().cc ());
			return false;
		}

		pField->m_pType = pType;
		
		if (pField->m_BitCount)
		{
			ASSERT (pField->m_pBitFieldBaseType == pField->m_pType_i);
			pField->m_pBitFieldBaseType = pType;
		}
	}

	return true;
}

CFunction*
CDerivableType::CreateMethod (
	EStorage StorageKind,
	const rtl::CString& Name,
	CFunctionType* pShortType
	)
{
	rtl::CString QualifiedName = CreateQualifiedName (Name);

	CFunction* pFunction = m_pModule->m_FunctionMgr.CreateFunction (EFunction_Named, pShortType);
	pFunction->m_StorageKind = StorageKind;
	pFunction->m_Name = Name;
	pFunction->m_QualifiedName = QualifiedName;
	pFunction->m_DeclaratorName = Name;
	pFunction->m_Tag = QualifiedName;

	bool Result = AddMethod (pFunction);
	if (!Result)
		return NULL;

	return pFunction;
}

CFunction*
CDerivableType::CreateUnnamedMethod (
	EStorage StorageKind,
	EFunction FunctionKind,
	CFunctionType* pShortType
	)
{
	CFunction* pFunction = m_pModule->m_FunctionMgr.CreateFunction (FunctionKind, pShortType);
	pFunction->m_StorageKind = StorageKind;
	pFunction->m_Tag.Format ("%s.%s", m_Tag.cc (), GetFunctionKindString (FunctionKind));

	bool Result = AddMethod (pFunction);
	if (!Result)
		return NULL;

	return pFunction;
}

CProperty*
CDerivableType::CreateProperty (
	EStorage StorageKind,
	const rtl::CString& Name,
	CPropertyType* pShortType
	)
{
	rtl::CString QualifiedName = CreateQualifiedName (Name);

	CProperty* pProperty = m_pModule->m_FunctionMgr.CreateProperty (Name, QualifiedName);

	bool Result =
		AddProperty (pProperty) &&
		pProperty->Create (pShortType);

	if (!Result)
		return NULL;

	return pProperty;
}

bool
CDerivableType::AddMethod (CFunction* pFunction)
{
	EStorage StorageKind = pFunction->GetStorageKind ();
	EFunction FunctionKind = pFunction->GetFunctionKind ();
	uint_t FunctionKindFlags = GetFunctionKindFlags (FunctionKind);
	uint_t ThisArgTypeFlags = pFunction->m_ThisArgTypeFlags;

	pFunction->m_pParentNamespace = this;

	switch (StorageKind)
	{
	case EStorage_Static:
		if (ThisArgTypeFlags)
		{
			err::SetFormatStringError ("static method cannot be '%s'", GetPtrTypeFlagString (ThisArgTypeFlags).cc ());
			return false;
		}

		break;

	case EStorage_Undefined:
		pFunction->m_StorageKind = EStorage_Member;
		// and fall through

	case EStorage_Member:
		pFunction->ConvertToMemberMethod (this);
		break;

	default:
		err::SetFormatStringError ("invalid storage specifier '%s' for method member", GetStorageKindString (StorageKind));
		return false;
	}

	CFunction** ppTarget = NULL;

	switch (FunctionKind)
	{
	case EFunction_PreConstructor:
		ppTarget = &m_pPreConstructor;
		break;

	case EFunction_Constructor:
		ppTarget = &m_pConstructor;
		break;

	case EFunction_StaticConstructor:
		ppTarget = &m_pStaticConstructor;
		break;

	case EFunction_StaticDestructor:
		ppTarget = &m_pStaticDestructor;
		break;

	case EFunction_Named:
		return AddFunction (pFunction);

	case EFunction_UnaryOperator:
		pFunction->m_Tag.Format ("%s.operator %s", m_Tag.cc (), GetUnOpKindString (pFunction->GetUnOpKind ()));

		if (m_UnaryOperatorTable.IsEmpty ())
			m_UnaryOperatorTable.SetCount (EUnOp__Count);

		ppTarget = &m_UnaryOperatorTable [pFunction->GetUnOpKind ()];
		break;

	case EFunction_BinaryOperator:
		pFunction->m_Tag.Format ("%s.operator %s", m_Tag.cc (), GetBinOpKindString (pFunction->GetBinOpKind ()));

		if (m_BinaryOperatorTable.IsEmpty ())
			m_BinaryOperatorTable.SetCount (EBinOp__Count);

		ppTarget = &m_BinaryOperatorTable [pFunction->GetBinOpKind ()];
		break;

	case EFunction_CallOperator:
		pFunction->m_Tag.Format ("%s.operator ()", m_Tag.cc ());
		ppTarget = &m_pCallOperator;
		break;

	default:
		err::SetFormatStringError (
			"invalid %s in '%s'",
			GetFunctionKindString (FunctionKind),
			GetTypeString ().cc ()
			);
		return false;
	}

	pFunction->m_Tag.Format ("%s.%s", m_Tag.cc (), GetFunctionKindString (FunctionKind));

	if (!*ppTarget)
	{
		*ppTarget = pFunction;
	}
	else if (FunctionKindFlags & EFunctionKindFlag_NoOverloads)
	{
		err::SetFormatStringError (
			"'%s' already has '%s' method",
			GetTypeString ().cc (),
			GetFunctionKindString (FunctionKind)
			);
		return false;
	}
	else
	{
		bool Result = (*ppTarget)->AddOverload (pFunction);
		if (!Result)
			return false;
	}

	return true;
}

bool
CDerivableType::AddProperty (CProperty* pProperty)
{
	ASSERT (pProperty->IsNamed ());
	bool Result = AddItem (pProperty);
	if (!Result)
		return false;

	pProperty->m_pParentNamespace = this;

	EStorage StorageKind = pProperty->GetStorageKind ();
	switch (StorageKind)
	{
	case EStorage_Static:
		break;

	case EStorage_Undefined:
		pProperty->m_StorageKind = EStorage_Member;
		//and fall through

	case EStorage_Member:
		pProperty->m_pParentType = this;
		break;

	default:
		err::SetFormatStringError ("invalid storage specifier '%s' for method member", GetStorageKindString (StorageKind));
		return false;
	}

	m_MemberPropertyArray.Append (pProperty);
	return true;
}

bool
CDerivableType::CreateDefaultMethod (
	EFunction FunctionKind,
	EStorage StorageKind
	)
{
	CFunctionType* pType = (CFunctionType*) m_pModule->m_TypeMgr.GetStdType (EStdType_SimpleFunction);
	CFunction* pFunction = m_pModule->m_FunctionMgr.CreateFunction (FunctionKind, pType);
	pFunction->m_StorageKind = StorageKind;
	pFunction->m_Tag.Format ("%s.%s", m_Tag.cc (), GetFunctionKindString (FunctionKind));

	bool Result = AddMethod (pFunction);
	if (!Result)
		return false;

	m_pModule->MarkForCompile (this);
	return true;
}

bool
CDerivableType::CompileDefaultStaticConstructor ()
{
	ASSERT (m_pStaticConstructor);

	CToken::CPos Pos;

	TOnceStmt Stmt;
	m_pModule->m_ControlFlowMgr.OnceStmt_Create (&Stmt, Pos);

	bool Result = m_pModule->m_ControlFlowMgr.OnceStmt_PreBody (&Stmt, Pos);
	if (!Result)
		return false;

	m_pModule->m_ControlFlowMgr.OnceStmt_PostBody (&Stmt, Pos);
	return true;
}

bool
CDerivableType::CompileDefaultConstructor ()
{
	ASSERT (m_pConstructor);

	bool Result;

	CValue ThisValue;
	m_pModule->m_FunctionMgr.InternalPrologue (m_pConstructor, &ThisValue, 1);

	Result =
		CallBaseTypeConstructors (ThisValue) &&
		CallMemberPropertyConstructors (ThisValue) &&
		CallMemberFieldConstructors (ThisValue);

	if (!Result)
		return false;

	if (m_pPreConstructor)
	{
		Result = m_pModule->m_OperatorMgr.CallOperator (m_pPreConstructor, ThisValue);
		if (!Result)
			return false;
	}

	m_pModule->m_FunctionMgr.InternalEpilogue ();
	return true;
}

bool
CDerivableType::CallBaseTypeConstructors (const CValue& ThisValue)
{
	bool Result;

	size_t Count = m_BaseTypeConstructArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CBaseTypeSlot* pSlot = m_BaseTypeConstructArray [i];
		if (pSlot->m_Flags & EModuleItemFlag_Constructed)
		{
			pSlot->m_Flags &= ~EModuleItemFlag_Constructed;
			continue;
		}

		CFunction* pConstructor = pSlot->m_pType->GetDefaultConstructor ();
		if (!pConstructor)
			return false;

		Result = m_pModule->m_OperatorMgr.CallOperator (pConstructor, ThisValue);
		if (!Result)
			return false;
	}

	return true;
}

bool
CDerivableType::CallMemberFieldConstructors (const CValue& ThisValue)
{
	bool Result;

	size_t Count = m_MemberFieldConstructArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CStructField* pField = m_MemberFieldConstructArray [i];
		if (pField->m_Flags & EModuleItemFlag_Constructed)
		{
			pField->m_Flags &= ~EModuleItemFlag_Constructed;
			continue;
		}

		CValue FieldValue;
		Result = m_pModule->m_OperatorMgr.GetClassField (ThisValue, pField, NULL, &FieldValue);
		if (!Result)
			return false;

		ASSERT (pField->GetType ()->GetTypeKindFlags () & ETypeKindFlag_Derivable);
		CDerivableType* pType = (CDerivableType*) pField->GetType ();

		CFunction* pConstructor;

		rtl::CBoxListT <CValue> ArgList;
		ArgList.InsertTail (FieldValue);

		if (!(pType->GetFlags () & ETypeFlag_Child))
		{
			pConstructor = pType->GetDefaultConstructor ();
			if (!pConstructor)
				return false;
		}
		else
		{
			pConstructor = pType->GetConstructor ();
			ASSERT (pConstructor && !pConstructor->IsOverloaded ());

			ArgList.InsertTail (ThisValue);
		}

		Result = m_pModule->m_OperatorMgr.CallOperator (pConstructor, &ArgList);
		if (!Result)
			return false;
	}

	return true;
}

bool
CDerivableType::CallMemberPropertyConstructors (const CValue& ThisValue)
{
	bool Result;

	size_t Count = m_MemberPropertyConstructArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CProperty* pProperty = m_MemberPropertyConstructArray [i];
		if (pProperty->m_Flags & EModuleItemFlag_Constructed)
		{
			pProperty->m_Flags &= ~EModuleItemFlag_Constructed;
			continue;
		}

		CFunction* pDestructor = pProperty->GetDefaultConstructor ();
		ASSERT (pDestructor);

		Result = m_pModule->m_OperatorMgr.CallOperator (pDestructor, ThisValue);
		if (!Result)
			return false;
	}

	return true;
}

bool
CDerivableType::FindBaseTypeTraverseImpl (
	CType* pType,
	CBaseTypeCoord* pCoord,
	size_t Level
	)
{
	rtl::CStringHashTableMapIteratorT <CBaseTypeSlot*> It = m_BaseTypeMap.Find (pType->GetSignature ());
	if (It)
	{
		if (!pCoord)
			return true;

		CBaseTypeSlot* pSlot = It->m_Value;
		pCoord->m_pType = pSlot->m_pType;
		pCoord->m_Offset = pSlot->m_Offset;
		pCoord->m_VTableIndex = pSlot->m_VTableIndex;
		pCoord->m_LlvmIndexArray.SetCount (Level + 1);
		pCoord->m_LlvmIndexArray [Level] = pSlot->m_LlvmIndex;
		return true;
	}

	rtl::CIteratorT <CBaseTypeSlot> Slot = m_BaseTypeList.GetHead ();
	for (; Slot; Slot++)
	{
		CBaseTypeSlot* pSlot = *Slot;
		ASSERT (pSlot->m_pType);

		bool Result = pSlot->m_pType->FindBaseTypeTraverseImpl (pType, pCoord, Level + 1);
		if (Result)
		{
			if (pCoord)
			{
				pCoord->m_Offset += pSlot->m_Offset;
				pCoord->m_VTableIndex += pSlot->m_VTableIndex;
				pCoord->m_LlvmIndexArray [Level] = pSlot->m_LlvmIndex;
			}

			return true;
		}
	}

	return false;
}

CModuleItem*
CDerivableType::FindItemTraverseImpl (
	const char* pName,
	CMemberCoord* pCoord,
	uint_t Flags,
	size_t Level
	)
{
	CModuleItem* pItem;

	if (!(Flags & ETraverse_NoThis))
	{
		pItem = FindItem (pName);
		if (pItem)
		{
			if (pCoord)
			{
				pCoord->m_pType = this;
				pCoord->m_LlvmIndexArray.SetCount (Level);
			}

			return pItem;
		}

		size_t Count = m_UnnamedFieldArray.GetCount ();
		for	(size_t i = 0; i < Count; i++)
		{
			CStructField* pField = m_UnnamedFieldArray [i];
			if (pField->GetType ()->GetTypeKindFlags () & ETypeKindFlag_Derivable)
			{
				CDerivableType* pType = (CDerivableType*) pField->GetType ();
				pItem = pType->FindItemTraverseImpl (pName, pCoord, Flags | ETraverse_NoParentNamespace, Level + 1);
				if (pItem)
				{
					if (pCoord)
					{
						pCoord->m_Offset += pField->m_Offset;
						pCoord->m_LlvmIndexArray [Level] = pField->m_LlvmIndex;

						if (m_TypeKind == EType_Union)
						{
							TUnionCoord UnionCoord;
							UnionCoord.m_pType = (CUnionType*) this;
							UnionCoord.m_Level = Level;
							pCoord->m_UnionCoordArray.Insert (0, UnionCoord);
						}
					}

					return pItem;
				}
			}
		}
	}

	if (!(Flags & ETraverse_NoExtensionNamespace) && m_pExtensionNamespace)
	{
		pItem = m_pExtensionNamespace->FindItem (pName);
		if (pItem)
		{
			if (pCoord)
			{
				pCoord->m_pType = this;
				pCoord->m_LlvmIndexArray.SetCount (Level);
			}

			return pItem;
		}
	}

	Flags &= ~ETraverse_NoThis;

	if (!(Flags & ETraverse_NoBaseType))
	{
		rtl::CIteratorT <CBaseTypeSlot> Slot = m_BaseTypeList.GetHead ();
		for (; Slot; Slot++)
		{
			CBaseTypeSlot* pSlot = *Slot;

			CDerivableType* pBaseType = NULL;

			if (pSlot->m_pType)
			{
				pBaseType = pSlot->m_pType;
			}
			else if (pSlot->m_pType_i && pSlot->m_pType_i->IsResolved ())
			{
				CType* pActualType = pSlot->m_pType_i->GetActualType ();
				if (pActualType->GetTypeKindFlags () & ETypeKindFlag_Derivable)
					pBaseType = (CDerivableType*) pActualType;
			}

			if (!pBaseType)
				return NULL;

			pItem = pBaseType->FindItemTraverseImpl (pName, pCoord, Flags, Level + 1);
			if (pItem)
			{
				if (pCoord)
				{
					pCoord->m_Offset += pSlot->m_Offset;
					pCoord->m_LlvmIndexArray [Level] = pSlot->m_LlvmIndex;
					pCoord->m_VTableIndex += pSlot->m_VTableIndex;
				}

				return pItem;
			}
		}
	}

	if (!(Flags & ETraverse_NoParentNamespace) && m_pParentNamespace)
	{
		pItem = m_pParentNamespace->FindItemTraverse (pName, pCoord, Flags);
		if (pItem)
			return pItem;
	}

	return NULL;
}

//.............................................................................

} // namespace jnc {
