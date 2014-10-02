#include "pch.h"
#include "jnc_Type.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

uint_t
getTypeKindFlags (TypeKind typeKind)
{
	static uint_t flagTable [TypeKind__Count] =
	{
		0,                           // EType_Void
		0,                           // EType_Variant

		TypeKindFlagKind_Numeric,       // EType_Bool

		TypeKindFlagKind_Integer |      // EType_Int8
		TypeKindFlagKind_Numeric,

		TypeKindFlagKind_Unsigned |     // EType_Int8_u
		TypeKindFlagKind_Integer |
		TypeKindFlagKind_Numeric,

		TypeKindFlagKind_Integer |      // EType_Int16
		TypeKindFlagKind_Numeric,

		TypeKindFlagKind_Unsigned |     // EType_Int16_u
		TypeKindFlagKind_Integer |
		TypeKindFlagKind_Numeric,

		TypeKindFlagKind_Integer |      // EType_Int32
		TypeKindFlagKind_Numeric,

		TypeKindFlagKind_Unsigned |     // EType_Int32_u
		TypeKindFlagKind_Integer |
		TypeKindFlagKind_Numeric,

		TypeKindFlagKind_Integer |      // EType_Int64
		TypeKindFlagKind_Numeric,

		TypeKindFlagKind_Unsigned |     // EType_Int64_u
		TypeKindFlagKind_Integer |
		TypeKindFlagKind_Numeric,

		TypeKindFlagKind_BigEndian |    // EType_Int16_be
		TypeKindFlagKind_Integer |
		TypeKindFlagKind_Numeric,

		TypeKindFlagKind_Unsigned |     // EType_Int16_beu
		TypeKindFlagKind_BigEndian |
		TypeKindFlagKind_Integer |
		TypeKindFlagKind_Numeric,

		TypeKindFlagKind_BigEndian |    // EType_Int32_be
		TypeKindFlagKind_Integer |
		TypeKindFlagKind_Numeric,

		TypeKindFlagKind_Unsigned |     // EType_Int32_beu
		TypeKindFlagKind_BigEndian |
		TypeKindFlagKind_Integer |
		TypeKindFlagKind_Numeric,

		TypeKindFlagKind_BigEndian |    // EType_Int64_be
		TypeKindFlagKind_Integer |
		TypeKindFlagKind_Numeric,

		TypeKindFlagKind_Unsigned |     // EType_Int64_beu
		TypeKindFlagKind_BigEndian |
		TypeKindFlagKind_Integer |
		TypeKindFlagKind_Numeric,

		TypeKindFlagKind_Fp |           // EType_Float
		TypeKindFlagKind_Numeric,

		TypeKindFlagKind_Fp |           // EType_Double
		TypeKindFlagKind_Numeric,

		TypeKindFlagKind_Aggregate,     // EType_Array
		0,                           // EType_BitField

		TypeKindFlagKind_Named |        // EType_Enum
		TypeKindFlagKind_Integer |
		TypeKindFlagKind_Numeric,

		TypeKindFlagKind_Aggregate |    // EType_Struct
		TypeKindFlagKind_Derivable |
		TypeKindFlagKind_Named,

		TypeKindFlagKind_Aggregate |    // EType_Union
		TypeKindFlagKind_Derivable |
		TypeKindFlagKind_Named,

		TypeKindFlagKind_Aggregate |    // EType_Class
		TypeKindFlagKind_Derivable |
		TypeKindFlagKind_Named,

		TypeKindFlagKind_Code,          // EType_Function
		TypeKindFlagKind_Code,          // EType_Property

		TypeKindFlagKind_DataPtr |      // EType_DataPtr
		TypeKindFlagKind_Ptr,

		TypeKindFlagKind_DataPtr |      // EType_DataRef
		TypeKindFlagKind_Ptr |
		TypeKindFlagKind_Ref,

		TypeKindFlagKind_ClassPtr |     // EType_ClassPtr
		TypeKindFlagKind_Ptr,

		TypeKindFlagKind_ClassPtr |     // EType_ClassRef
		TypeKindFlagKind_Ptr |
		TypeKindFlagKind_Ref,

		TypeKindFlagKind_FunctionPtr |  // EType_FunctionPtr
		TypeKindFlagKind_Ptr,

		TypeKindFlagKind_FunctionPtr |  // EType_FunctionRef
		TypeKindFlagKind_Ptr |
		TypeKindFlagKind_Ref,

		TypeKindFlagKind_PropertyPtr |  // EType_PropertyPtr
		TypeKindFlagKind_Ptr,

		TypeKindFlagKind_PropertyPtr |  // EType_PropertyRef
		TypeKindFlagKind_Ptr |
		TypeKindFlagKind_Ref,

		TypeKindFlagKind_Import,        // EType_NamedImport

		TypeKindFlagKind_Import |       // EType_ImportPtr
		TypeKindFlagKind_Ptr,

		TypeKindFlagKind_Import |       // EType_ImportIntMod
		TypeKindFlagKind_Integer |
		TypeKindFlagKind_Numeric,
	};

	return typeKind < TypeKind__Count ? flagTable [typeKind] : 0;
}

