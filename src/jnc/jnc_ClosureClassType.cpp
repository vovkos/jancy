#include "pch.h"
#include "jnc_ClosureClassType.h"
#include "jnc_Module.h"
#include "jnc_Runtime.h"
#include "jnc_StdLib.h"

namespace jnc {

//.............................................................................

rtl::CString
CClosureClassType::CreateSignature (
	CType* pTargetType, // function or property
	CType* pThunkType, // function or property
	CType* const* ppArgTypeArray,
	const size_t* pClosureMap,
	size_t ArgCount,
	uint64_t WeakMask
	)
{
	rtl::CString Signature = "CF";

	if (WeakMask)
		Signature.AppendFormat ("W%x-", WeakMask);

	Signature.AppendFormat (
		"%s-%s(",
		pTargetType->GetSignature ().cc (),
		pThunkType->GetSignature ().cc ()
		);

	for (size_t i = 0; i < ArgCount; i++)
		Signature.AppendFormat ("%d:%s", pClosureMap [i], ppArgTypeArray [i]->GetSignature ().cc ());

	Signature.Append (')');
	return Signature;
}

void
CClosureClassType::BuildArgValueList (
	const CValue& ClosureValue,
	const CValue* pThunkArgValueArray,
	size_t ThunkArgCount,
	rtl::CBoxListT <CValue>* pArgValueList
	)
{
	rtl::CIteratorT <CStructField> Field = GetFieldList ().GetHead ();
	Field++; // skip function / property ptr

	size_t iClosure = 0;
	size_t iThunk = 1; // skip 'this' arg

	// part 1 -- arguments come both from closure and from thunk

	for (size_t i = 0; Field; i++)
	{
		CValue ArgValue;

		if (i == m_ClosureMap [iClosure])
		{
			m_pModule->m_OperatorMgr.GetClassField (ClosureValue, *Field, NULL, &ArgValue);
			Field++;
			iClosure++;
		}
		else
		{
			ArgValue = pThunkArgValueArray [iThunk];
			iThunk++;
		}

		pArgValueList->InsertTail (ArgValue);
	}

	// part 2 -- arguments come from thunk only

	for (; iThunk < ThunkArgCount; iThunk++)
		pArgValueList->InsertTail (pThunkArgValueArray [iThunk]);
}

jnc::TIfaceHdr*
CClosureClassType::Strengthen (jnc::TIfaceHdr* p)
{
	if (!m_WeakMask)
		return p;

	size_t Count = m_pIfaceStructType->GetFieldList ().GetCount ();

	uint64_t WeakMask = m_WeakMask;
	while (WeakMask)
	{
		size_t Index = rtl::GetLoBitIdx64 (WeakMask);

		CStructField* pField = GetFieldByIndex (Index);
		ASSERT (pField && (pField->GetFlags () & EStructFieldFlag_WeakMasked));

		// only strengthen if source arg is weak, but target arg is strong

		jnc::TIfaceHdr* pWeakPtr = NULL;

		void* p2 = (char*) p + pField->GetOffset ();

		CType* pType = pField->GetType ();
		EType TypeKind = pType->GetTypeKind ();

		switch (TypeKind)
		{
		case EType_ClassPtr:
			if (((CClassPtrType*) pType)->GetPtrTypeKind () == EClassPtrType_Normal)
				pWeakPtr = *(jnc::TIfaceHdr**) p2;

			break;

		case EType_FunctionPtr:
			if (((CFunctionPtrType*) pType)->GetPtrTypeKind () == EFunctionPtrType_Normal)
				pWeakPtr = ((jnc::TFunctionPtr*) p2)->m_pClosure;

			break;

		case EType_PropertyPtr:
			if (((CPropertyPtrType*) pType)->GetPtrTypeKind () == EPropertyPtrType_Normal)
				pWeakPtr = ((jnc::TPropertyPtr*) p2)->m_pClosure;

			break;
		}

		if (pWeakPtr && !CStdLib::StrengthenClassPtr (pWeakPtr))
			return NULL;

		WeakMask &= ~(1 << Index);
	}

	return p;
}

//.............................................................................

CFunctionClosureClassType::CFunctionClosureClassType ()
{
	m_ClassTypeKind = EClassType_FunctionClosure;
	m_pThunkFunction = NULL;
}

bool
CFunctionClosureClassType::Compile ()
{
	ASSERT (m_pThunkFunction);

	bool Result = CClassType::Compile ();
	if (!Result)
		return false;

	size_t ArgCount = m_pThunkFunction->GetType ()->GetArgArray ().GetCount ();

	char Buffer [256];
	rtl::CArrayT <CValue> ArgValueArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	ArgValueArray.SetCount (ArgCount);

	m_pModule->m_FunctionMgr.InternalPrologue (m_pThunkFunction, ArgValueArray, ArgCount);

	CValue ThisValue = m_pModule->m_FunctionMgr.GetThisValue ();
	ASSERT (ThisValue);

	CValue PfnValue;
	m_pModule->m_OperatorMgr.GetClassField (ThisValue, *GetFieldList ().GetHead (), NULL, &PfnValue);

	rtl::CBoxListT <CValue> ArgValueList;
	BuildArgValueList (ThisValue, ArgValueArray, ArgCount, &ArgValueList);

	CValue ReturnValue;
	Result = m_pModule->m_OperatorMgr.CallOperator (PfnValue, &ArgValueList, &ReturnValue);
	if (!Result)
		return false;

	if (m_pThunkFunction->GetType ()->GetReturnType ()->GetTypeKind () != EType_Void)
	{
		Result = m_pModule->m_ControlFlowMgr.Return (ReturnValue);
		if (!Result)
			return false;
	}

	m_pModule->m_FunctionMgr.InternalEpilogue ();
	return true;
}

//.............................................................................

CPropertyClosureClassType::CPropertyClosureClassType ()
{
	m_ClassTypeKind = EClassType_PropertyClosure;
	m_pThunkProperty = NULL;
}

bool
CPropertyClosureClassType::Compile ()
{
	ASSERT (m_pThunkProperty);

	bool Result = CClassType::Compile ();
	if (!Result)
		return false;

	CFunction* pGetter = m_pThunkProperty->GetGetter ();
	CFunction* pSetter = m_pThunkProperty->GetSetter ();
	CFunction* pBinder = m_pThunkProperty->GetBinder ();

	if (pBinder)
	{
		Result = CompileAccessor (pBinder);
		if (!Result)
			return false;
	}

	Result = CompileAccessor (pGetter);
	if (!Result)
		return false;

	if (pSetter)
	{
		size_t OverloadCount = pSetter->GetOverloadCount ();

		for (size_t i = 0; i < OverloadCount; i++)
		{
			CFunction* pOverload = pSetter->GetOverload (i);

			Result = CompileAccessor (pOverload);
			if (!Result)
				return false;
		}
	}

	return true;
}

bool
CPropertyClosureClassType::CompileAccessor (CFunction* pAccessor)
{
	ASSERT (pAccessor->GetProperty () == m_pThunkProperty);

	bool Result;

	size_t ArgCount = pAccessor->GetType ()->GetArgArray ().GetCount ();

	char Buffer [256];
	rtl::CArrayT <CValue> ArgValueArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	ArgValueArray.SetCount (ArgCount);

	m_pModule->m_FunctionMgr.InternalPrologue (pAccessor, ArgValueArray, ArgCount);

	CValue ThisValue = m_pModule->m_FunctionMgr.GetThisValue ();
	ASSERT (ThisValue);

	CValue PropertyPtrValue;
	Result = m_pModule->m_OperatorMgr.GetClassField (ThisValue, *GetFieldList ().GetHead (), NULL, &PropertyPtrValue);
	ASSERT (Result);

	CValue PfnValue;

	EFunction AccessorKind = pAccessor->GetFunctionKind ();
	switch (AccessorKind)
	{
	case EFunction_Binder:
		Result = m_pModule->m_OperatorMgr.GetPropertyBinder (PropertyPtrValue, &PfnValue);
		break;

	case EFunction_Getter:
		Result = m_pModule->m_OperatorMgr.GetPropertyGetter (PropertyPtrValue, &PfnValue);
		break;

	case EFunction_Setter:
		Result = m_pModule->m_OperatorMgr.GetPropertySetter (PropertyPtrValue, ArgValueArray [ArgCount - 1], &PfnValue);
		break;

	default:
		err::SetFormatStringError ("invalid property accessor '%s' in property closure", GetFunctionKindString (AccessorKind));
		return false;
	}

	if (!Result)
		return false;

	rtl::CBoxListT <CValue> ArgValueList;
	BuildArgValueList (ThisValue, ArgValueArray, ArgCount, &ArgValueList);

	CValue ReturnValue;
	Result = m_pModule->m_OperatorMgr.CallOperator (PfnValue, &ArgValueList, &ReturnValue);
	if (!Result)
		return false;

	if (pAccessor->GetType ()->GetReturnType ()->GetTypeKind () != EType_Void)
	{
		Result = m_pModule->m_ControlFlowMgr.Return (ReturnValue);
		if (!Result)
			return false;
	}

	m_pModule->m_FunctionMgr.InternalEpilogue ();
	return true;
}

//.............................................................................

CDataClosureClassType::CDataClosureClassType ()
{
	m_ClassTypeKind = EClassType_DataClosure;
	m_pThunkProperty = NULL;
}

rtl::CString
CDataClosureClassType::CreateSignature (
	CType* pTargetType,
	CPropertyType* pThunkType
	)
{
	rtl::CString Signature = "CD";

	Signature.AppendFormat (
		"%s-%s",
		pTargetType->GetTypeString ().cc (),
		pThunkType->GetTypeString ().cc ()
		);

	return Signature;
}

bool
CDataClosureClassType::Compile ()
{
	ASSERT (m_pThunkProperty);

	bool Result = CClassType::Compile ();
	if (!Result)
		return false;

	CFunction* pGetter = m_pThunkProperty->GetGetter ();
	CFunction* pSetter = m_pThunkProperty->GetSetter ();

	Result = CompileGetter (pGetter);
	if (!Result)
		return false;

	if (pSetter)
	{
		size_t OverloadCount = pSetter->GetOverloadCount ();

		for (size_t i = 0; i < OverloadCount; i++)
		{
			CFunction* pOverload = pSetter->GetOverload (i);

			Result = CompileSetter (pOverload);
			if (!Result)
				return false;
		}
	}

	return true;
}

bool
CDataClosureClassType::CompileGetter (CFunction* pGetter)
{
	m_pModule->m_FunctionMgr.InternalPrologue (pGetter);

	CValue ThisValue = m_pModule->m_FunctionMgr.GetThisValue ();
	ASSERT (ThisValue);

	CValue PtrValue;

	bool Result =
		m_pModule->m_OperatorMgr.GetClassField (ThisValue, *GetFieldList ().GetHead (), NULL, &PtrValue) &&
		m_pModule->m_OperatorMgr.UnaryOperator (EUnOp_Indir, &PtrValue) &&
		m_pModule->m_ControlFlowMgr.Return (PtrValue);

	if (!Result)
		return false;

	m_pModule->m_FunctionMgr.InternalEpilogue ();
	return true;
}

bool
CDataClosureClassType::CompileSetter (CFunction* pSetter)
{
	CValue ArgValue;
	m_pModule->m_FunctionMgr.InternalPrologue (pSetter, &ArgValue, 1);

	CValue ThisValue = m_pModule->m_FunctionMgr.GetThisValue ();
	ASSERT (ThisValue);

	CValue PtrValue;

	bool Result =
		m_pModule->m_OperatorMgr.GetClassField (ThisValue, *GetFieldList ().GetHead (), NULL, &PtrValue) &&
		m_pModule->m_OperatorMgr.UnaryOperator (EUnOp_Indir, &PtrValue) &&
		m_pModule->m_OperatorMgr.StoreDataRef (PtrValue, ArgValue);

	if (!Result)
		return false;

	m_pModule->m_FunctionMgr.InternalEpilogue ();
	return true;
}

//.............................................................................

} // namespace jnc {
