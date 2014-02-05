#include "pch.h"
#include "jnc_EnumType.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CEnumType::CEnumType ()
{
	m_TypeKind = EType_Enum;
	m_EnumTypeKind = EEnumType_Normal;
	m_Flags = ETypeFlag_Pod;
	m_pBaseType = NULL;
	m_pBaseType_i = NULL;
}

CEnumConst*
CEnumType::CreateConst (
	const rtl::CString& Name,
	rtl::CBoxListT <CToken>* pInitializer
	)
{
	CEnumConst* pConst = AXL_MEM_NEW (CEnumConst);
	pConst->m_Name = Name;
	pConst->m_pParentEnumType = this;

	if (pInitializer)
		pConst->m_Initializer.TakeOver (pInitializer);

	m_ConstList.InsertTail (pConst);

	bool Result = AddItem (pConst);
	if (!Result)
		return NULL;

	return pConst;
}

bool
CEnumType::CalcLayout ()
{
	bool Result;

	if (m_pBaseType_i)
		m_pBaseType = m_pBaseType_i->GetActualType ();

	if (!(m_pBaseType->GetTypeKindFlags () & ETypeKindFlag_Integer))
	{
		err::SetFormatStringError ("enum base type must be integer type");
		return NULL;
	}

	m_Size = m_pBaseType->GetSize ();
	m_AlignFactor = m_pBaseType->GetAlignFactor ();

	// assign values to consts

	m_pModule->m_NamespaceMgr.OpenNamespace (this);
	CUnit* pUnit = m_pItemDecl->GetParentUnit ();

	if (m_EnumTypeKind == EEnumType_Flag)
	{
		intptr_t Value = 1;

		rtl::CIteratorT <CEnumConst> Const = m_ConstList.GetHead ();
		for (; Const; Const++)
		{
			if (!Const->m_Initializer.IsEmpty ())
			{
				Result = m_pModule->m_OperatorMgr.ParseConstIntegerExpression (
					pUnit,
					Const->m_Initializer,
					&Value
					);

				if (!Result)
					return false;
			}

			Const->m_Value = Value;

			uint8_t HiBitIdx = rtl::GetHiBitIdx (Value);
			Value = HiBitIdx != -1 ? 2 << HiBitIdx : 1;
		}
	}
	else
	{
		intptr_t Value = 0;

		rtl::CIteratorT <CEnumConst> Const = m_ConstList.GetHead ();
		for (; Const; Const++, Value++)
		{
			if (!Const->m_Initializer.IsEmpty ())
			{
				Result = m_pModule->m_OperatorMgr.ParseConstIntegerExpression (
					pUnit,
					Const->m_Initializer,
					&Value
					);

				if (!Result)
					return false;
			}

			Const->m_Value = Value;
		}
	}

	m_pModule->m_NamespaceMgr.CloseNamespace ();

	return true;
}

//.............................................................................

} // namespace jnc {
