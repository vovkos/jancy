#include "pch.h"
#include "jnc_Value.h"
#include "jnc_Value.h"
#include "jnc_Closure.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

class CLlvmPodArray: public llvm::ConstantDataSequential
{
public:
	static
	llvm::Constant*
	Get (
		CArrayType* pType,
		const void* p
		)
	{
		llvm::Type* pLlvmType = pType->GetLlvmType ();
		return getImpl (llvm::StringRef ((char*) p, pType->GetSize ()), pLlvmType);
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CLlvmPodStruct: public llvm::ConstantStruct
{
public:
	static
	llvm::Constant*
	Get (
		CStructType* pType,
		const void* p
		)
	{
		llvm::Type* pLlvmType = pType->GetLlvmType ();

		char Buffer [256];
		rtl::CArrayT <llvm::Constant*> LlvmMemberArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));

		rtl::CIteratorT <CStructField> Member = pType->GetFieldList ().GetHead ();
		for (; Member; Member++)
		{
			CValue MemberConst ((char*) p + Member->GetOffset (), Member->GetType ());
			LlvmMemberArray.Append ((llvm::Constant*) MemberConst.GetLlvmValue ());
		}

		return get (
			(llvm::StructType*) pLlvmType,
			llvm::ArrayRef <llvm::Constant*> (LlvmMemberArray, LlvmMemberArray.GetCount ())
			);
	}
};

//.............................................................................

const char*
GetValueKindString (EValue ValueKind)
{
	static const char* StringTable [EValue__Count] =
	{
		"void",                   // EValue_Void = 0,
		"null",                   // EValue_Null,
		"namespace",              // EValue_Namespace,
		"type",                   // EValue_Type,
		"const",                  // EValue_Const,
		"variable",               // EValue_Variable,
		"function",               // EValue_Function,
		"function-type-overload", // EValue_FunctionTypeOverload,
		"property",               // EValue_Property,
		"llvm-register",          // EValue_LlvmRegister,
		"bool-not",               // EValue_BoolNot,
		"bool-and",               // EValue_BoolAnd,
		"bool-or",                // EValue_BoolOr,
	};

	return (size_t) ValueKind < EValue__Count ?
		StringTable [ValueKind] :
		"undefined-value-kind";
}

//.............................................................................

void
CValue::Init ()
{
	m_ValueKind = EValue_Void;
	m_pType = NULL;
	m_pVariable = NULL;
	m_pLlvmValue = NULL;
}

void
CValue::Clear ()
{
	m_ValueKind = EValue_Void;
	m_pType = NULL;
	m_pItem = NULL;
	m_pLlvmValue = NULL;
	m_Closure = ref::EPtr_Null;
	m_LeanDataPtrValidator = ref::EPtr_Null;
}

llvm::Value*
CValue::GetLlvmValue () const
{
	if (m_pLlvmValue)
		return m_pLlvmValue;

	ASSERT (m_ValueKind == EValue_Const);

	m_pLlvmValue = GetLlvmConst (m_pType, GetConstData ());
	return m_pLlvmValue;
}

llvm::Constant*
GetLlvmPtrConst (
	CType* pType,
	const void* p
	)
{
	int64_t Integer = *(int64_t*) p;

	llvm::Constant* pLlvmConst = llvm::ConstantInt::get (
		pType->GetModule ()->m_TypeMgr.GetPrimitiveType (EType_Int_pu)->GetLlvmType (),
		llvm::APInt (sizeof (void*) * 8, Integer, false)
		);

	return llvm::ConstantExpr::getIntToPtr (pLlvmConst, pType->GetLlvmType ());
}

llvm::Constant*
CValue::GetLlvmConst (
	CType* pType,
	const void* p
	)
{
	int64_t Integer;
	double Double;
	llvm::Constant* pLlvmConst = NULL;

	if (pType->GetTypeKind () == EType_Enum)
		pType = ((CEnumType*) pType)->GetBaseType ();

	CModule* pModule = pType->GetModule ();

	EType TypeKind = pType->GetTypeKind ();
	switch (TypeKind)
	{
	case EType_Bool:
		Integer = *(int8_t*) p != 0;
		pLlvmConst = llvm::ConstantInt::get (
			pType->GetLlvmType (),
			llvm::APInt (1, Integer, !(pType->GetTypeKindFlags () & ETypeKindFlag_Unsigned))
			);
		break;

	case EType_Int8:
	case EType_Int8_u:
	case EType_Int16:
	case EType_Int16_u:
	case EType_Int32:
	case EType_Int32_u:
	case EType_Int64:
	case EType_Int64_u:
	case EType_Int16_be:
	case EType_Int16_beu:
	case EType_Int32_be:
	case EType_Int32_beu:
	case EType_Int64_be:
	case EType_Int64_beu:
		Integer = *(int64_t*) p;
		pLlvmConst = llvm::ConstantInt::get (
			pType->GetLlvmType (),
			llvm::APInt (pType->GetSize () * 8, Integer, !(pType->GetTypeKindFlags () & ETypeKindFlag_Unsigned))
			);
		break;

	case EType_Float:
		Double = *(float*) p;
		pLlvmConst = llvm::ConstantFP::get (pType->GetLlvmType (), Double);
		break;

	case EType_Double:
		Double = *(double*) p;
		pLlvmConst = llvm::ConstantFP::get (pType->GetLlvmType (), Double);
		break;

	case EType_Array:
		pLlvmConst = CLlvmPodArray::Get ((CArrayType*) pType, p);
		break;

	case EType_Struct:
		pLlvmConst = CLlvmPodStruct::Get ((CStructType*) pType, p);
		break;

	case EType_DataPtr:
	case EType_DataRef:
		if (((CDataPtrType*) pType)->GetPtrTypeKind () == EDataPtrType_Normal)
		{
			pLlvmConst = CLlvmPodStruct::Get (((CDataPtrType*) pType)->GetDataPtrStructType (), p);
		}
		else // thin or unsafe
		{
			pLlvmConst = GetLlvmPtrConst (pType, p);
		}
		break;

	case EType_ClassPtr:
		pLlvmConst = GetLlvmPtrConst (pType, p);
		break;

	default:
		ASSERT (false);
	}

	return pLlvmConst;
}

