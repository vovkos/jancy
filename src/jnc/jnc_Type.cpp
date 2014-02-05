#include "pch.h"
#include "jnc_Type.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

uint_t
GetTypeKindFlags (EType TypeKind)
{
	static uint_t FlagTable [EType__Count] =
	{
		0,                           // EType_Void
		0,                           // EType_Variant

		ETypeKindFlag_Numeric,       // EType_Bool

		ETypeKindFlag_Integer |      // EType_Int8
		ETypeKindFlag_Numeric,

		ETypeKindFlag_Unsigned |     // EType_Int8_u
		ETypeKindFlag_Integer |
		ETypeKindFlag_Numeric,

		ETypeKindFlag_Integer |      // EType_Int16
		ETypeKindFlag_Numeric,

		ETypeKindFlag_Unsigned |     // EType_Int16_u
		ETypeKindFlag_Integer |
		ETypeKindFlag_Numeric,

		ETypeKindFlag_Integer |      // EType_Int32
		ETypeKindFlag_Numeric,

		ETypeKindFlag_Unsigned |     // EType_Int32_u
		ETypeKindFlag_Integer |
		ETypeKindFlag_Numeric,
				
		ETypeKindFlag_Integer |      // EType_Int64
		ETypeKindFlag_Numeric,

		ETypeKindFlag_Unsigned |     // EType_Int64_u
		ETypeKindFlag_Integer |
		ETypeKindFlag_Numeric,

		ETypeKindFlag_BigEndian |    // EType_Int16_be
		ETypeKindFlag_Integer |
		ETypeKindFlag_Numeric,

		ETypeKindFlag_Unsigned |     // EType_Int16_beu
		ETypeKindFlag_BigEndian |
		ETypeKindFlag_Integer |
		ETypeKindFlag_Numeric,

		ETypeKindFlag_BigEndian |    // EType_Int32_be
		ETypeKindFlag_Integer |
		ETypeKindFlag_Numeric,

		ETypeKindFlag_Unsigned |     // EType_Int32_beu
		ETypeKindFlag_BigEndian |
		ETypeKindFlag_Integer |
		ETypeKindFlag_Numeric,

		ETypeKindFlag_BigEndian |    // EType_Int64_be
		ETypeKindFlag_Integer |
		ETypeKindFlag_Numeric,

		ETypeKindFlag_Unsigned |     // EType_Int64_beu
		ETypeKindFlag_BigEndian |
		ETypeKindFlag_Integer |
		ETypeKindFlag_Numeric,

		ETypeKindFlag_Fp |           // EType_Float
		ETypeKindFlag_Numeric,

		ETypeKindFlag_Fp |           // EType_Double
		ETypeKindFlag_Numeric,

		ETypeKindFlag_Aggregate,     // EType_Array
		0,                           // EType_BitField

		ETypeKindFlag_Named |        // EType_Enum
		ETypeKindFlag_Integer |
		ETypeKindFlag_Numeric,

		ETypeKindFlag_Aggregate |    // EType_Struct
		ETypeKindFlag_Derivable |
		ETypeKindFlag_Named,

		ETypeKindFlag_Aggregate |    // EType_Union
		ETypeKindFlag_Derivable |
		ETypeKindFlag_Named,

		ETypeKindFlag_Aggregate |    // EType_Class
		ETypeKindFlag_Derivable |
		ETypeKindFlag_Named,

		ETypeKindFlag_Code,          // EType_Function
		ETypeKindFlag_Code,          // EType_Property

		ETypeKindFlag_DataPtr |      // EType_DataPtr
		ETypeKindFlag_Ptr,

		ETypeKindFlag_DataPtr |      // EType_DataRef
		ETypeKindFlag_Ptr |
		ETypeKindFlag_Ref,

		ETypeKindFlag_ClassPtr |     // EType_ClassPtr
		ETypeKindFlag_Ptr,

		ETypeKindFlag_ClassPtr |     // EType_ClassRef
		ETypeKindFlag_Ptr |
		ETypeKindFlag_Ref,

		ETypeKindFlag_FunctionPtr |  // EType_FunctionPtr
		ETypeKindFlag_Ptr,

		ETypeKindFlag_FunctionPtr |  // EType_FunctionRef
		ETypeKindFlag_Ptr |
		ETypeKindFlag_Ref,

		ETypeKindFlag_PropertyPtr |  // EType_PropertyPtr
		ETypeKindFlag_Ptr,

		ETypeKindFlag_PropertyPtr |  // EType_PropertyRef
		ETypeKindFlag_Ptr |
		ETypeKindFlag_Ref,

		ETypeKindFlag_Import,        // EType_NamedImport

		ETypeKindFlag_Import |       // EType_ImportPtr
		ETypeKindFlag_Ptr,

		ETypeKindFlag_Import |       // EType_ImportIntMod
		ETypeKindFlag_Integer | 
		ETypeKindFlag_Numeric,
	};

	return TypeKind < EType__Count ? FlagTable [TypeKind] : 0;
}

