#include "pch.h"
#include "jnc_Property.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CProperty::CProperty ()
{
	m_ItemKind = EModuleItem_Property;
	m_NamespaceKind = ENamespace_Property;
	m_PropertyKind = EProperty_Normal;
	m_pItemDecl = this;
	m_pType = NULL;

	m_pPreConstructor = NULL;
	m_pConstructor = NULL;
	m_pDefaultConstructor = NULL;
	m_pStaticConstructor = NULL;
	m_pDestructor = NULL;
	m_pStaticDestructor = NULL;
	m_pGetter = NULL;
	m_pSetter = NULL;
	m_pBinder = NULL;

	m_pAutoGetValue = NULL;
	m_pOnChanged = NULL;

	m_pParentType = NULL;
	m_ParentClassVTableIndex = -1;
}

CFunction*
CProperty::GetDefaultConstructor ()
{
	ASSERT (m_pConstructor);
	if (m_pDefaultConstructor)
		return m_pDefaultConstructor;

	// avoid allocations

	rtl::CBoxListEntryT <CValue> ThisArgValue;
	rtl::CAuxListT <rtl::CBoxListEntryT <CValue> > ArgList;

	if (m_pParentType)
	{
		CType* pThisArgType = m_pParentType->GetThisArgType (EPtrTypeFlag_Checked);
		ThisArgValue.m_Value.SetType (pThisArgType);
		ArgList.InsertTail (&ThisArgValue);
	}

	m_pDefaultConstructor = m_pConstructor->ChooseOverload (ArgList);
	if (!m_pDefaultConstructor)
	{
		err::SetFormatStringError ("'%s' has no default constructor", m_Tag.cc ()); // thanks a lot gcc
		return NULL;
	}

	return m_pDefaultConstructor;
}

bool
CProperty::Compile ()
{
	bool Result;

	if (m_Flags & EPropertyFlag_AutoGet)
	{
		Result = CompileAutoGetter ();
		if (!Result)
			return false;
	}

	if (m_Flags & EPropertyFlag_AutoSet)
	{
		Result = CompileAutoSetter ();
		if (!Result)
			return false;
	}

	if (m_pType->GetFlags () & EPropertyTypeFlag_Bindable)
	{
		Result = CompileBinder ();
		if (!Result)
			return false;
	}

	return true;
}

bool
CProperty::Create (CPropertyType* pType)
{
	bool Result;

	EStorage StorageKind = m_StorageKind == EStorage_Abstract ? EStorage_Virtual : m_StorageKind;

	uint_t GetterFlags = 0;
	uint_t SetterFlags = 0;

	if (m_Flags & EModuleItemFlag_User)
	{
		if (!(m_Flags & EPropertyFlag_AutoGet))
			GetterFlags |= EModuleItemFlag_User;

		if (!(m_Flags & EPropertyFlag_AutoSet))
			SetterFlags |= EModuleItemFlag_User;
	}

	if (pType->GetFlags () & EPropertyTypeFlag_Bindable)
	{
		Result = CreateOnChanged ();
		if (!Result)
			return false;
	}

	CFunctionType* pGetterType = pType->GetGetterType ();

	if (m_Flags & EPropertyFlag_AutoGet)
	{
		Result = CreateAutoGetValue (pGetterType->GetReturnType ());
		if (!Result)
			return false;
	}
	else
	{
		CFunction* pGetter = m_pModule->m_FunctionMgr.CreateFunction (EFunction_Getter, pGetterType);
		pGetter->m_StorageKind = StorageKind;
		pGetter->m_Flags |= GetterFlags;

		if (m_pParentType)
			pGetter->m_ThisArgTypeFlags = EPtrTypeFlag_Const;

		Result = AddMethod (pGetter);
		if (!Result)
			return false;
	}

	size_t SetterTypeOverloadCount = pType->GetSetterType ()->GetOverloadCount ();
	for (size_t i = 0; i < SetterTypeOverloadCount; i++)
	{
		CFunctionType* pSetterType = pType->GetSetterType ()->GetOverload (i);
		CFunction* pSetter = m_pModule->m_FunctionMgr.CreateFunction (EFunction_Setter, pSetterType);
		pSetter->m_StorageKind = StorageKind;
		pSetter->m_Flags |= SetterFlags;

		Result = AddMethod (pSetter);
		if (!Result)
			return false;
	}

	m_pType = m_pParentType ? m_pParentType->GetMemberPropertyType (pType) : pType;

	if (m_Flags & (EPropertyFlag_AutoGet | EPropertyFlag_AutoSet))
		m_pModule->MarkForCompile (this);

	return true;
}

