#include "pch.h"
#include "jnc_ct_Type.h"
#include "jnc_rt_VariantUtils.h"
#include "jnc_ct_Decl.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

uint_t
getTypeKindFlags (TypeKind typeKind)
{
	static uint_t flagTable [TypeKind__Count] =
	{
		0,                          // EType_Void
		TypeKindFlag_Nullable,      // EType_Variant

		TypeKindFlag_Numeric,       // EType_Bool

		TypeKindFlag_Integer |      // EType_Int8
		TypeKindFlag_Numeric,

		TypeKindFlag_Unsigned |     // EType_Int8_u
		TypeKindFlag_Integer |
		TypeKindFlag_Numeric,

		TypeKindFlag_Integer |      // EType_Int16
		TypeKindFlag_Numeric,

		TypeKindFlag_Unsigned |     // EType_Int16_u
		TypeKindFlag_Integer |
		TypeKindFlag_Numeric,

		TypeKindFlag_Integer |      // EType_Int32
		TypeKindFlag_Numeric,

		TypeKindFlag_Unsigned |     // EType_Int32_u
		TypeKindFlag_Integer |
		TypeKindFlag_Numeric,

		TypeKindFlag_Integer |      // EType_Int64
		TypeKindFlag_Numeric,

		TypeKindFlag_Unsigned |     // EType_Int64_u
		TypeKindFlag_Integer |
		TypeKindFlag_Numeric,

		TypeKindFlag_BigEndian |    // EType_Int16_be
		TypeKindFlag_Integer |
		TypeKindFlag_Numeric,

		TypeKindFlag_Unsigned |     // EType_Int16_beu
		TypeKindFlag_BigEndian |
		TypeKindFlag_Integer |
		TypeKindFlag_Numeric,

		TypeKindFlag_BigEndian |    // EType_Int32_be
		TypeKindFlag_Integer |
		TypeKindFlag_Numeric,

		TypeKindFlag_Unsigned |     // EType_Int32_beu
		TypeKindFlag_BigEndian |
		TypeKindFlag_Integer |
		TypeKindFlag_Numeric,

		TypeKindFlag_BigEndian |    // EType_Int64_be
		TypeKindFlag_Integer |
		TypeKindFlag_Numeric,

		TypeKindFlag_Unsigned |     // EType_Int64_beu
		TypeKindFlag_BigEndian |
		TypeKindFlag_Integer |
		TypeKindFlag_Numeric,

		TypeKindFlag_Fp |           // EType_Float
		TypeKindFlag_Numeric,

		TypeKindFlag_Fp |           // EType_Double
		TypeKindFlag_Numeric,

		TypeKindFlag_Aggregate |
		TypeKindFlag_Nullable,      // EType_Array
		
		0,                          // EType_BitField

		TypeKindFlag_Named |        // EType_Enum
		TypeKindFlag_Integer |
		TypeKindFlag_Numeric,

		TypeKindFlag_Aggregate |    // EType_Struct
		TypeKindFlag_Derivable |
		TypeKindFlag_Named |
		TypeKindFlag_Nullable,

		TypeKindFlag_Aggregate |    // EType_Union
		TypeKindFlag_Derivable |
		TypeKindFlag_Named |
		TypeKindFlag_Nullable,

		TypeKindFlag_Aggregate |    // EType_Class
		TypeKindFlag_Derivable |
		TypeKindFlag_Named,

		TypeKindFlag_Code,          // EType_Function
		TypeKindFlag_Code,          // EType_Property

		TypeKindFlag_DataPtr |      // EType_DataPtr
		TypeKindFlag_Ptr |
		TypeKindFlag_Nullable,

		TypeKindFlag_DataPtr |      // EType_DataRef
		TypeKindFlag_Ptr |
		TypeKindFlag_Ref,

		TypeKindFlag_ClassPtr |     // EType_ClassPtr
		TypeKindFlag_Ptr |
		TypeKindFlag_Nullable,

		TypeKindFlag_ClassPtr |     // EType_ClassRef
		TypeKindFlag_Ptr |
		TypeKindFlag_Ref,

		TypeKindFlag_FunctionPtr |  // EType_FunctionPtr
		TypeKindFlag_Ptr |
		TypeKindFlag_Nullable,

		TypeKindFlag_FunctionPtr |  // EType_FunctionRef
		TypeKindFlag_Ptr |
		TypeKindFlag_Ref,

		TypeKindFlag_PropertyPtr |  // EType_PropertyPtr
		TypeKindFlag_Ptr |
		TypeKindFlag_Nullable,

		TypeKindFlag_PropertyPtr |  // EType_PropertyRef
		TypeKindFlag_Ptr |
		TypeKindFlag_Ref,

		TypeKindFlag_Import,        // EType_NamedImport

		TypeKindFlag_Import |       // EType_ImportPtr
		TypeKindFlag_Ptr |
		TypeKindFlag_Nullable,

		TypeKindFlag_Import |       // EType_ImportIntMod
		TypeKindFlag_Integer |
		TypeKindFlag_Numeric,
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
		(uint32_t) integer <= UINT16_MAX ? TypeKind_Int16_u : TypeKind_Int32;
}

TypeKind
getInt32TypeKind_u (uint32_t integer)
{
	return
		integer <= INT8_MAX ? TypeKind_Int8 :
		integer <= UINT8_MAX ? TypeKind_Int8_u :
		integer <= INT16_MAX ? TypeKind_Int16 :
		integer <= UINT16_MAX ? TypeKind_Int16_u : 
		integer <= INT32_MAX ? TypeKind_Int32 : TypeKind_Int32_u;
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
		(uint64_t) integer <= UINT32_MAX ? TypeKind_Int32_u : TypeKind_Int64;
}

TypeKind
getInt64TypeKind_u (uint64_t integer)
{
	return
		integer <= INT8_MAX ? TypeKind_Int8 :
		integer <= UINT8_MAX ? TypeKind_Int8_u :
		integer <= INT16_MAX ? TypeKind_Int16 :
		integer <= UINT16_MAX ? TypeKind_Int16_u :
		integer <= INT32_MAX ? TypeKind_Int32 :
		integer <= UINT32_MAX ? TypeKind_Int32_u :
		integer <= INT64_MAX ? TypeKind_Int64 : TypeKind_Int64_u;
}

//.............................................................................

sl::String
getLlvmTypeString (llvm::Type* llvmType)
{
	std::string s;
	llvm::raw_string_ostream stream (s);
	llvmType->print (stream);
	return stream.str ().c_str ();
}

//.............................................................................

const char*
getPtrTypeFlagString (PtrTypeFlag flag)
{
	static const char* stringTable [] =
	{
		"safe",      // PtrTypeFlag_Safe      = 0x0010000
		"unused",    // PtrTypeFlag_Unused    = 0x0020000
		"const",     // PtrTypeFlag_Const     = 0x0040000
		"readonly",  // PtrTypeFlag_ReadOnly  = 0x0080000
		"volatile",  // PtrTypeFlag_Volatile  = 0x0100000
		"event",     // PtrTypeFlag_Event     = 0x0200000
		"dualevent", // PtrTypeFlag_DualEvent = 0x0400000
		"bindable",  // PtrTypeFlag_Bindable  = 0x0800000
		"autoget",   // PtrTypeFlag_AutoGet   = 0x1000000
	};

	size_t i = sl::getLoBitIdx32 (flag >> 12);

	return i < countof (stringTable) ?
		stringTable [i] :
		"undefined-ptr-type-flag";
}

sl::String
getPtrTypeFlagString (uint_t flags)
{
	sl::String string;

	if (flags & PtrTypeFlag_Safe)
		string = "safe ";

	if (flags & PtrTypeFlag_Const)
		string += "const ";
	else if (flags & PtrTypeFlag_ReadOnly)
		string += "readonly ";

	if (flags & PtrTypeFlag_Volatile)
		string += "volatile ";

	if (flags & PtrTypeFlag_Event)
		string += "event ";
	else if (flags & PtrTypeFlag_DualEvent)
		string += "dualevent ";

	if (flags & PtrTypeFlag_Bindable)
		string += "bindable ";

	if (flags & PtrTypeFlag_AutoGet)
		string += "autoget ";

	if (!string.isEmpty ())
		string.reduceLength (1);

	return string;
}

sl::String
getPtrTypeFlagSignature (uint_t flags)
{
	sl::String signature;

	if (flags & PtrTypeFlag_Safe)
		signature += 's';

	if (flags & PtrTypeFlag_Const)
		signature += 'c';
	else if (flags & PtrTypeFlag_ReadOnly)
		signature += "pc";

	if (flags & PtrTypeFlag_Volatile)
		signature += 'v';

	if (flags & PtrTypeFlag_Event)
		signature += 'e';
	else if (flags & PtrTypeFlag_DualEvent)
		signature += "pe";

	return signature;
}

uint_t
getPtrTypeFlagsFromModifiers (uint_t modifiers)
{
	uint_t flags = 0;

	if (modifiers & TypeModifier_Safe)
		flags |= PtrTypeFlag_Safe;

	if (modifiers & TypeModifier_Volatile)
		flags |= PtrTypeFlag_Volatile;

	if (modifiers & TypeModifier_Const)
		flags |= PtrTypeFlag_Const;
	else if (modifiers & TypeModifier_ReadOnly)
		flags |= PtrTypeFlag_ReadOnly;

	if (modifiers & TypeModifier_Event)
		flags |= PtrTypeFlag_Event;

	return flags;
}

//.............................................................................

Type::Type ()
{
	m_itemKind = ModuleItemKind_Type;
	m_typeKind = TypeKind_Void;
	m_stdType = (StdType) -1;
	m_size = 0;
	m_alignment = 1;
	m_llvmType = NULL;
	m_simplePropertyTypeTuple = NULL;
	m_functionArgTuple = NULL;
	m_dataPtrTypeTuple = NULL;
	m_boxClassType = NULL;
}

sl::String
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
	#pragma AXL_TODO ("Type::getZeroValue () probably should return ValueKind_Const")

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
		m_llvmType = m_module->m_typeMgr.getStdType (StdType_VariantStruct)->getLlvmType ();
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
	ASSERT (m_typeKind < TypeKind__PrimitiveTypeCount);

	if (m_typeKind == TypeKind_Variant)
	{
		m_llvmDiType = m_module->m_typeMgr.getStdType (StdType_VariantStruct)->getLlvmDiType ();
		return;
	}

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

	LlvmDiType* diType = &llvmDiTypeTable [m_typeKind];
	ASSERT (diType->m_size);

	m_llvmDiType = m_module->m_llvmDiBuilder.createBasicType (
		diType->m_name,
		diType->m_size,
		diType->m_size,
		diType->m_code
		);
}