void
CValue::InsertToClosureHead (const CValue& Value)
{
	if (!m_Closure)
		m_Closure = AXL_REF_NEW (CClosure);

	m_Closure->GetArgValueList ()->InsertHead (Value);
}

void
CValue::InsertToClosureTail (const CValue& Value)
{
	if (!m_Closure)
		m_Closure = AXL_REF_NEW (CClosure);

	m_Closure->GetArgValueList ()->InsertTail (Value);
}

CClosure*
CValue::CreateClosure ()
{
	m_Closure = AXL_REF_NEW (CClosure);
	return m_Closure;
}

void
CValue::SetClosure (CClosure* pClosure)
{
	m_Closure = pClosure;
}

CType*
CValue::GetClosureAwareType () const
{
	return m_Closure ? m_Closure->GetClosureType (m_pType) : m_pType;
}

void
CValue::OverrideType (EType TypeKind)
{
	CModule* pModule = GetCurrentThreadModule ();
	ASSERT (pModule);

	CType* pType = pModule->m_TypeMgr.GetPrimitiveType (TypeKind);
	OverrideType (pType);
}

void
CValue::SetVoid ()
{
	CModule* pModule = GetCurrentThreadModule ();
	ASSERT (pModule);

	Clear ();

	m_ValueKind = EValue_Void;
	m_pType = pModule->m_TypeMgr.GetPrimitiveType (EType_Void);
}

void
CValue::SetNull ()
{
	CModule* pModule = GetCurrentThreadModule ();
	ASSERT (pModule);

	Clear ();

	m_ValueKind = EValue_Null;
	m_pType = pModule->m_TypeMgr.GetPrimitiveType (EType_Void);
}

void
CValue::SetType (CType* pType)
{
	Clear ();

	m_ValueKind = EValue_Type;
	m_pType = pType;
}

void
CValue::SetType (EType TypeKind)
{
	CModule* pModule = GetCurrentThreadModule ();
	ASSERT (pModule);

	CType* pType = pModule->m_TypeMgr.GetPrimitiveType (TypeKind);
	SetType (pType);
}

void
CValue::SetNamespace (CNamespace* pNamespace)
{
	Clear ();

	CModule* pModule = GetCurrentThreadModule ();
	ASSERT (pModule);

	m_ValueKind = EValue_Namespace;
	m_pNamespace = pNamespace;
	m_pType = pModule->m_TypeMgr.GetPrimitiveType (EType_Void);
}

void
CValue::SetVariable (CVariable* pVariable)
{
	Clear ();

	m_ValueKind = EValue_Variable;
	m_pLlvmValue = pVariable->GetLlvmValue ();
	m_pVariable = pVariable;

	uint_t PtrTypeFlags = pVariable->GetPtrTypeFlags () | EPtrTypeFlag_Safe;

	CType* pType = pVariable->GetType ();
	if (pType->GetTypeKind () == EType_Class)
		m_pType = ((CClassType*) pType)->GetClassPtrType (
			pVariable->GetParentNamespace (),
			EType_ClassRef,
			EClassPtrType_Normal,
			PtrTypeFlags
			);
	else
		m_pType = pType->GetDataPtrType (
			pVariable->GetParentNamespace (),
			EType_DataRef,
			EDataPtrType_Lean,
			PtrTypeFlags
			);
}

void
CValue::SetFunction (CFunction* pFunction)
{
	Clear ();

	m_ValueKind = EValue_Function;
	m_pFunction = pFunction;
	m_pType = pFunction->GetType ()->GetFunctionPtrType (
		EType_FunctionRef,
		EFunctionPtrType_Thin,
		EPtrTypeFlag_Safe
		);

	if (!pFunction->IsVirtual ())
		m_pLlvmValue = pFunction->GetLlvmFunction ();
}

