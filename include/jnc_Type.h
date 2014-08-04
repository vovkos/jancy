// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ModuleItem.h"

namespace jnc {

class CTypeMgr;
class CArrayType;
class CStructType;
class CClassType;
class CPropertyType;
class CDataPtrType;
class CFunctionArg;
class CValue;
class CRuntime;

struct TDataPtrTypeTuple;
struct TSimplePropertyTypeTuple;
struct TFunctionArgTuple;

//.............................................................................

enum EType
{
	// primitive types (completely identified by EType)

	EType_Void,                // v
	EType_Variant,             // z
	EType_Bool,                // b

	// little-endian integers

	EType_Int8,                // is1
	EType_Int8_u,              // iu1
	EType_Int16,               // is2
	EType_Int16_u,             // iu2
	EType_Int32,               // is4
	EType_Int32_u,             // iu4
	EType_Int64,               // is8
	EType_Int64_u,             // iu8

	// big-endian integers

	EType_Int16_be,            // ibs2
	EType_Int16_beu,           // ibu2
	EType_Int32_be,            // ibs4
	EType_Int32_beu,           // ibu4
	EType_Int64_be,            // ibs8
	EType_Int64_beu,           // ibu8

	// floating point

	EType_Float,               // f4
	EType_Double,              // f8

	// derived types

	EType_Array,               // A
	EType_BitField,            // B

	// named types

	EType_Enum,                // E
	EType_Struct,              // SS/SP (struct/pointer struct)
	EType_Union,               // U
	EType_Class,               // CC/CO/CB/CA/CF/CD (class/object/box/reactor-iface/f-closure/d-closure)

	// function types

	EType_Function,            // F
	EType_Property,            // X

	// pointers & references

	EType_DataPtr,             // PD
	EType_DataRef,             // RD
	EType_ClassPtr,            // PC
	EType_ClassRef,            // RC
	EType_FunctionPtr,         // PF
	EType_FunctionRef,         // RF
	EType_PropertyPtr,         // PX
	EType_PropertyRef,         // RX

	// import types (resolved after linkage)

	EType_NamedImport,         // ZN
	EType_ImportPtr,           // ZP
	EType_ImportIntMod,        // ZI

	EType__Count,
	EType__EndianDelta = EType_Int16_be - EType_Int16,
	EType__PrimitiveTypeCount = EType_Double + 1,

	// aliases

#if (_AXL_PTR_BITNESS == 64)
	EType_Int_p    = EType_Int64,
	EType_Int_pu   = EType_Int64_u,
	EType_Int_pbe  = EType_Int64_be,
	EType_Int_pbeu = EType_Int64_beu,
#else
	EType_Int_p    = EType_Int32,
	EType_Int_pu   = EType_Int32_u,
	EType_Int_pbe  = EType_Int32_be,
	EType_Int_pbeu = EType_Int32_beu,
#endif