void
Type::markGcRoots (
	const void* p,
	rt::GcHeap* gcHeap
	)
{
	ASSERT (m_typeKind == TypeKind_Variant);

	rt::Variant* variant = (rt::Variant*) p;
	if (variant->m_type && (variant->m_type->m_flags & TypeFlag_GcRoot))
		variant->m_type->markGcRoots (p, gcHeap);
}

//.............................................................................

Type*
getSimpleType (
	TypeKind typeKind,
	Module* module
	)
{
	return module->m_typeMgr.getPrimitiveType (typeKind);
}

Type*
getSimpleType (
	StdType stdType,
	Module* module
	)
{
	return module->m_typeMgr.getStdType (stdType);
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

Type*
getDirectRefType (
	Namespace* anchorNamespace,
	Type* type,
	uint_t ptrTypeFlags
	)
{
	return type->getTypeKind () == TypeKind_Class ? 
		(Type*) ((ClassType*) type)->getClassPtrType (
			anchorNamespace,
			TypeKind_ClassRef,
			ClassPtrTypeKind_Normal,
			ptrTypeFlags
			) :
		(Type*) type->getDataPtrType (
			anchorNamespace,
			TypeKind_DataRef,
			DataPtrTypeKind_Lean,
			ptrTypeFlags
			);
}

//.............................................................................

bool 
isSafePtrType (Type* type)
{
	return 
		(type->getTypeKindFlags () & TypeKindFlag_Ptr) && 
		(type->getFlags () & PtrTypeFlag_Safe);
}

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

} // namespace ct
} // namespace jnc