//.............................................................................

TypeKind
getInt32TypeKind (int32_t integer)
{
	return
		integer >= INT8_MIN && integer <= INT8_MAX ? TypeKind_Int8 :
		(uint32_t) integer <= UINT8_MAX ? TypeKind_Int8_u :
		integer >= INT16_MIN && integer <= INT16_MAX ? TypeKind_Int16 :
		(uint32_t) integer <= UINT16_MAX ? TypeKind_Int16_u :
		integer >= INT32_MIN && integer <= INT32_MAX ? TypeKind_Int32 : TypeKind_Int32_u;
}

TypeKind
getInt32TypeKind_u (uint32_t integer)
{
	return
		integer <= UINT8_MAX ? TypeKind_Int8_u :
		integer <= UINT16_MAX ? TypeKind_Int16_u : TypeKind_Int32_u;
}

TypeKind
getInt64TypeKind (int64_t integer)
{
	return
		integer >= INT8_MIN && integer <= INT8_MAX ? TypeKind_Int8 :
		(uint64_t) integer <= UINT8_MAX ? TypeKind_Int8_u :
		integer >= INT16_MIN && integer <= INT16_MAX ? TypeKind_Int16 :
		(uint64_t) integer <= UINT16_MAX ? TypeKind_Int16_u :
		integer >= INT32_MIN && integer <= INT32_MAX ? TypeKind_Int32 :
		(uint64_t) integer <= UINT32_MAX ? TypeKind_Int32_u :
		integer >= INT64_MIN && integer <= INT64_MAX ? TypeKind_Int64 : TypeKind_Int64_u;
}

TypeKind
getInt64TypeKind_u (uint64_t integer)
{
	return
		integer <= UINT8_MAX ? TypeKind_Int8_u :
		integer <= UINT16_MAX ? TypeKind_Int16_u :
		integer <= UINT32_MAX ? TypeKind_Int32_u : TypeKind_Int64_u;
}

//.............................................................................

rtl::String
getLlvmTypeString (llvm::Type* llvmType)
{
	std::string s;
	llvm::raw_string_ostream stream (s);
	llvmType->print (stream);
	return stream.str ().c_str ();
}

//.............................................................................

const char*
getTypeModifierString (TypeModifierKind modifier)
{
	static const char* stringTable [] =
	{
		"unsigned",     // ETypeModifier_Unsigned    = 0x00000001,
		"bigendian",    // ETypeModifier_BigEndian   = 0x00000002,
		"const",        // ETypeModifier_Const       = 0x00000004,
		"dconst",       // ETypeModifier_DConst      = 0x00000008,
		"volatile",     // ETypeModifier_Volatile    = 0x00000010,
		"weak",         // ETypeModifier_Weak        = 0x00000020,
		"thin",         // ETypeModifier_Thin        = 0x00000040,
		"safe",         // ETypeModifier_Safe        = 0x00000080,
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
		"thiscall",     // ETypeModifier_Thiscall    = 0x00100000,
		"jnccall",      // ETypeModifier_Jnccall     = 0x00200000,
	};

	size_t i = rtl::getLoBitIdx32 (modifier);
	return i < countof (stringTable) ?
		stringTable [i] :
		"undefined-type-modifier";
}

