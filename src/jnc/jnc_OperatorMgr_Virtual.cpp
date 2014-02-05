#include "pch.h"
#include "jnc_OperatorMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

bool
COperatorMgr::GetClassVTable (
	const CValue& OpValue,
	CClassType* pClassType,
	CValue* pResultValue
	)
{
	int32_t LlvmIndexArray [] =
	{
		0, // class.iface*
		0, // class.iface.hdr*
		0, // class.vtbl**
	};

	CValue PtrValue;
	m_pModule->m_LlvmIrBuilder.CreateGep (
		OpValue,
		LlvmIndexArray,
		countof (LlvmIndexArray),
		NULL,
		&PtrValue
		);

	// class.vtbl*

	CDataPtrType* pResultType = pClassType->GetVTableStructType ()->GetDataPtrType_c ();
	m_pModule->m_LlvmIrBuilder.CreateLoad (PtrValue, pResultType, pResultValue);
	return true;
}

bool
COperatorMgr::GetVirtualMethod (
	CFunction* pFunction,
	CClosure* pClosure,
	CValue* pResultValue
	)
{
	ASSERT (pFunction->IsVirtual ());

	if (!pClosure || !pClosure->IsMemberClosure ())
	{
		err::SetFormatStringError ("virtual method requires an object pointer");
		return false;
	}

	CValue Value = *pClosure->GetArgValueList ()->GetHead ();
	CClassType* pClassType = ((CClassPtrType*) Value.GetType ())->GetTargetType ();
	CClassType* pVTableType = pFunction->GetVirtualOriginClassType ();
	size_t VTableIndex = pFunction->GetClassVTableIndex ();

	CBaseTypeCoord Coord;
	pClassType->FindBaseTypeTraverse (pVTableType, &Coord);
	VTableIndex += Coord.m_VTableIndex;

	// class.vtbl*

	CValue PtrValue;
	GetClassVTable (Value, pClassType, &PtrValue);

	// pf*

	m_pModule->m_LlvmIrBuilder.CreateGep2 (
		PtrValue,
		VTableIndex,
		NULL,
		&PtrValue
		);

	// pf

	m_pModule->m_LlvmIrBuilder.CreateLoad (
		PtrValue,
		NULL,
		&PtrValue
		);

	pResultValue->SetLlvmValue (
		PtrValue.GetLlvmValue (),
		pFunction->GetType ()->GetFunctionPtrType (EFunctionPtrType_Thin)
		);

	pResultValue->SetClosure (pClosure);
	return true;
}

bool
COperatorMgr::GetVirtualProperty (
	CProperty* pProperty,
	CClosure* pClosure,
	CValue* pResultValue
	)
{
	ASSERT (pProperty->IsVirtual ());

	if (!pClosure || !pClosure->IsMemberClosure ())
	{
		err::SetFormatStringError ("virtual method requires an object pointer");
		return false;
	}

	CValue Value = *pClosure->GetArgValueList ()->GetHead ();
	CClassType* pClassType = ((CClassPtrType*) Value.GetType ())->GetTargetType ();
	size_t VTableIndex = pProperty->GetParentClassVTableIndex ();

	CBaseTypeCoord Coord;
	pClassType->FindBaseTypeTraverse (pProperty->GetParentType (), &Coord);
	VTableIndex += Coord.m_VTableIndex;

	// class.vtbl*

	CValue PtrValue;
	GetClassVTable (Value, pClassType, &PtrValue);

	// property.vtbl*

	m_pModule->m_LlvmIrBuilder.CreateGep2 (
		PtrValue,
		VTableIndex,
		NULL,
		&PtrValue
		);

	m_pModule->m_LlvmIrBuilder.CreateBitCast (
		PtrValue,
		pProperty->GetType ()->GetVTableStructType ()->GetDataPtrType_c (),
		&PtrValue
		);

	pResultValue->OverrideType (PtrValue, pProperty->GetType ()->GetPropertyPtrType (EPropertyPtrType_Thin));
	pResultValue->SetClosure (pClosure);
	return true;
}

//.............................................................................

} // namespace jnc {