//.............................................................................

EType
GetInt32TypeKind (int32_t Integer)
{
	return
		Integer >= INT8_MIN && Integer <= INT8_MAX ? EType_Int8 :
		(uint32_t) Integer <= UINT8_MAX ? EType_Int8_u :
		Integer >= INT16_MIN && Integer <= INT16_MAX ? EType_Int16 :
		(uint32_t) Integer <= UINT16_MAX ? EType_Int16_u :
		Integer >= INT32_MIN && Integer <= INT32_MAX ? EType_Int32 : EType_Int32_u;
}

EType
GetInt32TypeKind_u (uint32_t Integer)
{
	return
		Integer <= UINT8_MAX ? EType_Int8_u :
		Integer <= UINT16_MAX ? EType_Int16_u : EType_Int32_u;
}

EType
GetInt64TypeKind (int64_t Integer)
{
	return
		Integer >= INT8_MIN && Integer <= INT8_MAX ? EType_Int8 :
		(uint64_t) Integer <= UINT8_MAX ? EType_Int8_u :
		Integer >= INT16_MIN && Integer <= INT16_MAX ? EType_Int16 :
		(uint64_t) Integer <= UINT16_MAX ? EType_Int16_u :
		Integer >= INT32_MIN && Integer <= INT32_MAX ? EType_Int32 :
		(uint64_t) Integer <= UINT32_MAX ? EType_Int32_u :
		Integer >= INT64_MIN && Integer <= INT64_MAX ? EType_Int64 : EType_Int64_u;
}

EType
GetInt64TypeKind_u (uint64_t Integer)
{
	return
		Integer <= UINT8_MAX ? EType_Int8_u :
		Integer <= UINT16_MAX ? EType_Int16_u :
		Integer <= UINT32_MAX ? EType_Int32_u : EType_Int64_u;
}

//.............................................................................

rtl::CString
GetLlvmTypeString (llvm::Type* pLlvmType)
{
	std::string s;
	llvm::raw_string_ostream Stream (s);
	pLlvmType->print (Stream);
	return Stream.str ().c_str ();
}

//.............................................................................