void
CProperty::ConvertToMemberProperty (CNamedType* pParentType)
{
	ASSERT (!m_pParentType);
	m_pParentType = pParentType;
	m_pType = pParentType->GetMemberPropertyType (m_pType);
}

bool
CProperty::SetOnChanged (CModuleItem* pItem)
{
	if (m_pOnChanged)
	{
		err::SetFormatStringError ("'%s' already has 'bindable %s'", m_Tag.cc (), m_pOnChanged->m_Tag.cc ());
		return false;
	}

	CType* pType = GetModuleItemType (pItem);
	if (!pType)
	{
		err::SetFormatStringError ("invalid bindable item");
		return false;
	}

	m_pOnChanged = pItem;
	m_Flags |= EPropertyFlag_Bindable;

	CFunctionType* pBinderType = (CFunctionType*) m_pModule->GetSimpleType (EStdType_Binder);
	CFunction* pBinder = m_pModule->m_FunctionMgr.CreateFunction (EFunction_Binder, pBinderType);
	pBinder->m_StorageKind = m_StorageKind == EStorage_Abstract ? EStorage_Virtual : m_StorageKind;

	if (m_pParentType)
		pBinder->m_ThisArgTypeFlags = EPtrTypeFlag_Const;

	m_pModule->MarkForCompile (this);

	return AddMethod (pBinder);
}

bool
CProperty::CreateOnChanged ()
{
	rtl::CString Name = "m_onChanged";

	CType* pType = m_pModule->GetSimpleType (EStdType_SimpleMulticast);

	if (m_pParentType)
	{
		CStructField* pField = CreateField (Name, pType);
		return
			pField != NULL &&
			SetOnChanged (pField);
	}
	else
	{
		CVariable* pVariable = m_pModule->m_VariableMgr.CreateVariable (
			EStorage_Static,
			Name,
			CreateQualifiedName (Name),
			pType
			);

		return
			pVariable != NULL &&
			AddItem (pVariable) &&
			SetOnChanged (pVariable);
	}
}

bool
CProperty::SetAutoGetValue (CModuleItem* pItem)
{
	if (m_pAutoGetValue)
	{
		err::SetFormatStringError ("'%s' already has 'autoget %s'", m_Tag.cc (), m_pAutoGetValue->m_Tag.cc ());
		return false;
	}

	CType* pType = GetModuleItemType (pItem);
	if (!pType)
	{
		err::SetFormatStringError ("invalid autoget item");
		return false;
	}

	m_pAutoGetValue = pItem;
	m_Flags |= EPropertyFlag_AutoGet;

	CFunctionType* pGetterType = m_pModule->m_TypeMgr.GetFunctionType (pType, NULL, 0, 0);

	if (m_pGetter)
	{
		if (m_pGetter->GetType ()->GetReturnType ()->Cmp (pType) != 0)
		{
			err::SetFormatStringError ("'autoget %s' does not match property declaration", pType->GetTypeString ().cc ());
			return false;
		}

		return true;
	}

	CFunction* pGetter = m_pModule->m_FunctionMgr.CreateFunction (EFunction_Getter, pGetterType);
	pGetter->m_StorageKind = m_StorageKind == EStorage_Abstract ? EStorage_Virtual : m_StorageKind;

	if (m_pParentType)
		pGetter->m_ThisArgTypeFlags = EPtrTypeFlag_Const;

	m_pModule->MarkForCompile (this);

	return AddMethod (pGetter);
}


bool
CProperty::CreateAutoGetValue (CType* pType)
{
	rtl::CString Name = "m_value";

	if (m_pParentType)
	{
		CStructField* pField = CreateField (Name, pType);
		return
			pField != NULL &&
			SetAutoGetValue (pField);
	}
	else
	{
		CVariable* pVariable = m_pModule->m_VariableMgr.CreateVariable (
			EStorage_Static,
			Name,
			CreateQualifiedName (Name),
			pType
			);

		return
			pVariable != NULL &&
			AddItem (pVariable) &&
			SetAutoGetValue (pVariable);
	}
}

