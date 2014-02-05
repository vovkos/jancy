#include "pch.h"
#include "jnc_OperatorMgr.h"
#include "jnc_Closure.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

bool
COperatorMgr::CreateClosureObject (
	EStorage StorageKind,
	const CValue& OpValue, // thin function or property ptr with closure
	CType* pThunkType, // function or property type
	bool IsWeak,
	CValue* pResultValue
	)
{
	ASSERT (pThunkType->GetTypeKind () == EType_Function || pThunkType->GetTypeKind () == EType_Property);

	bool Result;

	// choose reference function type

	CFunctionType* pSrcFunctionType;
	if (OpValue.GetType ()->GetTypeKindFlags () & ETypeKindFlag_FunctionPtr)
	{
		ASSERT (((CFunctionPtrType*) OpValue.GetType ())->GetPtrTypeKind () == EFunctionPtrType_Thin);
		pSrcFunctionType = ((CFunctionPtrType*) OpValue.GetType ())->GetTargetType ();
	}
	else
	{
		ASSERT (OpValue.GetType ()->GetTypeKindFlags () & ETypeKindFlag_PropertyPtr);
		ASSERT (((CPropertyPtrType*) OpValue.GetType ())->GetPtrTypeKind () == EPropertyPtrType_Thin);
		pSrcFunctionType = ((CPropertyPtrType*) OpValue.GetType ())->GetTargetType ()->GetGetterType ();
	}

	char Buffer1 [256];
	char Buffer2 [256];

	rtl::CArrayT <CType*> ClosureArgTypeArray (ref::EBuf_Stack, Buffer1, sizeof (Buffer1));
	rtl::CArrayT <size_t> ClosureMap (ref::EBuf_Stack, Buffer2, sizeof (Buffer2));
	size_t ClosureArgCount = 0;
	uint64_t WeakMask = 0;

	// build closure arg type array & closure map

	CClosure* pClosure = OpValue.GetClosure ();
	if (pClosure)
	{
		ClosureArgCount = pClosure->GetArgValueList ()->GetCount ();

		rtl::CArrayT <CFunctionArg*> SrcArgArray = pSrcFunctionType->GetArgArray ();
		size_t SrcArgCount = SrcArgArray.GetCount ();

		if (ClosureArgCount > SrcArgCount)
		{
			err::SetFormatStringError ("closure is too big for '%s'", pSrcFunctionType->GetTypeString ().cc ());
			return false;
		}

		ClosureArgTypeArray.SetCount (ClosureArgCount);
		ClosureMap.SetCount (ClosureArgCount);

		rtl::CBoxIteratorT <CValue> ClosureArgValue = pClosure->GetArgValueList ()->GetHead ();

		size_t j = 0;

		for (size_t i = 0; i < ClosureArgCount; ClosureArgValue++, i++)
		{
			if (ClosureArgValue->IsEmpty ())
				continue;

			CType* pType = ClosureArgValue->GetType ();
			if (IsWeakPtrType (pType))
				WeakMask |= (uint64_t) 2 << j; // account for function ptr field (at idx 0)

			ClosureArgTypeArray [j] = SrcArgArray [i]->GetType ();
			ClosureMap [j] = i;
			j++;
		}

		ClosureArgCount = j; // account for possible skipped args
	}

	// find or create closure class type

	CClosureClassType* pClosureType;

	if (pThunkType->GetTypeKind () == EType_Function)
	{
		pClosureType = m_pModule->m_TypeMgr.GetFunctionClosureClassType (
			((CFunctionPtrType*) OpValue.GetType ())->GetTargetType (),
			(CFunctionType*) pThunkType,
			ClosureArgTypeArray,
			ClosureMap,
			ClosureArgCount,
			WeakMask
			);
	}
	else
	{
		ASSERT (pThunkType->GetTypeKind () == EType_Property);

		pClosureType = m_pModule->m_TypeMgr.GetPropertyClosureClassType (
			((CPropertyPtrType*) OpValue.GetType ())->GetTargetType (),
			(CPropertyType*) pThunkType,
			ClosureArgTypeArray,
			ClosureMap,
			ClosureArgCount,
			WeakMask
			);
	}

	// create instance

	CValue ClosureValue;
	Result = m_pModule->m_OperatorMgr.NewOperator (StorageKind, pClosureType, NULL, &ClosureValue);
	if (!Result)
		return false;

	rtl::CIteratorT <CStructField> Field = pClosureType->GetFieldList ().GetHead ();

	// save function/property pointer

	CValue PfnValue = OpValue;
	PfnValue.ClearClosure ();

	CValue FieldValue;
	Result =
		GetClassField (ClosureValue, *Field, NULL, &FieldValue) &&
		BinaryOperator (EBinOp_Assign, FieldValue, PfnValue);

	if (!Result)
		return false;

	Field++;

	// save closure arguments (if any)

	if (pClosure)
	{
		rtl::CBoxIteratorT <CValue> ClosureArgValue = pClosure->GetArgValueList ()->GetHead ();
		for (; ClosureArgValue; ClosureArgValue++)
		{
			if (ClosureArgValue->IsEmpty ())
				continue;

			ASSERT (Field);

			CValue FieldValue;
			Result =
				GetClassField (ClosureValue, *Field, NULL, &FieldValue) &&
				BinaryOperator (EBinOp_Assign, FieldValue, *ClosureArgValue);

			if (!Result)
				return false;

			Field++;
		}
	}

	*pResultValue = ClosureValue;
	return true;
}

bool
COperatorMgr::CreateDataClosureObject (
	EStorage StorageKind,
	const CValue& OpValue, // data ptr
	CPropertyType* pThunkType, // function or property type
	CValue* pResultValue
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_DataPtr);

	bool Result;

	// find or create closure class type

	CDataClosureClassType* pClosureType = m_pModule->m_TypeMgr.GetDataClosureClassType (
		((CDataPtrType*) OpValue.GetType ())->GetTargetType (),
		pThunkType
		);

	// create instance

	CValue ClosureValue;
	Result = m_pModule->m_OperatorMgr.NewOperator (StorageKind, pClosureType, NULL, &ClosureValue);
	if (!Result)
		return false;

	rtl::CIteratorT <CStructField> Field = pClosureType->GetFieldList ().GetHead ();

	// save data pointer

	CValue FieldValue;
	Result =
		GetClassField (ClosureValue, *Field, NULL, &FieldValue) &&
		BinaryOperator (EBinOp_Assign, FieldValue, OpValue);

	if (!Result)
		return false;

	*pResultValue = ClosureValue;
	return true;
}

//.............................................................................

} // namespace jnc {