const char*
GetTypeModifierString (ETypeModifier Modifier)
{
	static const char* StringTable [] =
	{
		"unsigned",     // ETypeModifier_Unsigned    = 0x00000001,
		"bigendian",    // ETypeModifier_BigEndian   = 0x00000002,
		"const",        // ETypeModifier_Const       = 0x00000004,
		"dconst",       // ETypeModifier_DConst      = 0x00000008,
		"volatile",     // ETypeModifier_Volatile    = 0x00000010,
		"weak",         // ETypeModifier_Weak        = 0x00000020,
		"thin",         // ETypeModifier_Thin        = 0x00000040,
		"unused-0",     // ETypeModifier_Unused      = 0x00000080,
		"cdecl",        // ETypeModifier_Cdecl       = 0x00000100,
		"stdcall",      // ETypeModifier_Stdcall     = 0x00000200,
		"array",        // ETypeModifier_Array       = 0x00000400,
		"function",     // ETypeModifier_Function    = 0x00000800,
		"property",     // ETypeModifier_Property    = 0x00001000,
		"bindable",     // ETypeModifier_Bindable    = 0x00002000,
		"autoget",      // ETypeModifier_AutoGet     = 0x00004000,
		"indexed",      // ETypeModifier_Indexed     = 0x00008000,
		"multicast",    // ETypeModifier_Multicast   = 0x00010000,
		"event",        // ETypeModifier_Event       = 0x00020000,
		"devent",       // ETypeModifier_DEvent      = 0x00040000,
		"reactor",      // ETypeModifier_Reactor     = 0x00080000,
	};

	size_t i = rtl::GetLoBitIdx32 (Modifier);
	return i < countof (StringTable) ?
		StringTable [i] :
		"undefined-type-modifier";
}

rtl::CString
GetTypeModifierString (uint_t Modifiers)
{
	if (!Modifiers)
		return rtl::CString ();

	ETypeModifier Modifier = GetFirstTypeModifier (Modifiers);
	rtl::CString String = GetTypeModifierString (Modifier);
	Modifiers &= ~Modifier;

	while (Modifiers)
	{
		Modifier = GetFirstTypeModifier (Modifiers);

		String += ' ';
		String += GetTypeModifierString (Modifier);

		Modifiers &= ~Modifier;
	}

	return String;
}

//.............................................................................

const char*
GetPtrTypeFlagString (EPtrTypeFlag Flag)
{
	static const char* StringTable [] =
	{
		"checked",  // EPtrTypeFlag_Checked   = 0x0010000
		"markup",   // EPtrTypeFlag_Markup    = 0x0020000
		"const",    // EPtrTypeFlag_Const     = 0x0040000
		"dconst",   // EPtrTypeFlag_ConstD    = 0x0080000
		"volatile", // EPtrTypeFlag_Volatile  = 0x0100000
		"event",    // EPtrTypeFlag_Event     = 0x0200000
		"devent",   // EPtrTypeFlag_EventD    = 0x0400000
		"bindable", // EPtrTypeFlag_Bindable  = 0x0800000
		"autoget",  // EPtrTypeFlag_AutoGet   = 0x1000000
	};

	size_t i = rtl::GetLoBitIdx32 (Flag >> 12);

	return i < countof (StringTable) ?
		StringTable [i] :
		"undefined-ptr-type-flag";
}

rtl::CString
GetPtrTypeFlagString (uint_t Flags)
{
	rtl::CString String;

	if (Flags & EPtrTypeFlag_Checked)
		String = "checked ";

	if (Flags & EPtrTypeFlag_Markup)
		String = "markup ";

	if (Flags & EPtrTypeFlag_Const)
		String += "const ";
	else if (Flags & EPtrTypeFlag_ConstD)
		String += "dconst ";

	if (Flags & EPtrTypeFlag_Volatile)
		String += "volatile ";

	if (Flags & EPtrTypeFlag_Event)
		String += "event ";
	else if (Flags & EPtrTypeFlag_EventD)
		String += "devent ";

	if (Flags & EPtrTypeFlag_Bindable)
		String += "bindable ";

	if (Flags & EPtrTypeFlag_AutoGet)
		String += "autoget ";

	if (!String.IsEmpty ())
		String.ReduceLength (1);

	return String;
}

rtl::CString
GetPtrTypeFlagSignature (uint_t Flags)
{
	rtl::CString Signature;

	if (Flags & EPtrTypeFlag_Checked)
		Signature = 's';

	if (Flags & EPtrTypeFlag_Markup)
		Signature = 'm';

	if (Flags & EPtrTypeFlag_Const)
		Signature += 'c';
	else if (Flags & EPtrTypeFlag_ConstD)
		Signature += "pc";

	if (Flags & EPtrTypeFlag_Volatile)
		Signature += 'v';

	if (Flags & EPtrTypeFlag_Event)
		Signature += 'e';
	else if (Flags & EPtrTypeFlag_EventD)
		Signature += "pe";

	return Signature;
}