rtl::String
getTypeModifierString (uint_t modifiers)
{
	if (!modifiers)
		return rtl::String ();

	TypeModifierKind modifier = getFirstTypeModifier (modifiers);
	rtl::String string = getTypeModifierString (modifier);
	modifiers &= ~modifier;

	while (modifiers)
	{
		modifier = getFirstTypeModifier (modifiers);

		string += ' ';
		string += getTypeModifierString (modifier);

		modifiers &= ~modifier;
	}

	return string;
}

//.............................................................................

const char*
getPtrTypeFlagString (PtrTypeFlagKind flag)
{
	static const char* stringTable [] =
	{
		"safe",     // EPtrTypeFlag_Safe      = 0x0010000
		"unused",   // EPtrTypeFlag_Unused    = 0x0020000
		"const",    // EPtrTypeFlag_Const     = 0x0040000
		"dconst",   // EPtrTypeFlag_ConstD    = 0x0080000
		"volatile", // EPtrTypeFlag_Volatile  = 0x0100000
		"event",    // EPtrTypeFlag_Event     = 0x0200000
		"devent",   // EPtrTypeFlag_EventD    = 0x0400000
		"bindable", // EPtrTypeFlag_Bindable  = 0x0800000
		"autoget",  // EPtrTypeFlag_AutoGet   = 0x1000000
	};

	size_t i = rtl::getLoBitIdx32 (flag >> 12);

	return i < countof (stringTable) ?
		stringTable [i] :
		"undefined-ptr-type-flag";
}

rtl::String
getPtrTypeFlagString (uint_t flags)
{
	rtl::String string;

	if (flags & PtrTypeFlagKind_Safe)
		string = "safe ";

	if (flags & PtrTypeFlagKind_Const)
		string += "const ";
	else if (flags & PtrTypeFlagKind_ConstD)
		string += "dconst ";

	if (flags & PtrTypeFlagKind_Volatile)
		string += "volatile ";

	if (flags & PtrTypeFlagKind_Event)
		string += "event ";
	else if (flags & PtrTypeFlagKind_EventD)
		string += "devent ";

	if (flags & PtrTypeFlagKind_Bindable)
		string += "bindable ";

	if (flags & PtrTypeFlagKind_AutoGet)
		string += "autoget ";

	if (!string.isEmpty ())
		string.reduceLength (1);

	return string;
}

rtl::String
getPtrTypeFlagSignature (uint_t flags)
{
	rtl::String signature;

	if (flags & PtrTypeFlagKind_Safe)
		signature = 's';

	if (flags & PtrTypeFlagKind_Const)
		signature += 'c';
	else if (flags & PtrTypeFlagKind_ConstD)
		signature += "pc";

	if (flags & PtrTypeFlagKind_Volatile)
		signature += 'v';

	if (flags & PtrTypeFlagKind_Event)
		signature += 'e';
	else if (flags & PtrTypeFlagKind_EventD)
		signature += "pe";

	return signature;
}

uint_t
getPtrTypeFlagsFromModifiers (uint_t modifiers)
{
	uint_t flags = 0;

	if (modifiers & TypeModifierKind_Safe)
		flags |= PtrTypeFlagKind_Safe;

	if (modifiers & TypeModifierKind_Volatile)
		flags |= PtrTypeFlagKind_Volatile;

	if (modifiers & TypeModifierKind_Const)
		flags |= PtrTypeFlagKind_Const;
	else if (modifiers & TypeModifierKind_DConst)
		flags |= PtrTypeFlagKind_ConstD;

	if (modifiers & TypeModifierKind_Event)
		flags |= PtrTypeFlagKind_Event;
	else if (modifiers & TypeModifierKind_DEvent)
		flags |= PtrTypeFlagKind_EventD;

	return flags;
}

