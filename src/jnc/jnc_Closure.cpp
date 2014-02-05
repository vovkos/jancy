#include "pch.h"
#include "jnc_Closure.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

size_t
CClosure::Append (const rtl::CConstBoxListT <CValue>& ArgValueList)
{
	ASSERT (!ArgValueList.IsEmpty ());

	rtl::CBoxIteratorT <CValue> InternalArg = m_ArgValueList.GetHead ();
	rtl::CBoxIteratorT <CValue> ExternalArg = ArgValueList.GetHead ();

	for (;;)
	{
		while (InternalArg && !InternalArg->IsEmpty ())
			InternalArg++;

		if (!InternalArg)
			break;

		*InternalArg = *ExternalArg;
		InternalArg++;
		ExternalArg++;

		if (!ExternalArg)
			return m_ArgValueList.GetCount ();
	}

	for (; ExternalArg; ExternalArg++)
		m_ArgValueList.InsertTail (*ExternalArg);

	return m_ArgValueList.GetCount ();
}

bool
CClosure::Apply (rtl::CBoxListT <CValue>* pArgValueList)
{
	if (m_ArgValueList.IsEmpty ())
		return true;

	rtl::CBoxIteratorT <CValue> ClosureArg = m_ArgValueList.GetHead ();
	rtl::CBoxIteratorT <CValue> TargetArg = pArgValueList->GetHead ();

	for (size_t i = 0; ClosureArg; ClosureArg++, i++)
	{
		if (!ClosureArg->IsEmpty ())
		{
			pArgValueList->InsertBefore (*ClosureArg, TargetArg);
		}
		else if (TargetArg)
		{
			TargetArg++;
		}
		else
		{
			err::SetFormatStringError ("closure call misses argument #%d", i + 1);
			return false;
		}
	}

	return true;
}

CType*
CClosure::GetClosureType (CType* pType)
{
	EType TypeKind = pType->GetTypeKind ();

	switch (TypeKind)
	{
	case EType_FunctionPtr:
	case EType_FunctionRef:
		return GetFunctionClosureType ((CFunctionPtrType*) pType);

	case EType_PropertyPtr:
	case EType_PropertyRef:
		return GetPropertyClosureType ((CPropertyPtrType*) pType);

	default:
		return pType;
	}
}

CFunctionPtrType*
CClosure::GetFunctionClosureType (CFunction* pFunction)
{
	if (!pFunction->IsOverloaded ())
		return GetFunctionClosureType (pFunction->GetType ()->GetFunctionPtrType (EType_FunctionRef, EFunctionPtrType_Thin));

	err::SetFormatStringError ("function overload closures are not implemented yet");
	return NULL;
}

bool
CClosure::GetArgTypeArray (
	CModule* pModule,
	rtl::CArrayT <CFunctionArg*>* pArgArray
	)
{
	bool Result;

	size_t ClosureArgCount = m_ArgValueList.GetCount ();
	size_t ArgCount = pArgArray->GetCount ();

	if (ClosureArgCount > ArgCount)
	{
		err::SetFormatStringError ("closure with %d arguments for function with %d arguments", ClosureArgCount, ArgCount);
		return NULL;
	}

	rtl::CBoxIteratorT <CValue> ClosureArg = m_ArgValueList.GetHead ();
	for (size_t i = 0; ClosureArg; ClosureArg++)
	{
		if (ClosureArg->IsEmpty ())
		{
			i++;
			continue;
		}

		ASSERT (i < ArgCount);

		Result = pModule->m_OperatorMgr.CheckCastKind (ClosureArg->GetType (), (*pArgArray) [i]->GetType ());
		if (!Result)
			return false;

		pArgArray->Remove (i);
		ArgCount--;
	}

	return true;
}

CFunctionPtrType*
CClosure::GetFunctionClosureType (CFunctionPtrType* pPtrType)
{
	bool Result;

	CModule* pModule = pPtrType->GetModule ();
	CFunctionType* pType = pPtrType->GetTargetType ();

	if (pType->GetFlags () & EFunctionTypeFlag_VarArg)
	{
		err::SetFormatStringError ("function closures cannot be applied to vararg functions");
		return NULL;
	}

	rtl::CArrayT <CFunctionArg*> ArgArray = pType->GetArgArray ();
	Result = GetArgTypeArray (pModule, &ArgArray);
	if (!Result)
		return NULL;

	CFunctionType* pClosureType = pModule->m_TypeMgr.GetFunctionType (
		pType->GetCallConv (),
		pType->GetReturnType (),
		ArgArray
		);

	return pClosureType->GetFunctionPtrType (
		pPtrType->GetTypeKind (),
		pPtrType->GetPtrTypeKind (),
		pPtrType->GetFlags ()
		);
}

CPropertyPtrType*
CClosure::GetPropertyClosureType (CPropertyPtrType* pPtrType)
{
	bool Result;

	CModule* pModule = pPtrType->GetModule ();
	CPropertyType* pType = pPtrType->GetTargetType ();
	CFunctionType* pGetterType = pType->GetGetterType ();
	CFunctionTypeOverload* pSetterType = pType->GetSetterType ();

	rtl::CArrayT <CFunctionArg*> ArgArray = pGetterType->GetArgArray ();
	Result = GetArgTypeArray (pModule, &ArgArray);
	if (!Result)
		return NULL;

	CFunctionType* pClosureGetterType = pModule->m_TypeMgr.GetFunctionType (
		pGetterType->GetCallConv (),
		pGetterType->GetReturnType (),
		ArgArray
		);

	CFunctionTypeOverload ClosureSetterType;

	size_t SetterCount = pSetterType->GetOverloadCount ();
	for (size_t i = 0; i < SetterCount; i++)
	{
		CFunctionType* pOverloadType = pSetterType->GetOverload (i);
		ASSERT (!pOverloadType->GetArgArray ().IsEmpty ());

		ArgArray.Append (pOverloadType->GetArgArray ().GetBack ());

		CFunctionType* pClosureOverloadType = pModule->m_TypeMgr.GetFunctionType (
			pOverloadType->GetCallConv (),
			pOverloadType->GetReturnType (),
			ArgArray
			);

		ArgArray.Pop ();

		Result = ClosureSetterType.AddOverload (pClosureOverloadType);
		if (!Result)
			return NULL;
	}

	CPropertyType* pClosureType = pModule->m_TypeMgr.GetPropertyType (
		pClosureGetterType,
		ClosureSetterType,
		pType->GetFlags ()
		);

	return pClosureType->GetPropertyPtrType (
		pPtrType->GetTypeKind (),
		pPtrType->GetPtrTypeKind (),
		pPtrType->GetFlags ()
		);
}

//.............................................................................

} // namespace jnc {
