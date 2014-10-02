// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ModuleItem.h"

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
	TypeKind_Char     = TypeKind_Int8,
	TypeKind_UChar    = TypeKind_Int8_u,
	TypeKind_Byte     = TypeKind_Int8_u,
	TypeKind_WChar    = TypeKind_Int16,
	TypeKind_Short    = TypeKind_Int16,
	TypeKind_UShort   = TypeKind_Int16_u,
	TypeKind_Word     = TypeKind_Int16_u,
	TypeKind_Long     = TypeKind_Int32,
	TypeKind_ULong    = TypeKind_Int32_u,
	TypeKind_DWord    = TypeKind_Int32_u,
	TypeKind_QWord    = TypeKind_Int64_u,
};

//.............................................................................

enum StdTypeKind
{
	StdTypeKind_BytePtr,
	StdTypeKind_ObjHdr,
	StdTypeKind_ObjHdrPtr,
	StdTypeKind_ObjectClass,
	StdTypeKind_ObjectPtr,
	StdTypeKind_SimpleFunction,
	StdTypeKind_SimpleMulticast,
	StdTypeKind_SimpleEventPtr,
	StdTypeKind_Binder,
	StdTypeKind_ReactorBindSite,
	StdTypeKind_Scheduler,
	StdTypeKind_SchedulerPtr,
	StdTypeKind_FmtLiteral,
	StdTypeKind_Guid,
	StdTypeKind_Error,
	StdTypeKind_Int64Int64, // for system V coercion
	StdTypeKind_Fp64Fp64,   // for system V coercion
	StdTypeKind_Int64Fp64,  // for system V coercion
	StdTypeKind_Fp64Int64,  // for system V coercion
	StdTypeKind__Count,
};

//.............................................................................