//.............................................................................

Type::Type ()
{
	m_itemKind = ModuleItemKind_Type;
	m_typeKind = TypeKind_Void;
	m_size = 0;
	m_alignFactor = 0;
	m_llvmType = NULL;
	m_simplePropertyTypeTuple = NULL;
	m_functionArgTuple = NULL;
	m_dataPtrTypeTuple = NULL;
	m_boxClassType = NULL;
}

rtl::String
Type::getTypeString ()
{
	if (!m_typeString.isEmpty ())
		return m_typeString;

	prepareTypeString ();

	ASSERT (!m_typeString.isEmpty ());
	return m_typeString;
}

llvm::Type*
Type::getLlvmType ()
{
	if (m_llvmType)
		return m_llvmType;

	prepareLlvmType ();

	ASSERT (m_llvmType);
	return m_llvmType;
}

llvm::DIType
Type::getLlvmDiType ()
{
	if (m_llvmDiType)
		return m_llvmDiType;

	if (m_typeKind == TypeKind_Void)
		return llvm::DIType ();

	prepareLlvmDiType ();

	ASSERT (m_llvmDiType);
	return m_llvmDiType;
}

Value
Type::getUndefValue ()
{
	llvm::Value* llvmValue = llvm::UndefValue::get (getLlvmType ());
	return Value (llvmValue, this);
}

Value
Type::getZeroValue ()
{
	llvm::Value* llvmValue = llvm::Constant::getNullValue (getLlvmType ());
	return Value (llvmValue, this);
}

ArrayType*
Type::getArrayType (size_t elementCount)
{
	return m_module->m_typeMgr.getArrayType (this, elementCount);
}

DataPtrType*
Type::getDataPtrType (
	Namespace* anchorNamespace,
	TypeKind typeKind,
	DataPtrTypeKind ptrTypeKind,
	uint_t flags
	)
{
	return m_module->m_typeMgr.getDataPtrType (anchorNamespace, this, typeKind, ptrTypeKind, flags);
}

FunctionArg*
Type::getSimpleFunctionArg (uint_t ptrTypeFlags)
{
	return m_module->m_typeMgr.getSimpleFunctionArg (this, ptrTypeFlags);
}

ClassType*
Type::getBoxClassType ()
{
	return m_module->m_typeMgr.getBoxClassType (this);
}

void
Type::prepareTypeString ()
{
	static const char* stringTable [TypeKind__PrimitiveTypeCount] =
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

	ASSERT (m_typeKind < TypeKind__PrimitiveTypeCount);
	m_typeString = stringTable [m_typeKind];
}

void
Type::prepareLlvmType ()
{
	ASSERT (m_typeKind < TypeKind__PrimitiveTypeCount);

	switch (m_typeKind)
	{
	case TypeKind_Void:
		m_llvmType = llvm::Type::getVoidTy (*m_module->getLlvmContext ());
		break;

	case TypeKind_Variant:
		ASSERT (false); // variants are not supported yet
		break;

	case TypeKind_Bool:
		m_llvmType = llvm::Type::getInt1Ty (*m_module->getLlvmContext ());
		break;

	case TypeKind_Int8:
	case TypeKind_Int8_u:
		m_llvmType = llvm::Type::getInt8Ty (*m_module->getLlvmContext ());
		break;

	case TypeKind_Int16:
	case TypeKind_Int16_u:
	case TypeKind_Int16_be:
	case TypeKind_Int16_beu:
		m_llvmType = llvm::Type::getInt16Ty (*m_module->getLlvmContext ());
		break;

	case TypeKind_Int32:
	case TypeKind_Int32_u:
	case TypeKind_Int32_be:
	case TypeKind_Int32_beu:
		m_llvmType = llvm::Type::getInt32Ty (*m_module->getLlvmContext ());
		break;

	case TypeKind_Int64:
	case TypeKind_Int64_u:
	case TypeKind_Int64_be:
	case TypeKind_Int64_beu:
		m_llvmType = llvm::Type::getInt64Ty (*m_module->getLlvmContext ());
		break;

	case TypeKind_Float:
		m_llvmType = llvm::Type::getFloatTy (*m_module->getLlvmContext ());
		break;

	case TypeKind_Double:
		m_llvmType = llvm::Type::getDoubleTy (*m_module->getLlvmContext ());
		break;

	default:
		ASSERT (false);
	}
}