CStructField*
CProperty::CreateField (
	const rtl::CString& Name,
	CType* pType,
	size_t BitCount,
	uint_t PtrTypeFlags,
	rtl::CBoxListT <CToken>* pConstructor,
	rtl::CBoxListT <CToken>* pInitializer
	)
{
	ASSERT (m_pParentType);

	if (!(m_pParentType->GetTypeKindFlags () & ETypeKindFlag_Derivable))
	{
		err::SetFormatStringError ("'%s' cannot have field members", m_pParentType->GetTypeString ().cc ());
		return NULL;
	}

	CDerivableType* pParentType = (CDerivableType*) m_pParentType;

	bool Result;

	// don't add field to parent namespace

	CStructField* pField = pParentType->CreateField (rtl::CString (), pType, BitCount, PtrTypeFlags, pConstructor, pInitializer);
	if (!pField)
		return NULL;

	// re-parent

	pField->m_pParentNamespace = this;
	pField->m_Name = Name;

	if (!Name.IsEmpty ())
	{
		Result = AddItem (pField);
		if (!Result)
			return NULL;
	}

	return pField;
}

bool
CProperty::AddMethod (CFunction* pFunction)
{
	bool Result;

	EStorage StorageKind = pFunction->GetStorageKind ();
	EFunction FunctionKind = pFunction->GetFunctionKind ();
	uint_t FunctionKindFlags = GetFunctionKindFlags (FunctionKind);
	uint_t ThisArgTypeFlags = pFunction->m_ThisArgTypeFlags;

	if (m_pParentType)
	{
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
			if (FunctionKind == EFunction_Getter)
				pFunction->m_ThisArgTypeFlags |= EPtrTypeFlag_Const;

			pFunction->ConvertToMemberMethod (m_pParentType);
			break;

		case EStorage_Abstract:
		case EStorage_Virtual:
		case EStorage_Override:
			if (FunctionKind == EFunction_Getter)
				pFunction->m_ThisArgTypeFlags |= EPtrTypeFlag_Const;

			if (m_pParentType->GetTypeKind () != EType_Class)
			{
				err::SetFormatStringError (
					"'%s' method cannot be part of '%s'",
					GetStorageKindString (StorageKind),
					m_pParentType->GetTypeString ().cc ()
					);
				return false;
			}

			if (!pFunction->IsAccessor ())
				((CClassType*) m_pParentType)->m_VirtualMethodArray.Append (pFunction); // otherwise we are already on VirtualPropertyArray

			pFunction->ConvertToMemberMethod (m_pParentType);
			break;

		default:
			err::SetFormatStringError ("invalid storage specifier '%s' for method member", GetStorageKindString (StorageKind));
			return false;
		}
	}
	else
	{
		switch (StorageKind)
		{
		case EStorage_Undefined:
			pFunction->m_StorageKind = EStorage_Static;
			// and fall through

		case EStorage_Static:
			break;

		default:
			err::SetFormatStringError ("invalid storage specifier '%s' for static property member", GetStorageKindString (StorageKind));
			return false;
		}

		if (ThisArgTypeFlags)
		{
			err::SetFormatStringError ("global property methods cannot be '%s'", GetPtrTypeFlagString (ThisArgTypeFlags).cc ());
			return false;
		}

	}

	pFunction->m_pParentNamespace = this;
	pFunction->m_pProperty = this;

	CFunction** ppTarget = NULL;

	switch (FunctionKind)
	{
	case EFunction_Constructor:
		ppTarget = &m_pConstructor;
		break;

	case EFunction_StaticConstructor:
		ppTarget = &m_pStaticConstructor;
		break;

	case EFunction_Destructor:
		ppTarget = &m_pDestructor;
		break;

	case EFunction_Getter:
		Result = m_Verifier.CheckGetter (pFunction->GetType ());
		if (!Result)
			return false;

		ppTarget = &m_pGetter;
		break;

	case EFunction_Setter:
		if (m_Flags & EPropertyFlag_Const)
		{
			err::SetFormatStringError ("const property '%s' cannot have setters", m_Tag.cc ());
			return false;
		}

		Result = m_Verifier.CheckSetter (pFunction->GetType ());
		if (!Result)
			return false;

		ppTarget = &m_pSetter;
		break;

	case EFunction_Binder:
		ppTarget = &m_pBinder;
		break;

	case EFunction_Named:
		return AddFunction (pFunction);

	default:
		err::SetFormatStringError (
			"invalid %s in '%s'",
			GetFunctionKindString (FunctionKind),
			m_Tag.cc ()
			);
		return false;
	}

	pFunction->m_Tag.Format ("%s.%s", m_Tag.cc (), GetFunctionKindString (FunctionKind));

	if (!*ppTarget)
	{
		*ppTarget = pFunction;
	}
	else
	{
		Result = (*ppTarget)->AddOverload (pFunction);
		if (!Result)
			return false;
	}

	return true;
}