uint_t
GetPtrTypeFlagsFromModifiers (uint_t Modifiers)
{
	uint_t Flags = 0;

	if (Modifiers & ETypeModifier_Volatile)
		Flags |= EPtrTypeFlag_Volatile;

	if (Modifiers & ETypeModifier_Const)
		Flags |= EPtrTypeFlag_Const;
	else if (Modifiers & ETypeModifier_DConst)
		Flags |= EPtrTypeFlag_ConstD;

	if (Modifiers & ETypeModifier_Event)
		Flags |= EPtrTypeFlag_Event;
	else if (Modifiers & ETypeModifier_DEvent)
		Flags |= EPtrTypeFlag_EventD;

	return Flags;
}

//.............................................................................

CType::CType ()
{
	m_ItemKind = EModuleItem_Type;
	m_TypeKind = EType_Void;
	m_Size = 0;
	m_AlignFactor = 0;
	m_pLlvmType = NULL;
	m_pSimplePropertyTypeTuple = NULL;
	m_pFunctionArgTuple = NULL;
	m_pDataPtrTypeTuple = NULL;
	m_pBoxClassType = NULL;
}

rtl::CString
CType::GetTypeString ()
{
	if (!m_TypeString.IsEmpty ())
		return m_TypeString;

	PrepareTypeString ();

	ASSERT (!m_TypeString.IsEmpty ());
	return m_TypeString;
}

llvm::Type*
CType::GetLlvmType ()
{
	if (m_pLlvmType)
		return m_pLlvmType;

	PrepareLlvmType ();

	ASSERT (m_pLlvmType);
	return m_pLlvmType;
}

llvm::DIType
CType::GetLlvmDiType ()
{
	if (m_LlvmDiType)
		return m_LlvmDiType;

	if (m_TypeKind == EType_Void)
		return llvm::DIType ();

	PrepareLlvmDiType ();

	ASSERT (m_LlvmDiType);
	return m_LlvmDiType;
}

CValue
CType::GetUndefValue ()
{
	llvm::Value* pLlvmValue = llvm::UndefValue::get (GetLlvmType ());
	return CValue (pLlvmValue, this);
}

CValue
CType::GetZeroValue ()
{
	llvm::Value* pLlvmValue = llvm::Constant::getNullValue (GetLlvmType ());
	return CValue (pLlvmValue, this);
}

CArrayType*
CType::GetArrayType (size_t ElementCount)
{
	return m_pModule->m_TypeMgr.GetArrayType (this, ElementCount);
}

CDataPtrType*
CType::GetDataPtrType (
	CNamespace* pAnchorNamespace,
	EType TypeKind,
	EDataPtrType PtrTypeKind,
	uint_t Flags
	)
{
	return m_pModule->m_TypeMgr.GetDataPtrType (pAnchorNamespace, this, TypeKind, PtrTypeKind, Flags);
}

CFunctionArg*
CType::GetSimpleFunctionArg (uint_t PtrTypeFlags)
{
	return m_pModule->m_TypeMgr.GetSimpleFunctionArg (this, PtrTypeFlags);
}

CClassType*
CType::GetBoxClassType ()
{
	return m_pModule->m_TypeMgr.GetBoxClassType (this);
}

void
CType::PrepareTypeString ()
{
	static const char* StringTable [EType__PrimitiveTypeCount] =
	{
		"void",
		"variant",
		"bool",
		"int8",
		"unsigned int8",
		"int16",
		"unsigned int16",
		"int32",
		"unsigned int32",
		"int64",
		"unsigned int64",
		"bigendian int16",
		"unsigned bigendian int16",
		"bigendian int32",
		"unsigned bigendian int32",
		"bigendian int64",
		"unsigned bigendian int64",
		"float",
		"double",
	};

	ASSERT (m_TypeKind < EType__PrimitiveTypeCount);
	m_TypeString = StringTable [m_TypeKind];
}