void
CValue::SetFunctionTypeOverload (CFunctionTypeOverload* pFunctionTypeOverload)
{
	Clear ();

	m_ValueKind = pFunctionTypeOverload->IsOverloaded () ? EValue_FunctionTypeOverload : EValue_Type;
	m_pFunctionTypeOverload = pFunctionTypeOverload;
	m_pType = pFunctionTypeOverload->GetOverload (0);
}

void
CValue::SetProperty (CProperty* pProperty)
{
	Clear ();

	m_ValueKind = EValue_Property;
	m_pProperty = pProperty;
	m_pType = pProperty->GetType ()->GetPropertyPtrType (
		pProperty->GetParentNamespace (),
		EType_PropertyRef,
		EPropertyPtrType_Thin,
		EPtrTypeFlag_Safe
		);

	// don't assign LlvmValue yet cause property LlvmValue is only needed for pointers
}

void
CValue::SetField (
	CStructField* pField,
	CType* pType,
	size_t BaseOffset
	)
{
	Clear ();

	m_ValueKind = EValue_Field;
	m_pField = pField;
	m_pType = pType;
	m_Const.GetBuffer (sizeof (TBufHdr) + sizeof (size_t));
	*(size_t*) GetConstData () = BaseOffset + pField->GetOffset ();
}

void
CValue::SetField (
	CStructField* pField,
	size_t BaseOffset
	)
{
	SetField (pField, pField->GetType (), BaseOffset);
}

void
CValue::SetLlvmValue (
	llvm::Value* pLlvmValue,
	CType* pType,
	EValue ValueKind
	)
{
	Clear ();

	m_ValueKind = ValueKind;
	m_pType = pType;
	m_pLlvmValue = pLlvmValue;
}

void
CValue::SetLlvmValue (
	llvm::Value* pLlvmValue,
	EType TypeKind,
	EValue ValueKind
	)
{
	CModule* pModule = GetCurrentThreadModule ();
	ASSERT (pModule);

	CType* pType = pModule->m_TypeMgr.GetPrimitiveType (TypeKind);
	SetLlvmValue (pLlvmValue, pType, ValueKind);
}

void
CValue::SetLeanDataPtrValidator (CLeanDataPtrValidator* pValidator)
{
	ASSERT (m_pType->GetTypeKindFlags () & ETypeKindFlag_DataPtr);
	ASSERT (((CDataPtrType*) m_pType)->GetPtrTypeKind () == EDataPtrType_Lean);

	m_LeanDataPtrValidator = pValidator;
}

void
CValue::SetLeanDataPtrValidator (const CValue& ValidatorValue)
{
	ref::CPtrT <CLeanDataPtrValidator> Validator = AXL_REF_NEW (CLeanDataPtrValidator);
	Validator->m_ValidatorKind = ELeanDataPtrValidator_Simple;
	Validator->m_ScopeValidatorValue = ValidatorValue;
	SetLeanDataPtrValidator (Validator);
}

void
CValue::SetLeanDataPtrValidator (
	const CValue& ScopeValidatorValue,
	const CValue& RangeBeginValue,
	const CValue& SizeValue
	)
{
	ref::CPtrT <CLeanDataPtrValidator> Validator = AXL_REF_NEW (CLeanDataPtrValidator);
	Validator->m_ValidatorKind = ELeanDataPtrValidator_Complex;
	Validator->m_ScopeValidatorValue = ScopeValidatorValue;
	Validator->m_RangeBeginValue = RangeBeginValue;
	Validator->m_SizeValue = SizeValue;

	SetLeanDataPtrValidator (Validator);
}

bool
CValue::CreateConst (
	const void* p,
	CType* pType
	)
{
	Clear ();

	size_t Size = pType->GetSize ();

	bool Result = m_Const.GetBuffer (sizeof (TBufHdr) + Size) != NULL;
	if (!Result)
		return false;

	m_ValueKind = EValue_Const;
	m_pType = pType;
	m_Const->m_Size = Size;

	if (p)
		memcpy (GetConstData (), p, Size);
	else
		memset (GetConstData (), 0, Size);

	return true;
}

bool
CValue::CreateConst (
	const void* p,
	EType TypeKind
	)
{
	CModule* pModule = GetCurrentThreadModule ();
	ASSERT (pModule);

	CType* pType = pModule->m_TypeMgr.GetPrimitiveType (TypeKind);
	return CreateConst (p, pType);
}

void
CValue::SetCharArray (
	const void* p,
	size_t Size
	)
{
	CModule* pModule = GetCurrentThreadModule ();
	ASSERT (pModule);

	if (!Size)
		Size = 1;

	CType* pType = pModule->m_TypeMgr.GetArrayType (EType_Char, Size);
	CreateConst (p, pType);
}

//.............................................................................

} // namespace jnc {
