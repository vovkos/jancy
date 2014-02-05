#include "pch.h"
#include "jnc_UnionType.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CUnionType::CUnionType ()
{
	m_TypeKind = EType_Union;
	m_Flags = ETypeFlag_Pod;
	m_pStructType = NULL;
	m_pInitializedField = NULL;
}

CStructField*
CUnionType::CreateFieldImpl (
	const rtl::CString& Name,
	CType* pType,
	size_t BitCount,
	uint_t PtrTypeFlags,
	rtl::CBoxListT <CToken>* pConstructor,
	rtl::CBoxListT <CToken>* pInitializer
	)
{
	CStructField* pField = AXL_MEM_NEW (CStructField);
	pField->m_Name = Name;
	pField->m_pParentNamespace = this;
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
		if (m_pInitializedField)
		{
			err::SetFormatStringError (
				"'%s' already has initialized field '%s'",
				pType->GetTypeString ().cc (),
				m_pInitializedField->GetName ().cc ()
				);
			return NULL;
		}

		m_pInitializedField = pField;
	}

	m_FieldList.InsertTail (pField);

	if (Name.IsEmpty ())
	{
		m_UnnamedFieldArray.Append (pField);
	}
	else
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
CUnionType::GetFieldByIndex (size_t Index)
{
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
CUnionType::CalcLayout ()
{
	bool Result;

	if (m_pExtensionNamespace)
		ApplyExtensionNamespace ();

	Result = ResolveImportFields ();
	if (!Result)
		return false;

	CType* pLargestFieldType = NULL;
	size_t LargestAlignFactor = 0;

	rtl::CIteratorT <CStructField> Field = m_FieldList.GetHead ();
	for (size_t i = 0; Field; Field++, i++)
	{
		CStructField* pField = *Field;

		Result = pField->m_pType->EnsureLayout ();
		if (!Result)
			return false;

		uint_t FieldTypeFlags = pField->m_pType->GetFlags ();
		size_t FieldAlignFactor = pField->m_pType->GetAlignFactor ();

		if (!(FieldTypeFlags & ETypeFlag_Pod))
		{
			err::SetFormatStringError ("non-POD '%s' cannot be union member", pField->m_pType->GetTypeString ().cc ());
			return NULL;
		}

		if (pField->m_BitCount)
		{
			pField->m_pType = m_pModule->m_TypeMgr.GetBitFieldType (pField->m_pBitFieldBaseType, 0, pField->m_BitCount);
			if (!pField->m_pType)
				return false;
		}

		if (!pLargestFieldType || pField->m_pType->GetSize () > pLargestFieldType->GetSize ())
			pLargestFieldType = pField->m_pType;

		if (LargestAlignFactor < pField->m_pType->GetAlignFactor ())
			LargestAlignFactor = pField->m_pType->GetAlignFactor ();

		pField->m_LlvmIndex = i;
	}

	ASSERT (pLargestFieldType);

	m_pStructType->CreateField (pLargestFieldType);
	m_pStructType->m_AlignFactor = AXL_MIN (LargestAlignFactor, m_pStructType->m_PackFactor);

	Result = m_pStructType->EnsureLayout ();
	if (!Result)
		return false;

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
		(m_pStaticConstructor || m_pInitializedField))
	{
		Result = CreateDefaultMethod (EFunction_PreConstructor);
		if (!Result)
			return false;
	}

	if (!m_pConstructor && m_pPreConstructor)
	{
		Result = CreateDefaultMethod (EFunction_Constructor);
		if (!Result)
			return false;
	}

	m_Size = m_pStructType->GetSize ();
	m_AlignFactor = m_pStructType->GetAlignFactor ();
	return true;
}

bool
CUnionType::Compile ()
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
CUnionType::CompileDefaultPreConstructor ()
{
	ASSERT (m_pPreConstructor);

	bool Result;

	CValue ThisValue;
	m_pModule->m_FunctionMgr.InternalPrologue (m_pPreConstructor, &ThisValue, 1);

	Result = InitializeField (ThisValue);
	if (!Result)
		return false;

	m_pModule->m_FunctionMgr.InternalEpilogue ();
	return true;
}


bool
CUnionType::InitializeField (const CValue& ThisValue)
{
	ASSERT (m_pInitializedField);

	CValue FieldValue;
	return
		m_pModule->m_OperatorMgr.GetField (ThisValue, m_pInitializedField, NULL, &FieldValue) &&
		m_pModule->m_OperatorMgr.ParseInitializer (
			FieldValue,
			m_pParentUnit,
			m_pInitializedField->m_Constructor,
			m_pInitializedField->m_Initializer
			);
}

void
CUnionType::PrepareLlvmDiType ()
{
	m_LlvmDiType = m_pModule->m_LlvmDiBuilder.CreateEmptyUnionType (this);
	m_pModule->m_LlvmDiBuilder.SetUnionTypeBody (this);
}

//.............................................................................

} // namespace jnc {
