#include "pch.h"
#include "jnc_StructType.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CStructField::CStructField ()
{
	m_ItemKind = EModuleItem_StructField;
	m_pType = NULL;
	m_pType_i = NULL;
	m_PtrTypeFlags = 0;
	m_pBitFieldBaseType = NULL;
	m_BitCount = 0;
	m_Offset = 0;
	m_LlvmIndex = -1;
}

//.............................................................................

CStructType::CStructType ()
{
	m_TypeKind = EType_Struct;
	m_Flags = ETypeFlag_Pod | ETypeFlag_StructRet;
	m_PackFactor = 8;
	m_FieldActualSize = 0;
	m_FieldAlignedSize = 0;
	m_pLastBitFieldType = NULL;
	m_LastBitFieldOffset = 0;
}

void
CStructType::PrepareLlvmType ()
{
	m_pLlvmType = llvm::StructType::create (*m_pModule->GetLlvmContext (), m_Tag.cc ());
}

CStructField*
CStructType::CreateFieldImpl (
	const rtl::CString& Name,
	CType* pType,
	size_t BitCount,
	uint_t PtrTypeFlags,
	rtl::CBoxListT <CToken>* pConstructor,
	rtl::CBoxListT <CToken>* pInitializer
	)
{
	CStructField* pField = AXL_MEM_NEW (CStructField);
	pField->m_pModule = m_pModule;
	pField->m_StorageKind = m_StorageKind;
	pField->m_pParentNamespace = this;
	pField->m_Name = Name;
	pField->m_pType = pType;
	pField->m_PtrTypeFlags = PtrTypeFlags;
	pField->m_pBitFieldBaseType = BitCount ? pType : NULL;
	pField->m_BitCount = BitCount;

	if (pConstructor)
		pField->m_Constructor.TakeOver (pConstructor);

	if (pInitializer)
		pField->m_Initializer.TakeOver (pInitializer);

	if (!pField->m_Constructor.IsEmpty () ||
		!pField->m_Initializer.IsEmpty ())
	{
		m_InitializedFieldArray.Append (pField);
	}

	m_FieldList.InsertTail (pField);

	if (Name.IsEmpty ())
	{
		m_UnnamedFieldArray.Append (pField);
	}
	else if (Name [0] != '!') // internal field
	{
		bool Result = AddItem (pField);
		if (!Result)
			return NULL;
	}

	if (pType->GetTypeKindFlags () & ETypeKindFlag_Import)
	{
		pField->m_pType_i = (CImportType*) pType;
		m_ImportFieldArray.Append (pField);
	}

	m_MemberFieldArray.Append (pField);
	return pField;
}

CStructField*
CStructType::GetFieldByIndexImpl (
	size_t Index,
	bool IgnoreBaseTypes
	)
{
	if (!IgnoreBaseTypes && !m_BaseTypeList.IsEmpty ())
	{
		err::SetFormatStringError ("'%s' has base types, cannot use indexed member operator", GetTypeString ().cc ());
		return NULL;
	}

	size_t Count = m_FieldList.GetCount ();
	if (Index >= Count)
	{
		err::SetFormatStringError ("index '%d' is out of bounds", Index);
		return NULL;
	}

	if (m_FieldArray.GetCount () != Count)
	{
		m_FieldArray.SetCount (Count);
		rtl::CIteratorT <CStructField> Field = m_FieldList.GetHead ();
		for (size_t i = 0; i < Count; i++, Field++)
			m_FieldArray [i] = *Field;
	}

	return m_FieldArray [Index];
}

bool
CStructType::Append (CStructType* pType)
{
	bool Result;

	rtl::CIteratorT <CBaseTypeSlot> Slot = pType->m_BaseTypeList.GetHead ();
	for (; Slot; Slot++)
	{
		Result = AddBaseType (Slot->m_pType) != NULL;
		if (!Result)
			return false;
	}

	rtl::CIteratorT <CStructField> Field = pType->m_FieldList.GetHead ();
	for (; Field; Field++)
	{
		Result = Field->m_BitCount ?
			CreateField (Field->m_Name, Field->m_pBitFieldBaseType, Field->m_BitCount, Field->m_PtrTypeFlags) != NULL:
			CreateField (Field->m_Name, Field->m_pType, 0, Field->m_PtrTypeFlags) != NULL;

		if (!Result)
			return false;
	}

	return true;
}

