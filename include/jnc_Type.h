// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Namespace.h"

namespace jnc {

class TypeMgr;
class ArrayType;
class StructType;
class ClassType;
class PropertyType;
class DataPtrType;
class FunctionArg;
class Value;
class Runtime;

struct DataPtrTypeTuple;
struct SimplePropertyTypeTuple;
struct FunctionArgTuple;

//.............................................................................

enum TypeKind
{
	// primitive types (completely identified by EType)

	TypeKind_Void,                // v
	TypeKind_Variant,             // z
	TypeKind_Bool,                // b

	// little-endian integers

	TypeKind_Int8,                // is1
	TypeKind_Int8_u,              // iu1
	TypeKind_Int16,               // is2
	TypeKind_Int16_u,             // iu2
	TypeKind_Int32,               // is4
	TypeKind_Int32_u,             // iu4
	TypeKind_Int64,               // is8
	TypeKind_Int64_u,             // iu8

	// big-endian integers

	TypeKind_Int16_be,            // ibs2
	TypeKind_Int16_beu,           // ibu2
	TypeKind_Int32_be,            // ibs4
	TypeKind_Int32_beu,           // ibu4
	TypeKind_Int64_be,            // ibs8
	TypeKind_Int64_beu,           // ibu8

	// floating point

	TypeKind_Float,               // f4
	TypeKind_Double,              // f8

	// derived types

	TypeKind_Array,               // A
	TypeKind_BitField,            // B

	// named types

	TypeKind_Enum,                // E
	TypeKind_Struct,              // SS/SP (struct/pointer struct)
	TypeKind_Union,               // U
	TypeKind_Class,               // CC/CO/CB/CA/CF/CD (class/object/box/reactor-iface/f-closure/d-closure)

	// function types

	TypeKind_Function,            // F
	TypeKind_Property,            // X

	// pointers & references

	TypeKind_DataPtr,             // PD
	TypeKind_DataRef,             // RD
	TypeKind_ClassPtr,            // PC
	TypeKind_ClassRef,            // RC
	TypeKind_FunctionPtr,         // PF
	TypeKind_FunctionRef,         // RF
	TypeKind_PropertyPtr,         // PX
	TypeKind_PropertyRef,         // RX

	// import types (resolved after linkage)

	TypeKind_NamedImport,         // ZN
	TypeKind_ImportPtr,           // ZP
	TypeKind_ImportIntMod,        // ZI

	TypeKind__Count,
	TypeKind__EndianDelta = TypeKind_Int16_be - TypeKind_Int16,
	TypeKind__PrimitiveTypeCount = TypeKind_Double + 1,

	// aliases

#if (_AXL_PTR_BITNESS == 64)
	TypeKind_Int_p    = TypeKind_Int64,
	TypeKind_Int_pu   = TypeKind_Int64_u,
	TypeKind_Int_pbe  = TypeKind_Int64_be,
	TypeKind_Int_pbeu = TypeKind_Int64_beu,
#else
	TypeKind_Int_p    = TypeKind_Int32,
	TypeKind_Int_pu   = TypeKind_Int32_u,
	TypeKind_Int_pbe  = TypeKind_Int32_be,
	TypeKind_Int_pbeu = TypeKind_Int32_beu,
#endif