bool
CProperty::AddProperty (CProperty* pProperty)
{
	ASSERT (pProperty->IsNamed ());
	bool Result = AddItem (pProperty);
	if (!Result)
		return false;

	pProperty->m_pParentNamespace = this;

	if (!m_pParentType)
		return true;

	EStorage StorageKind = pProperty->GetStorageKind ();
	switch (StorageKind)
	{
	case EStorage_Static:
		break;

	case EStorage_Undefined:
		pProperty->m_StorageKind = EStorage_Member;
		// and fall through

	case EStorage_Member:
		pProperty->m_pParentType = m_pParentType;
		break;

	case EStorage_Abstract:
	case EStorage_Virtual:
	case EStorage_Override:
		if (m_pParentType->GetTypeKind () != EType_Class)
		{
			err::SetFormatStringError (
				"'%s' property cannot be part of '%s'",
				GetStorageKindString (StorageKind),
				m_pParentType->GetTypeString ().cc ()
				);
			return false;
		}

		((CClassType*) m_pParentType)->m_VirtualPropertyArray.Append (pProperty);
		pProperty->m_pParentType = m_pParentType;
		break;

	default:
		err::SetFormatStringError ("invalid storage specifier '%s' for property member", GetStorageKindString (StorageKind));
		return false;
	}

	return true;
}

bool
CProperty::CallMemberFieldConstructors (const CValue& ThisValue)
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

		CType* pType = pField->GetType ();

		ASSERT (pType->GetTypeKindFlags () & ETypeKindFlag_Derivable);
		CFunction* pConstructor = ((CDerivableType*) pType)->GetDefaultConstructor ();
		if (!pConstructor)
			return false;

		CValue FieldValue;
		Result =
			m_pModule->m_OperatorMgr.GetClassField (ThisValue, pField, NULL, &FieldValue) &&
			m_pModule->m_OperatorMgr.CallOperator (pConstructor, FieldValue);

		if (!Result)
			return false;
	}

	return true;
}

bool
CProperty::CallMemberPropertyConstructors (const CValue& ThisValue)
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

		CFunction* pConstructor = pProperty->GetDefaultConstructor ();
		if (!pConstructor)
			return false;

		Result = m_pModule->m_OperatorMgr.CallOperator (pConstructor, ThisValue);
		if (!Result)
			return false;
	}

	return true;
}

bool
CProperty::CalcLayout ()
{
	bool Result;

	ASSERT (m_StorageKind && m_VTable.IsEmpty ());

	size_t SetterCount = m_pSetter ? m_pSetter->GetOverloadCount () : 0;

	m_VTable.Reserve (2 + SetterCount);

	if (m_pBinder)
	{
		Result = m_pBinder->GetType ()->EnsureLayout ();
		if (!Result)
			return false;

		m_VTable.Append (m_pBinder);
	}

	Result = m_pGetter->GetType ()->EnsureLayout ();
	if (!Result)
		return false;

	m_VTable.Append (m_pGetter);

	for (size_t i = 0; i < SetterCount; i++)
	{
		CFunction* pSetter = m_pSetter->GetOverload (i);
		Result = pSetter->GetType ()->EnsureLayout ();
		if (!Result)
			return false;

		m_VTable.Append (pSetter);
	}

	CreateVTablePtr ();
	return true;
}