	EType_SizeT    = EType_Int_pu,
	EType_Int      = EType_Int32,
	EType_Char     = EType_Int8,
	EType_UChar    = EType_Int8_u,
	EType_Byte     = EType_Int8_u,
	EType_WChar    = EType_Int16,
	EType_Short    = EType_Int16,
	EType_UShort   = EType_Int16_u,
	EType_Word     = EType_Int16_u,
	EType_Long     = EType_Int32,
	EType_ULong    = EType_Int32_u,
	EType_DWord    = EType_Int32_u,
	EType_QWord    = EType_Int64_u,
};

//.............................................................................

enum EStdType
{
	EStdType_BytePtr,
	EStdType_ObjHdr,
	EStdType_ObjHdrPtr,
	EStdType_ObjectClass,
	EStdType_ObjectPtr,
	EStdType_SimpleFunction,
	EStdType_SimpleMulticast,
	EStdType_SimpleEventPtr,
	EStdType_Binder,
	EStdType_ReactorBindSite,
	EStdType_Scheduler,
	EStdType_SchedulerPtr,
	EStdType_FmtLiteral,
	EStdType_Guid,
	EStdType_Error,
	EStdType__Count,
};

//.............................................................................

enum ETypeModifier
{
	ETypeModifier_Unsigned    = 0x00000001,
	ETypeModifier_BigEndian   = 0x00000002,
	ETypeModifier_Const       = 0x00000004,
	ETypeModifier_DConst      = 0x00000008,
	ETypeModifier_Volatile    = 0x00000010,
	ETypeModifier_Weak        = 0x00000020,
	ETypeModifier_Thin        = 0x00000040,
	ETypeModifier_Safe        = 0x00000080,
	ETypeModifier_Cdecl       = 0x00000100,
	ETypeModifier_Stdcall     = 0x00000200,
	ETypeModifier_Array       = 0x00000400,
	ETypeModifier_Function    = 0x00000800,
	ETypeModifier_Property    = 0x00001000,
	ETypeModifier_Bindable    = 0x00002000,
	ETypeModifier_AutoGet     = 0x00004000,
	ETypeModifier_Indexed     = 0x00008000,
	ETypeModifier_Multicast   = 0x00010000,
	ETypeModifier_Event       = 0x00020000,
	ETypeModifier_DEvent      = 0x00040000,
	ETypeModifier_Reactor     = 0x00080000,
	ETypeModifier_Thiscall    = 0x00100000,
	ETypeModifier_Jnccall     = 0x00200000,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum ETypeModifierMask
{
	ETypeModifierMask_Integer =
		ETypeModifier_Unsigned |
		ETypeModifier_BigEndian,

	ETypeModifierMask_CallConv =
		ETypeModifier_Cdecl |
		ETypeModifier_Stdcall |
		ETypeModifier_Thiscall |
		ETypeModifier_Jnccall,

	ETypeModifierMask_Function =
		ETypeModifier_Function |
		ETypeModifierMask_CallConv ,

	ETypeModifierMask_Property =
		ETypeModifier_Property |
		ETypeModifierMask_CallConv |
		ETypeModifier_Const |
		ETypeModifier_Bindable |
		ETypeModifier_Indexed,

	ETypeModifierMask_DataPtr =
		ETypeModifier_Safe |
		ETypeModifier_Const |
		ETypeModifier_DConst |
		ETypeModifier_Volatile |
		ETypeModifier_Thin,

	ETypeModifierMask_ClassPtr =
		ETypeModifier_Safe |
		ETypeModifier_Const |
		ETypeModifier_DConst |
		ETypeModifier_Volatile |
		ETypeModifier_Event |
		ETypeModifier_DEvent |
		ETypeModifier_Weak,

	ETypeModifierMask_FunctionPtr =
		ETypeModifier_Safe |
		ETypeModifier_Weak |
		ETypeModifier_Thin,

	ETypeModifierMask_PropertyPtr =
		ETypeModifier_Safe |
		ETypeModifier_Weak |
		ETypeModifier_Thin,

	ETypeModifierMask_ImportPtr =
		ETypeModifierMask_DataPtr |
		ETypeModifierMask_ClassPtr |
		ETypeModifierMask_FunctionPtr |
		ETypeModifierMask_PropertyPtr,

	ETypeModifierMask_DeclPtr =
		ETypeModifier_Const |
		ETypeModifier_DConst |
		ETypeModifier_Volatile |
		ETypeModifier_Event |
		ETypeModifier_DEvent |
		ETypeModifier_Bindable |
		ETypeModifier_AutoGet,

	ETypeModifierMask_PtrKind =
		ETypeModifier_Weak |
		ETypeModifier_Thin,

	ETypeModifierMask_TypeKind =
		ETypeModifier_Function |
		ETypeModifier_Property |
		ETypeModifier_Multicast |
		ETypeModifier_Reactor,

	ETypeModifierMask_Const =
		ETypeModifier_Const |
		ETypeModifier_DConst |
		ETypeModifier_Event |
		ETypeModifier_DEvent,

	ETypeModifierMask_Event =
		ETypeModifier_Event |
		ETypeModifier_DEvent |
		ETypeModifier_Const |
		ETypeModifier_DConst |
		ETypeModifierMask_TypeKind,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
ETypeModifier
GetFirstTypeModifier (uint_t Modifiers)
{
	return (ETypeModifier) (1 << rtl::GetLoBitIdx (Modifiers));
}

const char*
GetTypeModifierString (ETypeModifier Modifier);

rtl::CString
GetTypeModifierString (uint_t Modifiers);

inline
const char*
GetFirstTypeModifierString (uint_t Modifiers)
{
	return GetTypeModifierString (GetFirstTypeModifier (Modifiers));
}

//.............................................................................

enum ETypeFlag
{
	ETypeFlag_Named        = 0x000100,
	ETypeFlag_Child        = 0x000200, // constructor has an implicit 'parent' arg
	ETypeFlag_Pod          = 0x000400, // plain-old-data
	ETypeFlag_GcRoot       = 0x000800, // is or contains gc-traceable pointers
	ETypeFlag_StructRet    = 0x001000, // return through hidden 1st arg (gcc32 callconv)
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum EPtrTypeFlag
{
	EPtrTypeFlag_Safe      = 0x0010000, // all ptr
	EPtrTypeFlag_Unused    = 0x0020000, // unused
	EPtrTypeFlag_Const     = 0x0040000, // class & data ptr
	EPtrTypeFlag_ConstD    = 0x0080000, // class & data ptr
	EPtrTypeFlag_Volatile  = 0x0100000, // class & data ptr
	EPtrTypeFlag_Event     = 0x0200000, // multicast-class only
	EPtrTypeFlag_EventD    = 0x0400000, // multicast-class only
	EPtrTypeFlag_Bindable  = 0x0800000, // multicast-class only
	EPtrTypeFlag_AutoGet   = 0x1000000, // data ptr only

	EPtrTypeFlag__AllMask  = 0x1ff0000,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
EPtrTypeFlag
GetFirstPtrTypeFlag (uint_t Flags)
{
	return (EPtrTypeFlag) (1 << rtl::GetLoBitIdx (Flags));
}

const char*
GetPtrTypeFlagString (EPtrTypeFlag Flag);

rtl::CString
GetPtrTypeFlagString (uint_t Flags);

rtl::CString
GetPtrTypeFlagSignature (uint_t Flags);

inline
const char*
GetFirstPtrTypeFlagString (uint_t Flags)
{
	return GetPtrTypeFlagString (GetFirstPtrTypeFlag (Flags));
}

uint_t
GetPtrTypeFlagsFromModifiers (uint_t Modifiers);

//.............................................................................

// data ptr

enum EDataPtrType
{
	EDataPtrType_Normal = 0,
	EDataPtrType_Lean,
	EDataPtrType_Thin,
	EDataPtrType__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
GetDataPtrTypeKindString (EDataPtrType PtrTypeKind);

//.............................................................................

// useful for simple checks

enum ETypeKindFlag
{
	ETypeKindFlag_Integer      = 0x00000001,
	ETypeKindFlag_Unsigned     = 0x00000002,
	ETypeKindFlag_BigEndian    = 0x00000004,
	ETypeKindFlag_Fp           = 0x00000008,
	ETypeKindFlag_Numeric      = 0x00000010,
	ETypeKindFlag_Aggregate    = 0x00000020,
	ETypeKindFlag_Named        = 0x00000100,
	ETypeKindFlag_Derivable    = 0x00000200,
	ETypeKindFlag_DataPtr      = 0x00000400,
	ETypeKindFlag_ClassPtr     = 0x00000800,
	ETypeKindFlag_FunctionPtr  = 0x00001000,
	ETypeKindFlag_PropertyPtr  = 0x00002000,
	ETypeKindFlag_Ptr          = 0x00004000,
	ETypeKindFlag_Ref          = 0x00008000,
	ETypeKindFlag_Import       = 0x00010000,
	ETypeKindFlag_Code         = 0x00020000,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

uint_t
GetTypeKindFlags (EType TypeKind);

//.............................................................................

// integer type utils

EType
GetInt32TypeKind (int32_t Integer);

EType
GetInt32TypeKind_u (uint32_t Integer);

EType
GetInt64TypeKind (int64_t Integer);

EType
GetInt64TypeKind_u (uint64_t Integer);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
EType
GetBigEndianIntegerTypeKind (EType TypeKind)
{
	return !(GetTypeKindFlags (TypeKind) & ETypeKindFlag_BigEndian) ?
		(EType) (TypeKind + EType__EndianDelta) :
		TypeKind;
}

inline
EType
GetLittleEndianIntegerTypeKind (EType TypeKind)
{
	return (GetTypeKindFlags (TypeKind) & ETypeKindFlag_BigEndian) ?
		(EType) (TypeKind - EType__EndianDelta) :
		TypeKind;
}

inline
EType
GetUnsignedIntegerTypeKind (EType TypeKind)
{
	return !(GetTypeKindFlags (TypeKind) & ETypeKindFlag_Unsigned) ?
		(EType) (TypeKind + 1) :
		TypeKind;
}

inline
EType
GetSignedIntegerTypeKind (EType TypeKind)
{
	return (GetTypeKindFlags (TypeKind) & ETypeKindFlag_Unsigned) ?
		(EType) (TypeKind - 1) :
		TypeKind;
}

inline
bool
IsEquivalentIntegerTypeKind (
	EType TypeKind1,
	EType TypeKind2
	)
{
	return GetSignedIntegerTypeKind (TypeKind1) == GetSignedIntegerTypeKind (TypeKind2);
}

//.............................................................................

rtl::CString
GetLlvmTypeString (llvm::Type* pLlvmType);

//.............................................................................

class CType: public CModuleItem
{
	friend class CTypeMgr;

protected:
	EType m_TypeKind;
	size_t m_Size;
	size_t m_AlignFactor;
	rtl::CStringHashTableMapIteratorT <CType*> m_TypeMapIt;
	rtl::CString m_Signature;
	rtl::CString m_TypeString;
	llvm::Type* m_pLlvmType;
	llvm::DIType m_LlvmDiType;

	CClassType* m_pBoxClassType;
	TSimplePropertyTypeTuple* m_pSimplePropertyTypeTuple;
	TFunctionArgTuple* m_pFunctionArgTuple;
	TDataPtrTypeTuple* m_pDataPtrTypeTuple;

public:
	CType ();

	EType
	GetTypeKind ()
	{
		return m_TypeKind;
	}

	uint_t
	GetTypeKindFlags ()
	{
		return jnc::GetTypeKindFlags (m_TypeKind);
	}

	size_t
	GetSize ()
	{
		return m_Size;
	}

	size_t
	GetAlignFactor ()
	{
		return m_AlignFactor;
	}

	rtl::CString
	GetSignature ()
	{
		return m_Signature;
	}

	rtl::CString
	GetTypeString ();

	rtl::CString
	GetLlvmTypeString ()
	{
		return jnc::GetLlvmTypeString (GetLlvmType ());
	}

	llvm::Type*
	GetLlvmType ();

	llvm::DIType
	GetLlvmDiType ();

	CValue
	GetUndefValue ();

	CValue
	GetZeroValue ();

	int
	Cmp (CType* pType)
	{
		return pType != this ? m_Signature.Cmp (pType->m_Signature) : 0;
	}

	CArrayType*
	GetArrayType (size_t ElementCount);

	CDataPtrType*
	GetDataPtrType (
		CNamespace* pAnchorNamespace,
		EType TypeKind,
		EDataPtrType PtrTypeKind = EDataPtrType_Normal,
		uint_t Flags = 0
		);

	CDataPtrType*
	GetDataPtrType (
		EType TypeKind,
		EDataPtrType PtrTypeKind = EDataPtrType_Normal,
		uint_t Flags = 0
		)
	{
		return GetDataPtrType (NULL, TypeKind, PtrTypeKind, Flags);
	}

	CDataPtrType*
	GetDataPtrType (
		EDataPtrType PtrTypeKind = EDataPtrType_Normal,
		uint_t Flags = 0
		)
	{
		return GetDataPtrType (EType_DataPtr, PtrTypeKind, Flags);
	}

	CDataPtrType*
	GetDataPtrType_c (
		EType TypeKind = EType_DataPtr,
		uint_t Flags = 0
		)
	{
		return GetDataPtrType (TypeKind, EDataPtrType_Thin, Flags);
	}

	CFunctionArg*
	GetSimpleFunctionArg (uint_t PtrTypeFlags = 0);

	CClassType*
	GetBoxClassType ();

	virtual
	void
	GcMark (
		CRuntime* pRuntime,
		void* p
		)
	{
		ASSERT (false);
	}

protected:
	virtual
	void
	PrepareTypeString ();

	virtual
	void
	PrepareLlvmType ();

	virtual
	void
	PrepareLlvmDiType ();

	virtual
	bool
	CalcLayout ()
	{
		ASSERT (m_Size && m_AlignFactor);
		return true;
	}
};

//.............................................................................

class CTypedef: public CUserModuleItem
{
	friend class CTypeMgr;

protected:
	CType* m_pType;

public:
	CTypedef ()
	{
		m_ItemKind = EModuleItem_Typedef;
		m_pType  = NULL;
	}

	CType*
	GetType ()
	{
		return m_pType;
	}
};

//.............................................................................

class CLazyStdType: public CLazyModuleItem
{
	friend class CTypeMgr;

protected:
	EStdType m_StdType;

public:
	CLazyStdType ()
	{
		m_StdType = (EStdType) -1;
	}

	virtual
	CModuleItem*
	GetActualItem ();
};

//.............................................................................

CType*
GetSimpleType (
	CModule* pModule,
	EType TypeKind
	);

CType*
GetSimpleType (
	CModule* pModule,
	EStdType StdTypeKind
	);

CType*
GetModuleItemType (CModuleItem* pItem);

//.............................................................................

bool
IsWeakPtrType (CType* pType);

CType*
GetWeakPtrType (CType* pType);

//.............................................................................

} // namespace jnc {