void
Type::prepareLlvmDiType ()
{
	struct LlvmDiType
	{
		const char* m_name;
		uint_t m_code;
		size_t m_size;
	};

	LlvmDiType llvmDiTypeTable [TypeKind__PrimitiveTypeCount] =
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

	ASSERT (m_typeKind < TypeKind__PrimitiveTypeCount);
	LlvmDiType* diType = &llvmDiTypeTable [m_typeKind];

	m_llvmDiType = m_module->m_llvmDiBuilder.createBasicType (
		diType->m_name,
		diType->m_size,
		diType->m_size,
		diType->m_code
		);
}

//.............................................................................

ModuleItem*
LazyStdType::getActualItem ()
{
	return m_module->m_typeMgr.getStdType (m_stdType);
}

//.............................................................................

Type*
getSimpleType (
	Module* module,
	TypeKind typeKind
	)
{
	return module->m_typeMgr.getPrimitiveType (typeKind);
}

Type*
getSimpleType (
	Module* module,
	StdTypeKind stdTypeKind
	)
{
	return module->m_typeMgr.getStdType (stdTypeKind);
}

Type*
getModuleItemType (ModuleItem* item)
{
	ModuleItemKind itemKind = item->getItemKind ();
	switch (itemKind)
	{
	case ModuleItemKind_Type:
		return (Type*) item;

	case ModuleItemKind_Typedef:
		return ((Typedef*) item)->getType ();

	case ModuleItemKind_Alias:
		return ((Alias*) item)->getType ();

	case ModuleItemKind_Variable:
		return ((Variable*) item)->getType ();

	case ModuleItemKind_FunctionArg:
		return ((FunctionArg*) item)->getType ();

	case ModuleItemKind_Function:
		return ((Function*) item)->getType ();

	case ModuleItemKind_Property:
		return ((Property*) item)->getType ();

	case ModuleItemKind_EnumConst:
		return ((EnumConst*) item)->getParentEnumType ();

	case ModuleItemKind_StructField:
		return ((StructField*) item)->getType ();

	default:
		return NULL;
	}
}

//.............................................................................

bool
isWeakPtrType (Type* type)
{
	TypeKind typeKind = type->getTypeKind ();
	switch (typeKind)
	{
	case TypeKind_ClassPtr:
		return ((ClassPtrType*) type)->getPtrTypeKind () == ClassPtrTypeKind_Weak;

	case TypeKind_FunctionPtr:
		return ((FunctionPtrType*) type)->getPtrTypeKind () == FunctionPtrTypeKind_Weak;

	case TypeKind_PropertyPtr:
		return ((PropertyPtrType*) type)->getPtrTypeKind () == PropertyPtrTypeKind_Weak;

	default:
		return false;
	}
}

Type*
getWeakPtrType (Type* type)
{
	TypeKind typeKind = type->getTypeKind ();
	switch (typeKind)
	{
	case TypeKind_ClassPtr:
		return ((ClassPtrType*) type)->getWeakPtrType ();

	case TypeKind_FunctionPtr:
		return ((FunctionPtrType*) type)->getWeakPtrType ();

	case TypeKind_PropertyPtr:
		return ((PropertyPtrType*) type)->getWeakPtrType ();

	default:
		return type;
	}
}

//.............................................................................

} // namespace jnc {