enum TypeModifierKind
{
	TypeModifierKind_Unsigned    = 0x00000001,
	TypeModifierKind_BigEndian   = 0x00000002,
	TypeModifierKind_Const       = 0x00000004,
	TypeModifierKind_DConst      = 0x00000008,
	TypeModifierKind_Volatile    = 0x00000010,
	TypeModifierKind_Weak        = 0x00000020,
	TypeModifierKind_Thin        = 0x00000040,
	TypeModifierKind_Safe        = 0x00000080,
	TypeModifierKind_Cdecl       = 0x00000100,
	TypeModifierKind_Stdcall     = 0x00000200,
	TypeModifierKind_Array       = 0x00000400,
	TypeModifierKind_Function    = 0x00000800,
	TypeModifierKind_Property    = 0x00001000,
	TypeModifierKind_Bindable    = 0x00002000,
	TypeModifierKind_AutoGet     = 0x00004000,
	TypeModifierKind_Indexed     = 0x00008000,
	TypeModifierKind_Multicast   = 0x00010000,
	TypeModifierKind_Event       = 0x00020000,
	TypeModifierKind_DEvent      = 0x00040000,
	TypeModifierKind_Reactor     = 0x00080000,
	TypeModifierKind_Thiscall    = 0x00100000,
	TypeModifierKind_Jnccall     = 0x00200000,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum TypeModifierMaskKind
{
	TypeModifierMaskKind_Integer =
		TypeModifierKind_Unsigned |
		TypeModifierKind_BigEndian,

	TypeModifierMaskKind_CallConv =
		TypeModifierKind_Cdecl |
		TypeModifierKind_Stdcall |
		TypeModifierKind_Thiscall |
		TypeModifierKind_Jnccall,

	TypeModifierMaskKind_Function =
		TypeModifierKind_Function |
		TypeModifierMaskKind_CallConv ,

	TypeModifierMaskKind_Property =
		TypeModifierKind_Property |
		TypeModifierMaskKind_CallConv |
		TypeModifierKind_Const |
		TypeModifierKind_Bindable |
		TypeModifierKind_Indexed,

	TypeModifierMaskKind_DataPtr =
		TypeModifierKind_Safe |
		TypeModifierKind_Const |
		TypeModifierKind_DConst |
		TypeModifierKind_Volatile |
		TypeModifierKind_Thin,

	TypeModifierMaskKind_ClassPtr =
		TypeModifierKind_Safe |
		TypeModifierKind_Const |
		TypeModifierKind_DConst |
		TypeModifierKind_Volatile |
		TypeModifierKind_Event |
		TypeModifierKind_DEvent |
		TypeModifierKind_Weak,

	TypeModifierMaskKind_FunctionPtr =
		TypeModifierKind_Safe |
		TypeModifierKind_Weak |
		TypeModifierKind_Thin,

	TypeModifierMaskKind_PropertyPtr =
		TypeModifierKind_Safe |
		TypeModifierKind_Weak |
		TypeModifierKind_Thin,

	TypeModifierMaskKind_ImportPtr =
		TypeModifierMaskKind_DataPtr |
		TypeModifierMaskKind_ClassPtr |
		TypeModifierMaskKind_FunctionPtr |
		TypeModifierMaskKind_PropertyPtr,

	TypeModifierMaskKind_DeclPtr =
		TypeModifierKind_Const |
		TypeModifierKind_DConst |
		TypeModifierKind_Volatile |
		TypeModifierKind_Event |
		TypeModifierKind_DEvent |
		TypeModifierKind_Bindable |
		TypeModifierKind_AutoGet,

	TypeModifierMaskKind_PtrKind =
		TypeModifierKind_Weak |
		TypeModifierKind_Thin,

	TypeModifierMaskKind_TypeKind =
		TypeModifierKind_Function |
		TypeModifierKind_Property |
		TypeModifierKind_Multicast |
		TypeModifierKind_Reactor,

	TypeModifierMaskKind_Const =
		TypeModifierKind_Const |
		TypeModifierKind_DConst |
		TypeModifierKind_Event |
		TypeModifierKind_DEvent,

	TypeModifierMaskKind_Event =
		TypeModifierKind_Event |
		TypeModifierKind_DEvent |
		TypeModifierKind_Const |
		TypeModifierKind_DConst |
		TypeModifierMaskKind_TypeKind,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
TypeModifierKind
getFirstTypeModifier (uint_t modifiers)
{
	return (TypeModifierKind) (1 << rtl::getLoBitIdx (modifiers));
}

const char*
getTypeModifierString (TypeModifierKind modifier);

rtl::String
getTypeModifierString (uint_t modifiers);

inline
const char*
getFirstTypeModifierString (uint_t modifiers)
{
	return getTypeModifierString (getFirstTypeModifier (modifiers));
}

//.............................................................................

enum TypeFlagKind
{
	TypeFlagKind_Named        = 0x0100,
	TypeFlagKind_Child        = 0x0200, // constructor has an implicit 'parent' arg
	TypeFlagKind_Pod          = 0x0400, // plain-old-data
	TypeFlagKind_GcRoot       = 0x0800, // is or contains gc-traceable pointers
	TypeFlagKind_StructRet    = 0x1000, // return through hidden 1st arg (gcc32 callconv)
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum PtrTypeFlagKind
{
	PtrTypeFlagKind_Safe      = 0x0010000, // all ptr
	PtrTypeFlagKind_Unused    = 0x0020000, // unused
	PtrTypeFlagKind_Const     = 0x0040000, // class & data ptr
	PtrTypeFlagKind_ConstD    = 0x0080000, // class & data ptr
	PtrTypeFlagKind_Volatile  = 0x0100000, // class & data ptr
	PtrTypeFlagKind_Event     = 0x0200000, // multicast-class only
	PtrTypeFlagKind_EventD    = 0x0400000, // multicast-class only
	PtrTypeFlagKind_Bindable  = 0x0800000, // multicast-class only
	PtrTypeFlagKind_AutoGet   = 0x1000000, // data ptr only

	PtrTypeFlagKind__AllMask  = 0x1ff0000,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
PtrTypeFlagKind
getFirstPtrTypeFlag (uint_t flags)
{
	return (PtrTypeFlagKind) (1 << rtl::getLoBitIdx (flags));
}

const char*
getPtrTypeFlagString (PtrTypeFlagKind flag);

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

enum TypeKindFlagKind
{
	TypeKindFlagKind_Integer      = 0x00000001,
	TypeKindFlagKind_Unsigned     = 0x00000002,
	TypeKindFlagKind_BigEndian    = 0x00000004,
	TypeKindFlagKind_Fp           = 0x00000008,
	TypeKindFlagKind_Numeric      = 0x00000010,
	TypeKindFlagKind_Aggregate    = 0x00000020,
	TypeKindFlagKind_Named        = 0x00000100,
	TypeKindFlagKind_Derivable    = 0x00000200,
	TypeKindFlagKind_DataPtr      = 0x00000400,
	TypeKindFlagKind_ClassPtr     = 0x00000800,
	TypeKindFlagKind_FunctionPtr  = 0x00001000,
	TypeKindFlagKind_PropertyPtr  = 0x00002000,
	TypeKindFlagKind_Ptr          = 0x00004000,
	TypeKindFlagKind_Ref          = 0x00008000,
	TypeKindFlagKind_Import       = 0x00010000,
	TypeKindFlagKind_Code         = 0x00020000,
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
	return !(getTypeKindFlags (typeKind) & TypeKindFlagKind_BigEndian) ?
		(TypeKind) (typeKind + TypeKind__EndianDelta) :
		typeKind;
}

inline
TypeKind
getLittleEndianIntegerTypeKind (TypeKind typeKind)
{
	return (getTypeKindFlags (typeKind) & TypeKindFlagKind_BigEndian) ?
		(TypeKind) (typeKind - TypeKind__EndianDelta) :
		typeKind;
}

inline
TypeKind
getUnsignedIntegerTypeKind (TypeKind typeKind)
{
	return !(getTypeKindFlags (typeKind) & TypeKindFlagKind_Unsigned) ?
		(TypeKind) (typeKind + 1) :
		typeKind;
}

inline
TypeKind
getSignedIntegerTypeKind (TypeKind typeKind)
{
	return (getTypeKindFlags (typeKind) & TypeKindFlagKind_Unsigned) ?
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
	size_t m_size;
	size_t m_alignFactor;
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

	size_t
	getSize ()
	{
		return m_size;
	}

	size_t
	getAlignFactor ()
	{
		return m_alignFactor;
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
		ASSERT (m_size && m_alignFactor);
		return true;
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
	StdTypeKind m_stdType;

public:
	LazyStdType ()
	{
		m_stdType = (StdTypeKind) -1;
	}

	virtual
	ModuleItem*
	getActualItem ();
};

//.............................................................................

Type*
getSimpleType (
	Module* module,
	TypeKind typeKind
	);

Type*
getSimpleType (
	Module* module,
	StdTypeKind stdTypeKind
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