void
CType::PrepareLlvmType ()
{
	ASSERT (m_TypeKind < EType__PrimitiveTypeCount);

	switch (m_TypeKind)
	{
	case EType_Void:
		m_pLlvmType = llvm::Type::getVoidTy (*m_pModule->GetLlvmContext ());
		break;

	case EType_Variant:
		ASSERT (false); // variants are not supported yet
		break;

	case EType_Bool:
		m_pLlvmType = llvm::Type::getInt1Ty (*m_pModule->GetLlvmContext ());
		break;

	case EType_Int8:
	case EType_Int8_u:
		m_pLlvmType = llvm::Type::getInt8Ty (*m_pModule->GetLlvmContext ());
		break;

	case EType_Int16:
	case EType_Int16_u:
	case EType_Int16_be:
	case EType_Int16_beu:
		m_pLlvmType = llvm::Type::getInt16Ty (*m_pModule->GetLlvmContext ());
		break;

	case EType_Int32:
	case EType_Int32_u:
	case EType_Int32_be:
	case EType_Int32_beu:
		m_pLlvmType = llvm::Type::getInt32Ty (*m_pModule->GetLlvmContext ());
		break;

	case EType_Int64:
	case EType_Int64_u:
	case EType_Int64_be:
	case EType_Int64_beu:
		m_pLlvmType = llvm::Type::getInt64Ty (*m_pModule->GetLlvmContext ());
		break;

	case EType_Float:
		m_pLlvmType = llvm::Type::getFloatTy (*m_pModule->GetLlvmContext ());
		break;

	case EType_Double:
		m_pLlvmType = llvm::Type::getDoubleTy (*m_pModule->GetLlvmContext ());
		break;

	default:
		ASSERT (false);
	}
}

void
CType::PrepareLlvmDiType ()
{
	struct TLlvmDiType
	{
		const char* m_pName;
		uint_t m_Code;
		size_t m_Size;
	};

	TLlvmDiType LlvmDiTypeTable [EType__PrimitiveTypeCount] =
	{
		{ 0 }, // EType_Void,
		{ 0 }, // EType_Variant,

		// EType_Bool,
		{
			"bool",
			llvm::dwarf::DW_ATE_boolean,
			1,
		},

		// EType_Int8,
		{
			"int8",
			llvm::dwarf::DW_ATE_signed_char,
			1,
		},

		// EType_Int8_u,
		{
			"unsigned int8",
			llvm::dwarf::DW_ATE_unsigned_char,
			1,
		},

		// EType_Int16,
		{
			"int16",
			llvm::dwarf::DW_ATE_signed,
			2,
		},

		// EType_Int16_u,
		{
			"unsigned int16",
			llvm::dwarf::DW_ATE_unsigned,
			2,
		},

		// EType_Int32,
		{
			"int32",
			llvm::dwarf::DW_ATE_signed,
			4,
		},

		// EType_Int32_u,
		{
			"unsigned int32",
			llvm::dwarf::DW_ATE_unsigned,
			4,
		},

		// EType_Int64,
		{
			"unsigned int64",
			llvm::dwarf::DW_ATE_signed,
			8,
		},

		// EType_Int64_u,
		{
			"unsigned int64",
			llvm::dwarf::DW_ATE_unsigned,
			8,
		},

		// EType_Int16_be,
		{
			"bigendian int16",
			llvm::dwarf::DW_ATE_signed,
			2,
		},

		// EType_Int16_beu,
		{
			"unsigned bigendian int16",
			llvm::dwarf::DW_ATE_unsigned,
			2,
		},

		// EType_Int32_be,
		{
			"bigendian int16",
			llvm::dwarf::DW_ATE_signed,
			4,
		},

		// EType_Int32_beu,
		{
			"unsigned bigendian int16",
			llvm::dwarf::DW_ATE_unsigned,
			4,
		},

		// EType_Int64_be,
		{
			"bigendian int16",
			llvm::dwarf::DW_ATE_signed,
			8,
		},

		// EType_Int64_beu,
		{
			"unsigned bigendian int64",
			llvm::dwarf::DW_ATE_unsigned,
			8,
		},

		// EType_Float,
		{
			"float",
			llvm::dwarf::DW_ATE_float,
			4,
		},

		// EType_Double,
		{
			"float",
			llvm::dwarf::DW_ATE_float,
			8,
		},
	};

	ASSERT (m_TypeKind < EType__PrimitiveTypeCount);
	TLlvmDiType* pDiType = &LlvmDiTypeTable [m_TypeKind];

	m_LlvmDiType = m_pModule->m_LlvmDiBuilder.CreateBasicType (
		pDiType->m_pName,
		pDiType->m_Size,
		pDiType->m_Size,
		pDiType->m_Code
		);
}