bool
CStructType::CalcLayout ()
{
	bool Result;

	if (m_pExtensionNamespace)
		ApplyExtensionNamespace ();

	Result =
		ResolveImportBaseTypes () &&
		ResolveImportFields ();

	if (!Result)
		return false;

	rtl::CIteratorT <CBaseTypeSlot> Slot = m_BaseTypeList.GetHead ();
	for (; Slot; Slot++)
	{
		CBaseTypeSlot* pSlot = *Slot;

		Result = pSlot->m_pType->EnsureLayout ();
		if (!Result)
			return false;

		if (pSlot->m_pType->GetTypeKind () == EType_Class)
		{
			err::SetFormatStringError ("'%s' cannot be a base type of a struct", pSlot->m_pType->GetTypeString ().cc ());
			return false;
		}

		if (pSlot->m_pType->GetFlags () & ETypeFlag_GcRoot)
		{
			m_GcRootBaseTypeArray.Append (pSlot);
			m_Flags |= ETypeFlag_GcRoot;
		}

		if (pSlot->m_pType->GetConstructor ())
			m_BaseTypeConstructArray.Append (pSlot);

		Result = LayoutField (
			pSlot->m_pType,
			&pSlot->m_Offset,
			&pSlot->m_LlvmIndex
			);

		if (!Result)
			return false;
	}

	rtl::CIteratorT <CStructField> Field = m_FieldList.GetHead ();
	for (; Field; Field++)
	{
		CStructField* pField = *Field;

		Result = pField->m_pType->EnsureLayout ();
		if (!Result)
			return false;

		if (m_StructTypeKind != EStructType_IfaceStruct && pField->m_pType->GetTypeKind () == EType_Class)
		{
			err::SetFormatStringError ("'%s' cannot be a field of a struct", pField->m_pType->GetTypeString ().cc ());
			return false;
		}

		Result = pField->m_BitCount ?
			LayoutBitField (
				pField->m_pBitFieldBaseType,
				pField->m_BitCount,
				&pField->m_pType,
				&pField->m_Offset,
				&pField->m_LlvmIndex
				) :
			LayoutField (
				pField->m_pType,
				&pField->m_Offset,
				&pField->m_LlvmIndex
				);

		if (!Result)
			return false;
	}

	if (m_FieldAlignedSize > m_FieldActualSize)
		InsertPadding (m_FieldAlignedSize - m_FieldActualSize);

	// scan members for gcroots and constructors (not for auxilary structs such as class iface)

	if (m_StructTypeKind == EStructType_Normal)
	{
		size_t Count = m_MemberFieldArray.GetCount ();
		for (size_t i = 0; i < Count; i++)
		{
			CStructField* pField = m_MemberFieldArray [i];
			CType* pType = pField->GetType ();

			uint_t FieldTypeFlags = pType->GetFlags ();

			if (!(FieldTypeFlags & ETypeFlag_Pod))
				m_Flags &= ~ETypeFlag_Pod;

			if (FieldTypeFlags & ETypeFlag_GcRoot)
			{
				m_GcRootMemberFieldArray.Append (pField);
				m_Flags |= ETypeFlag_GcRoot;
			}

			if ((pType->GetTypeKindFlags () & ETypeKindFlag_Derivable) && ((CDerivableType*) pType)->GetConstructor ())
				m_MemberFieldConstructArray.Append (pField);
		}

		Count = m_MemberPropertyArray.GetCount ();
		for (size_t i = 0; i < Count; i++)
		{
			CProperty* pProperty = m_MemberPropertyArray [i];
			Result = pProperty->EnsureLayout ();
			if (!Result)
				return false;

			if (pProperty->GetConstructor ())
				m_MemberPropertyConstructArray.Append (pProperty);
		}
	}

	llvm::StructType* pLlvmStructType = (llvm::StructType*) GetLlvmType ();
	pLlvmStructType->setBody (
		llvm::ArrayRef<llvm::Type*> (m_LlvmFieldTypeArray, m_LlvmFieldTypeArray.GetCount ()),
		true
		);

	m_Size = m_FieldAlignedSize;

	if (m_StructTypeKind == EStructType_Normal)
	{
		if (!m_pStaticConstructor && m_pStaticDestructor)
		{
			Result = CreateDefaultMethod (EFunction_StaticConstructor, EStorage_Static);
			if (!Result)
				return false;
		}

		if (m_pStaticConstructor)
			m_pStaticOnceFlagVariable = m_pModule->m_VariableMgr.CreateOnceFlagVariable ();

		if (m_pStaticDestructor)
			m_pModule->m_VariableMgr.m_StaticDestructList.AddStaticDestructor (m_pStaticDestructor, m_pStaticOnceFlagVariable);

		if (!m_pPreConstructor &&
			(m_pStaticConstructor ||
			!m_InitializedFieldArray.IsEmpty ()))
		{
			Result = CreateDefaultMethod (EFunction_PreConstructor);
			if (!Result)
				return false;
		}

		if (!m_pConstructor &&
			(m_pPreConstructor ||
			!m_BaseTypeConstructArray.IsEmpty () ||
			!m_MemberFieldConstructArray.IsEmpty () ||
			!m_MemberPropertyConstructArray.IsEmpty ()))
		{
			Result = CreateDefaultMethod (EFunction_Constructor);
			if (!Result)
				return false;
		}
	}

	return true;
}