	TypeKind_SizeT    = TypeKind_Int_pu,
	TypeKind_Int      = TypeKind_Int32,
	TypeKind_Int_u    = TypeKind_Int32_u,
	TypeKind_Char     = TypeKind_Int8,
	TypeKind_Char_u   = TypeKind_Int8_u,
	TypeKind_Byte     = TypeKind_Int8_u,
	TypeKind_Short    = TypeKind_Int16,
	TypeKind_Short_u  = TypeKind_Int16_u,
	TypeKind_Word     = TypeKind_Int16_u,
	TypeKind_DWord    = TypeKind_Int32_u,
	TypeKind_QWord    = TypeKind_Int64_u,

#if (_AXL_CPP == AXL_CPP_GCC && _AXL_PTR_BITNESS == 64)
	TypeKind_Long     = TypeKind_Int64,
	TypeKind_Long_u   = TypeKind_Int64_u,
#else
	TypeKind_Long     = TypeKind_Int32,
	TypeKind_Long_u   = TypeKind_Int32_u,
#endif
};

//.............................................................................

enum StdType
{
	StdType_BytePtr,
	StdType_ByteConstPtr,
	StdType_SimpleIfaceHdr,
	StdType_SimpleIfaceHdrPtr,
	StdType_ObjHdr,
	StdType_ObjHdrPtr,
	StdType_VariableObjHdr,
	StdType_ObjectClass,
	StdType_ObjectPtr,
	StdType_SimpleFunction,
	StdType_SimpleMulticast,
	StdType_SimpleEventPtr,
	StdType_Binder,
	StdType_ReactorBindSite,
	StdType_Scheduler,
	StdType_SchedulerPtr,
	StdType_FmtLiteral,
	StdType_Guid,
	StdType_Error,
	StdType_String,
	StdType_StringRef,
	StdType_StringBuilder,
	StdType_ConstBuffer,
	StdType_ConstBufferRef,
	StdType_BufferRef,
	StdType_Buffer,
	StdType_Int64Int64, // for system V coercion
	StdType_Fp64Fp64,   // for system V coercion
	StdType_Int64Fp64,  // for system V coercion
	StdType_Fp64Int64,  // for system V coercion
	StdType__Count,
};

//.............................................................................

enum StdTypedef
{
	StdTypedef_uint_t,
	StdTypedef_uintptr_t,
	StdTypedef_size_t,
	StdTypedef_uint8_t,
	StdTypedef_uchar_t,
	StdTypedef_byte_t,
	StdTypedef_uint16_t,
	StdTypedef_ushort_t,
	StdTypedef_word_t,
	StdTypedef_uint32_t,
	StdTypedef_dword_t,
	StdTypedef_uint64_t,
	StdTypedef_qword_t,
	StdTypedef__Count,
};

//.............................................................................

enum TypeModifier
{
	TypeModifier_Unsigned    = 0x00000001,
	TypeModifier_BigEndian   = 0x00000002,
	TypeModifier_Const       = 0x00000004,
	TypeModifier_ReadOnly    = 0x00000008,
	TypeModifier_Volatile    = 0x00000010,
	TypeModifier_Weak        = 0x00000020,
	TypeModifier_Thin        = 0x00000040,
	TypeModifier_Safe        = 0x00000080,
	TypeModifier_Cdecl       = 0x00000100,
	TypeModifier_Stdcall     = 0x00000200,
	TypeModifier_Array       = 0x00000400,
	TypeModifier_Function    = 0x00000800,
	TypeModifier_Property    = 0x00001000,
	TypeModifier_Bindable    = 0x00002000,
	TypeModifier_AutoGet     = 0x00004000,
	TypeModifier_Indexed     = 0x00008000,
	TypeModifier_Multicast   = 0x00010000,
	TypeModifier_Event       = 0x00020000,
	TypeModifier_Unused      = 0x00040000,
	TypeModifier_Reactor     = 0x00080000,
	TypeModifier_Thiscall    = 0x00100000,
	TypeModifier_Jnccall     = 0x00200000,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum TypeModifierMaskKind
{
	TypeModifierMaskKind_Integer =
		TypeModifier_Unsigned |
		TypeModifier_BigEndian,

	TypeModifierMaskKind_CallConv =
		TypeModifier_Cdecl |
		TypeModifier_Stdcall |
		TypeModifier_Thiscall |
		TypeModifier_Jnccall,

	TypeModifierMaskKind_Function =
		TypeModifier_Function |
		TypeModifierMaskKind_CallConv ,

	TypeModifierMaskKind_Property =
		TypeModifier_Property |
		TypeModifierMaskKind_CallConv |
		TypeModifier_Const |
		TypeModifier_Bindable |
		TypeModifier_Indexed,

	TypeModifierMaskKind_DataPtr =
		TypeModifier_Safe |
		TypeModifier_Const |
		TypeModifier_ReadOnly |
		TypeModifier_Volatile |
		TypeModifier_Thin,

	TypeModifierMaskKind_ClassPtr =
		TypeModifier_Safe |
		TypeModifier_Const |
		TypeModifier_ReadOnly |
		TypeModifier_Volatile |
		TypeModifier_Event |
		TypeModifier_Weak,

	TypeModifierMaskKind_FunctionPtr =
		TypeModifier_Safe |
		TypeModifier_Weak |
		TypeModifier_Thin,

	TypeModifierMaskKind_PropertyPtr =
		TypeModifier_Safe |
		TypeModifier_Weak |
		TypeModifier_Thin,

	TypeModifierMaskKind_ImportPtr =
		TypeModifierMaskKind_DataPtr |
		TypeModifierMaskKind_ClassPtr |
		TypeModifierMaskKind_FunctionPtr |
		TypeModifierMaskKind_PropertyPtr,

	TypeModifierMaskKind_DeclPtr =
		TypeModifier_Const |
		TypeModifier_ReadOnly |
		TypeModifier_Volatile |
		TypeModifier_Event |
		TypeModifier_Bindable |
		TypeModifier_AutoGet,

	TypeModifierMaskKind_PtrKind =
		TypeModifier_Weak |
		TypeModifier_Thin,

	TypeModifierMaskKind_TypeKind =
		TypeModifier_Function |
		TypeModifier_Property |
		TypeModifier_Multicast |
		TypeModifier_Reactor,

	TypeModifierMaskKind_Const =
		TypeModifier_Const |
		TypeModifier_ReadOnly |
		TypeModifier_Event,

	TypeModifierMaskKind_Event =
		TypeModifier_Event |
		TypeModifier_Const |
		TypeModifier_ReadOnly |
		TypeModifierMaskKind_TypeKind,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
TypeModifier
getFirstTypeModifier (uint_t modifiers)
{
	return (TypeModifier) (1 << rtl::getLoBitIdx (modifiers));
}

const char*
getTypeModifierString (TypeModifier modifier);

rtl::String
getTypeModifierString (uint_t modifiers);

inline
const char*
getFirstTypeModifierString (uint_t modifiers)
{
	return getTypeModifierString (getFirstTypeModifier (modifiers));
}

//.............................................................................

enum TypeFlag
{
	TypeFlag_Named     = 0x0100,
	TypeFlag_Child     = 0x0200, // constructor has an implicit 'parent' arg
	TypeFlag_Pod       = 0x0400, // plain-old-data
	TypeFlag_GcRoot    = 0x0800, // is or contains gc-traceable pointers
	TypeFlag_StructRet = 0x1000, // return through hidden 1st arg (gcc32 callconv)
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum PtrTypeFlag
{
	PtrTypeFlag_Safe      = 0x0010000, // all ptr
	PtrTypeFlag_Unused    = 0x0020000, // unused
	PtrTypeFlag_Const     = 0x0040000, // class & data ptr
	PtrTypeFlag_ReadOnly  = 0x0080000, // class & data ptr
	PtrTypeFlag_Volatile  = 0x0100000, // class & data ptr
	PtrTypeFlag_Event     = 0x0200000, // multicast-class only
	PtrTypeFlag_DualEvent = 0x0400000, // multicast-class only
	PtrTypeFlag_Bindable  = 0x0800000, // multicast-class only
	PtrTypeFlag_AutoGet   = 0x1000000, // data ptr only

	PtrTypeFlag__AllMask  = 0x1ff0000,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
PtrTypeFlag
getFirstPtrTypeFlag (uint_t flags)
{
	return (PtrTypeFlag) (1 << rtl::getLoBitIdx (flags));
}

const char*
getPtrTypeFlagString (PtrTypeFlag flag);

rtl::String
getPtrTypeFlagString (uint_t flags);

rtl::String
getPtrTypeFlagSignature (uint_t flags);

inline
const char*
getFirstPtrTypeFlagString (uint_t flags)
{
	return getPtrTypeFlagString (getFirstPtrTypeFlag (flags));
}

uint_t
getPtrTypeFlagsFromModifiers (uint_t modifiers);

//.............................................................................

// data ptr

enum DataPtrTypeKind
{
	DataPtrTypeKind_Normal = 0,
	DataPtrTypeKind_Lean,
	DataPtrTypeKind_Thin,
	DataPtrTypeKind__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getDataPtrTypeKindString (DataPtrTypeKind ptrTypeKind);

//.............................................................................

// useful for simple checks

enum TypeKindFlag
{
	TypeKindFlag_Integer      = 0x00000001,
	TypeKindFlag_Unsigned     = 0x00000002,
	TypeKindFlag_BigEndian    = 0x00000004,
	TypeKindFlag_Fp           = 0x00000008,
	TypeKindFlag_Numeric      = 0x00000010,
	TypeKindFlag_Aggregate    = 0x00000020,
	TypeKindFlag_Named        = 0x00000100,
	TypeKindFlag_Derivable    = 0x00000200,
	TypeKindFlag_DataPtr      = 0x00000400,
	TypeKindFlag_ClassPtr     = 0x00000800,
	TypeKindFlag_FunctionPtr  = 0x00001000,
	TypeKindFlag_PropertyPtr  = 0x00002000,
	TypeKindFlag_Ptr          = 0x00004000,
	TypeKindFlag_Ref          = 0x00008000,
	TypeKindFlag_Import       = 0x00010000,
	TypeKindFlag_Code         = 0x00020000,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

uint_t
getTypeKindFlags (TypeKind typeKind);

//.............................................................................

// integer type utils

TypeKind
getInt32TypeKind (int32_t integer);

TypeKind
getInt32TypeKind_u (uint32_t integer);

TypeKind
getInt64TypeKind (int64_t integer);

TypeKind
getInt64TypeKind_u (uint64_t integer);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
TypeKind
getBigEndianIntegerTypeKind (TypeKind typeKind)
{
	return !(getTypeKindFlags (typeKind) & TypeKindFlag_BigEndian) ?
		(TypeKind) (typeKind + TypeKind__EndianDelta) :
		typeKind;
}

inline
TypeKind
getLittleEndianIntegerTypeKind (TypeKind typeKind)
{
	return (getTypeKindFlags (typeKind) & TypeKindFlag_BigEndian) ?
		(TypeKind) (typeKind - TypeKind__EndianDelta) :
		typeKind;
}

inline
TypeKind
getUnsignedIntegerTypeKind (TypeKind typeKind)
{
	return !(getTypeKindFlags (typeKind) & TypeKindFlag_Unsigned) ?
		(TypeKind) (typeKind + 1) :
		typeKind;
}

inline
TypeKind
getSignedIntegerTypeKind (TypeKind typeKind)
{
	return (getTypeKindFlags (typeKind) & TypeKindFlag_Unsigned) ?
		(TypeKind) (typeKind - 1) :
		typeKind;
}

inline
bool
isEquivalentIntegerTypeKind (
	TypeKind typeKind1,
	TypeKind typeKind2
	)
{
	return getSignedIntegerTypeKind (typeKind1) == getSignedIntegerTypeKind (typeKind2);
}

//.............................................................................

rtl::String
getLlvmTypeString (llvm::Type* llvmType);

//.............................................................................

class Type: public ModuleItem
{
	friend class TypeMgr;
	friend class CdeclCallConv_gcc64;

protected:
	TypeKind m_typeKind;
	StdType m_stdType;
	size_t m_size;
	size_t m_alignment;
	rtl::StringHashTableMapIterator <Type*> m_typeMapIt;
	rtl::String m_signature;
	rtl::String m_typeString;
	llvm::Type* m_llvmType;
	llvm::DIType m_llvmDiType;

	ClassType* m_boxClassType;
	SimplePropertyTypeTuple* m_simplePropertyTypeTuple;
	FunctionArgTuple* m_functionArgTuple;
	DataPtrTypeTuple* m_dataPtrTypeTuple;

public:
	Type ();

	TypeKind
	getTypeKind ()
	{
		return m_typeKind;
	}

	uint_t
	getTypeKindFlags ()
	{
		return jnc::getTypeKindFlags (m_typeKind);
	}

	StdType
	getStdType ()
	{
		return m_stdType;
	}

	size_t
	getSize ()
	{
		return m_size;
	}

	size_t
	getAlignment ()
	{
		return m_alignment;
	}

	rtl::String
	getSignature ()
	{
		return m_signature;
	}

	rtl::String
	getTypeString ();

	rtl::String
	getLlvmTypeString ()
	{
		return jnc::getLlvmTypeString (getLlvmType ());
	}

	llvm::Type*
	getLlvmType ();

	llvm::DIType
	getLlvmDiType ();

	Value
	getUndefValue ();

	Value
	getZeroValue ();

	int
	cmp (Type* type)
	{
		return type != this ? m_signature.cmp (type->m_signature) : 0;
	}

	ArrayType*
	getArrayType (size_t elementCount);

	DataPtrType*
	getDataPtrType (
		Namespace* anchorNamespace,
		TypeKind typeKind,
		DataPtrTypeKind ptrTypeKind = DataPtrTypeKind_Normal,
		uint_t flags = 0
		);

	DataPtrType*
	getDataPtrType (
		TypeKind typeKind,
		DataPtrTypeKind ptrTypeKind = DataPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return getDataPtrType (NULL, typeKind, ptrTypeKind, flags);
	}

	DataPtrType*
	getDataPtrType (
		DataPtrTypeKind ptrTypeKind = DataPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return getDataPtrType (TypeKind_DataPtr, ptrTypeKind, flags);
	}

	DataPtrType*
	getDataPtrType_c (
		TypeKind typeKind = TypeKind_DataPtr,
		uint_t flags = 0
		)
	{
		return getDataPtrType (typeKind, DataPtrTypeKind_Thin, flags);
	}

	FunctionArg*
	getSimpleFunctionArg (uint_t ptrTypeFlags = 0);

	ClassType*
	getBoxClassType ();

	virtual
	void
	gcMark (
		Runtime* runtime,
		void* p
		)
	{
		ASSERT (false);
	}

protected:
	virtual
	void
	prepareTypeString ();

	virtual
	void
	prepareLlvmType ();

	virtual
	void
	prepareLlvmDiType ();

	virtual
	bool
	calcLayout ()
	{
		ASSERT (m_size && m_alignment);
		return true;
	}
};

//.............................................................................

class NamedType:
	public Type,
	public Namespace
{
	friend class Parser;

public:
	NamedType ()
	{
		m_namespaceKind = NamespaceKind_Type;
		m_itemDecl = this;
	}
};

//.............................................................................

class Typedef: public UserModuleItem
{
	friend class TypeMgr;

protected:
	Type* m_type;

public:
	Typedef ()
	{
		m_itemKind = ModuleItemKind_Typedef;
		m_type  = NULL;
	}

	Type*
	getType ()
	{
		return m_type;
	}
};

//.............................................................................

class LazyStdType: public LazyModuleItem
{
	friend class TypeMgr;

protected:
	StdType m_stdType;

public:
	LazyStdType ()
	{
		m_stdType = (StdType) -1;
	}

	virtual
	ModuleItem*
	getActualItem ();
};

//.............................................................................

Type*
getSimpleType (
	TypeKind typeKind,
	Module* module
	);

Type*
getSimpleType (
	StdType stdType,
	Module* module
	);

Type*
getModuleItemType (ModuleItem* item);

//.............................................................................

bool
isWeakPtrType (Type* type);

Type*
getWeakPtrType (Type* type);

//.............................................................................

} // namespace jnc {