//.............................................................................
	
CModuleItem*
CLazyStdType::GetActualItem ()
{
	return m_pModule->m_TypeMgr.GetStdType (m_StdType);
}

//.............................................................................

CType*
GetSimpleType (
	CModule* pModule,
	EType TypeKind
	)
{
	return pModule->m_TypeMgr.GetPrimitiveType (TypeKind);
}

CType*
GetSimpleType (
	CModule* pModule,
	EStdType StdTypeKind
	)
{
	return pModule->m_TypeMgr.GetStdType (StdTypeKind);
}

CType*
GetModuleItemType (CModuleItem* pItem)
{
	EModuleItem ItemKind = pItem->GetItemKind ();
	switch (ItemKind)
	{
	case EModuleItem_Type:
		return (CType*) pItem;

	case EModuleItem_Typedef:
		return ((CTypedef*) pItem)->GetType ();

	case EModuleItem_Alias:
		return ((CAlias*) pItem)->GetType ();

	case EModuleItem_Variable:
		return ((CVariable*) pItem)->GetType ();

	case EModuleItem_FunctionArg:
		return ((CFunctionArg*) pItem)->GetType ();

	case EModuleItem_Function:
		return ((CFunction*) pItem)->GetType ();

	case EModuleItem_Property:
		return ((CProperty*) pItem)->GetType ();

	case EModuleItem_EnumConst:
		return ((CEnumConst*) pItem)->GetParentEnumType ();

	case EModuleItem_StructField:
		return ((CStructField*) pItem)->GetType ();

	default:
		return NULL;
	}
}

//.............................................................................

bool
IsWeakPtrType (CType* pType)
{
	EType TypeKind = pType->GetTypeKind ();
	switch (TypeKind)
	{
	case EType_ClassPtr:
		return ((CClassPtrType*) pType)->GetPtrTypeKind () == EClassPtrType_Weak;

	case EType_FunctionPtr:
		return ((CFunctionPtrType*) pType)->GetPtrTypeKind () == EFunctionPtrType_Weak;

	case EType_PropertyPtr:
		return ((CPropertyPtrType*) pType)->GetPtrTypeKind () == EPropertyPtrType_Weak;

	default:
		return false;
	}
}

CType*
GetWeakPtrType (CType* pType)
{
	EType TypeKind = pType->GetTypeKind ();
	switch (TypeKind)
	{
	case EType_ClassPtr:
		return ((CClassPtrType*) pType)->GetWeakPtrType ();

	case EType_FunctionPtr:
		return ((CFunctionPtrType*) pType)->GetWeakPtrType ();

	case EType_PropertyPtr:
		return ((CPropertyPtrType*) pType)->GetWeakPtrType ();

	default:
		return pType;
	}
}

//.............................................................................

} // namespace jnc {