bool
CStructType::Compile ()
{
	bool Result;

	if (m_pStaticConstructor && !(m_pStaticConstructor->GetFlags () & EModuleItemFlag_User))
	{
		Result = CompileDefaultStaticConstructor ();
		if (!Result)
			return false;
	}

	if (m_pPreConstructor && !(m_pPreConstructor->GetFlags () & EModuleItemFlag_User))
	{
		Result = CompileDefaultPreConstructor ();
		if (!Result)
			return false;
	}

	if (m_pConstructor && !(m_pConstructor->GetFlags () & EModuleItemFlag_User))
	{
		Result = CompileDefaultConstructor ();
		if (!Result)
			return false;
	}

	return true;
}

bool
CStructType::CompileDefaultPreConstructor ()
{
	ASSERT (m_pPreConstructor);

	bool Result;

	CValue ThisValue;
	m_pModule->m_FunctionMgr.InternalPrologue (m_pPreConstructor, &ThisValue, 1);

	Result = InitializeFields (ThisValue);
	if (!Result)
		return false;

	m_pModule->m_FunctionMgr.InternalEpilogue ();
	return true;
}

bool
CStructType::InitializeFields (const CValue& ThisValue)
{
	bool Result;

	size_t Count = m_InitializedFieldArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CStructField* pField = m_InitializedFieldArray [i];

		CValue FieldValue;
		Result = m_pModule->m_OperatorMgr.GetField (ThisValue, pField, NULL, &FieldValue);
		if (!Result)
			return false;

		Result = m_pModule->m_OperatorMgr.ParseInitializer (
			FieldValue,
			m_pParentUnit,
			pField->m_Constructor,
			pField->m_Initializer
			);

		if (!Result)
			return false;
	}

	return true;
}

bool
CStructType::LayoutField (
	llvm::Type* pLlvmType,
	size_t Size,
	size_t AlignFactor,
	size_t* pOffset,
	uint_t* pLlvmIndex
	)
{
	if (AlignFactor > m_AlignFactor)
		m_AlignFactor = AXL_MIN (AlignFactor, m_PackFactor);

	size_t Offset = GetFieldOffset (AlignFactor);
	if (Offset > m_FieldActualSize)
		InsertPadding (Offset - m_FieldActualSize);

	*pOffset = Offset;
	*pLlvmIndex = (uint_t) m_LlvmFieldTypeArray.GetCount ();

	m_pLastBitFieldType = NULL;
	m_LastBitFieldOffset = 0;

	m_LlvmFieldTypeArray.Append (pLlvmType);
	SetFieldActualSize (Offset + Size);
	return true;
}