void
CProperty::CreateVTablePtr ()
{
	char Buffer [256];
	rtl::CArrayT <llvm::Constant*> LlvmVTable (ref::EBuf_Stack, Buffer, sizeof (Buffer));

	size_t Count = m_VTable.GetCount ();
	LlvmVTable.SetCount (Count);

	for (size_t i = 0; i < Count; i++)
	{
		CFunction* pFunction = m_VTable [i];

		if (pFunction->GetStorageKind () == EStorage_Abstract)
			pFunction = pFunction->GetType ()->GetAbstractFunction ();

		LlvmVTable [i] = pFunction->GetLlvmFunction ();
	}

	CStructType* pVTableStructType = m_pType->GetVTableStructType ();

	llvm::Constant* pLlvmVTableConstant = llvm::ConstantStruct::get (
		(llvm::StructType*) pVTableStructType->GetLlvmType (),
		llvm::ArrayRef <llvm::Constant*> (LlvmVTable, Count)
		);

	llvm::GlobalVariable* pLlvmVTableVariable = new llvm::GlobalVariable (
		*m_pModule->GetLlvmModule (),
		pVTableStructType->GetLlvmType (),
		false,
		llvm::GlobalVariable::InternalLinkage,
		pLlvmVTableConstant,
		(const char*) (m_Tag + ".Vtbl")
		);

	m_VTablePtrValue.SetLlvmValue (
		pLlvmVTableVariable,
		pVTableStructType->GetDataPtrType_c (),
		EValue_Const
		);
}

bool
CProperty::CompileAutoGetter ()
{
	ASSERT (m_pGetter);

	bool Result;

	m_pModule->m_FunctionMgr.InternalPrologue (m_pGetter);

	CValue AutoGetValue;
	Result =
		m_pModule->m_OperatorMgr.GetPropertyAutoGetValue (GetAutoAccessorPropertyValue (), &AutoGetValue) &&
		m_pModule->m_ControlFlowMgr.Return (AutoGetValue);

	if (!Result)
		return false;

	m_pModule->m_FunctionMgr.InternalEpilogue ();
	return true;
}

bool
CProperty::CompileAutoSetter ()
{
	ASSERT (m_pSetter && !m_pSetter->IsOverloaded ());
	ASSERT (m_pType->GetFlags () & EPropertyTypeFlag_Bindable);

	bool Result;

	CValue SrcValue;

	if (IsMember ())
	{
		CValue ArgValueArray [2];
		m_pModule->m_FunctionMgr.InternalPrologue (m_pSetter, ArgValueArray, 2);
		SrcValue = ArgValueArray [1];
	}
	else
	{
		m_pModule->m_FunctionMgr.InternalPrologue (m_pSetter, &SrcValue, 1);
	}

	CBasicBlock* pAssignBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("assign_block");
	CBasicBlock* pReturnBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("return_block");

	CValue AutoGetValue;
	CValue CmpValue;

	Result =
		m_pModule->m_OperatorMgr.GetPropertyAutoGetValue (GetAutoAccessorPropertyValue (), &AutoGetValue) &&
		m_pModule->m_OperatorMgr.BinaryOperator (EBinOp_Ne, AutoGetValue, SrcValue, &CmpValue) &&
		m_pModule->m_ControlFlowMgr.ConditionalJump (CmpValue, pAssignBlock, pReturnBlock) &&
		m_pModule->m_OperatorMgr.StoreDataRef (AutoGetValue, SrcValue) &&
		m_pModule->m_FunctionMgr.FireOnChanged ();

	if (!Result)
		return false;

	m_pModule->m_ControlFlowMgr.Follow (pReturnBlock);
	m_pModule->m_FunctionMgr.InternalEpilogue ();
	return true;
}

bool
CProperty::CompileBinder ()
{
	ASSERT (m_pBinder);

	bool Result;

	m_pModule->m_FunctionMgr.InternalPrologue (m_pBinder);

	CValue OnChangedValue;
	Result =
		m_pModule->m_OperatorMgr.GetPropertyOnChanged (GetAutoAccessorPropertyValue (), &OnChangedValue) &&
		m_pModule->m_ControlFlowMgr.Return (OnChangedValue);

	if (!Result)
		return false;

	m_pModule->m_FunctionMgr.InternalEpilogue ();
	return true;
}

CValue
CProperty::GetAutoAccessorPropertyValue ()
{
	if (!IsMember ())
		return this;

	CValue Value = this;
	Value.InsertToClosureHead (m_pModule->m_FunctionMgr.GetThisValue ());
	return Value;
}

//.............................................................................

} // namespace jnc {