bool
CStructType::LayoutBitField (
	CType* pBaseType,
	size_t BitCount,
	CType** ppType,
	size_t* pOffset,
	uint_t* pLlvmIndex
	)
{
	size_t BitOffset = GetBitFieldBitOffset (pBaseType, BitCount);
	CBitFieldType* pType = m_pModule->m_TypeMgr.GetBitFieldType (pBaseType, BitOffset, BitCount);
	if (!pType)
		return false;

	*ppType = pType;
	m_pLastBitFieldType = pType;

	if (BitOffset)
	{
		*pOffset = m_LastBitFieldOffset;
		*pLlvmIndex = (uint_t) m_LlvmFieldTypeArray.GetCount () - 1;
		return true;
	}

	size_t AlignFactor = pType->GetAlignFactor ();
	if (AlignFactor > m_AlignFactor)
		m_AlignFactor = AXL_MIN (AlignFactor, m_PackFactor);

	size_t Offset = GetFieldOffset (AlignFactor);
	m_LastBitFieldOffset = Offset;

	if (Offset > m_FieldActualSize)
		InsertPadding (Offset - m_FieldActualSize);

	*pOffset = Offset;
	*pLlvmIndex = (uint_t) m_LlvmFieldTypeArray.GetCount ();

	m_LlvmFieldTypeArray.Append (pType->GetLlvmType ());
	SetFieldActualSize (Offset + pType->GetSize ());
	return true;
}

size_t
CStructType::GetFieldOffset (size_t AlignFactor)
{
	size_t Offset = m_FieldActualSize;

	if (AlignFactor > m_PackFactor)
		AlignFactor = m_PackFactor;

	size_t Mod = Offset % AlignFactor;
	if (Mod)
		Offset += AlignFactor - Mod;

	return Offset;
}

size_t
CStructType::GetBitFieldBitOffset (
	CType* pType,
	size_t BitCount
	)
{
	if (!m_pLastBitFieldType || m_pLastBitFieldType->GetBaseType ()->Cmp (pType) != 0)
		return 0;

	size_t LastBitOffset =
		m_pLastBitFieldType->GetBitOffset () +
		m_pLastBitFieldType->GetBitCount ();

	return LastBitOffset + BitCount <= pType->GetSize () * 8 ? LastBitOffset : 0;
}

size_t
CStructType::SetFieldActualSize (size_t Size)
{
	if (m_FieldActualSize >= Size)
		return m_FieldAlignedSize;

	m_FieldActualSize = Size;
	m_FieldAlignedSize = Size;

	size_t Mod = m_Size % m_AlignFactor;
	if (Mod)
		m_FieldAlignedSize += m_AlignFactor - Mod;

	return m_FieldAlignedSize;
}

CArrayType*
CStructType::InsertPadding (size_t Size)
{
	CArrayType* pType = m_pModule->m_TypeMgr.GetArrayType (EType_Int8_u, Size);
	m_LlvmFieldTypeArray.Append (pType->GetLlvmType ());
	return pType;
}

void
CStructType::PrepareLlvmDiType ()
{
	m_LlvmDiType = m_pModule->m_LlvmDiBuilder.CreateEmptyStructType (this);
	m_pModule->m_LlvmDiBuilder.SetStructTypeBody (this);
}

void
CStructType::GcMark (
	CRuntime* pRuntime,
	void* _p
	)
{
	char* p = (char*) _p;

	size_t Count = m_GcRootBaseTypeArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CBaseTypeSlot* pSlot = m_GcRootBaseTypeArray [i];
		pSlot->GetType ()->GcMark (pRuntime, p + pSlot->GetOffset ());
	}

	Count = m_GcRootMemberFieldArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CStructField* pField = m_GcRootMemberFieldArray [i];
		pField->GetType ()->GcMark (pRuntime, p + pField->GetOffset ());
	}
}

//.............................................................................

} // namespace jnc {

