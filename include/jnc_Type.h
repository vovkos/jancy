#pragma once

#define _JNC_TYPE_H

#include "jnc_Def.h"

//.............................................................................

enum jnc_TypeKind
{
	// primitive types (completely identified by EType)

	jnc_TypeKind_Void,                // v
	jnc_TypeKind_Variant,             // z
	jnc_TypeKind_Bool,                // b

	// little-endian integers

	jnc_TypeKind_Int8,                // is1
	jnc_TypeKind_Int8_u,              // iu1
	jnc_TypeKind_Int16,               // is2
	jnc_TypeKind_Int16_u,             // iu2
	jnc_TypeKind_Int32,               // is4
	jnc_TypeKind_Int32_u,             // iu4
	jnc_TypeKind_Int64,               // is8
	jnc_TypeKind_Int64_u,             // iu8

	// big-endian integers

	jnc_TypeKind_Int16_be,            // ibs2
	jnc_TypeKind_Int16_beu,           // ibu2
	jnc_TypeKind_Int32_be,            // ibs4
	jnc_TypeKind_Int32_beu,           // ibu4
	jnc_TypeKind_Int64_be,            // ibs8
	jnc_TypeKind_Int64_beu,           // ibu8

	// floating point

	jnc_TypeKind_Float,               // f4
	jnc_TypeKind_Double,              // f8

	// derived types

	jnc_TypeKind_Array,               // A
	jnc_TypeKind_BitField,            // B

	// named types

	jnc_TypeKind_Enum,                // E
	jnc_TypeKind_Struct,              // S
	jnc_TypeKind_Union,               // U
	jnc_TypeKind_Class,               // CC/CO/CB/CA/CF/CD (class/object/box/reactor-iface/f-closure/d-closure)

	// function types

	jnc_TypeKind_Function,            // F
	jnc_TypeKind_Property,            // X

	// pointers & references

	jnc_TypeKind_DataPtr,             // PD
	jnc_TypeKind_DataRef,             // RD
	jnc_TypeKind_ClassPtr,            // PC
	jnc_TypeKind_ClassRef,            // RC
	jnc_TypeKind_FunctionPtr,         // PF
	jnc_TypeKind_FunctionRef,         // RF
	jnc_TypeKind_PropertyPtr,         // PX
	jnc_TypeKind_PropertyRef,         // RX

	// import types (resolved after linkage)

	jnc_TypeKind_NamedImport,         // ZN
	jnc_TypeKind_ImportPtr,           // ZP
	jnc_TypeKind_ImportIntMod,        // ZI

	// meta

	jnc_TypeKind__Count,
	jnc_TypeKind__EndianDelta = jnc_TypeKind_Int16_be - jnc_TypeKind_Int16,
	jnc_TypeKind__PrimitiveTypeCount = jnc_TypeKind_Double + 1,

	// aliases

#if (_AXL_PTR_BITNESS == 64)
	jnc_TypeKind_IntPtr     = jnc_TypeKind_Int64,
	jnc_TypeKind_IntPtr_u   = jnc_TypeKind_Int64_u,
	jnc_TypeKind_IntPtr_be  = jnc_TypeKind_Int64_be,
	jnc_TypeKind_IntPtr_beu = jnc_TypeKind_Int64_beu,
#else
	jnc_TypeKind_IntPtr     = jnc_TypeKind_Int32,
	jnc_TypeKind_IntPtr_u   = jnc_TypeKind_Int32_u,
	jnc_TypeKind_IntPtr_be  = jnc_TypeKind_Int32_be,
	jnc_TypeKind_IntPtr_beu = jnc_TypeKind_Int32_beu,
#endif

	jnc_TypeKind_SizeT    = jnc_TypeKind_IntPtr_u,
	jnc_TypeKind_Int      = jnc_TypeKind_Int32,
	jnc_TypeKind_Int_u    = jnc_TypeKind_Int32_u,
	jnc_TypeKind_Char     = jnc_TypeKind_Int8,
	jnc_TypeKind_Char_u   = jnc_TypeKind_Int8_u,
	jnc_TypeKind_Byte     = jnc_TypeKind_Int8_u,
	jnc_TypeKind_Short    = jnc_TypeKind_Int16,
	jnc_TypeKind_Short_u  = jnc_TypeKind_Int16_u,
	jnc_TypeKind_Word     = jnc_TypeKind_Int16_u,
	jnc_TypeKind_DWord    = jnc_TypeKind_Int32_u,
	jnc_TypeKind_QWord    = jnc_TypeKind_Int64_u,

#if (_AXL_CPP == AXL_CPP_GCC && _AXL_PTR_BITNESS == 64)
	jnc_TypeKind_Long     = jnc_TypeKind_Int64,
	jnc_TypeKind_Long_u   = jnc_TypeKind_Int64_u,
#else
	jnc_TypeKind_Long     = jnc_TypeKind_Int32,
	jnc_TypeKind_Long_u   = jnc_TypeKind_Int32_u,
#endif
};

//.............................................................................

JNC_EXTERN_C
jnc_TypeKind
jnc_Type_getTypeKind (jnc_Type* type);

JNC_EXTERN_C
size_t
jnc_Type_getSize (jnc_Type* type);

JNC_EXTERN_C
int
jnc_Type_cmp (
	jnc_Type* type,
	jnc_Type* type2
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Type
{
	jnc_TypeKind
	getTypeKind ()
	{
		return jnc_Type_getTypeKind (this);
	}

	size_t
	getSize ()
	{
		return jnc_Type_getSize (this);
	}

	int
	cmp (jnc_Type* type)
	{
		return jnc_Type_cmp (this, type);
	}
};

#endif // _JNC_CORE

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

typedef jnc_TypeKind TypeKind;

const TypeKind 
	TypeKind_Void                = jnc_TypeKind_Void,
	TypeKind_Variant             = jnc_TypeKind_Variant,
	TypeKind_Bool                = jnc_TypeKind_Bool,
	TypeKind_Int8                = jnc_TypeKind_Int8,
	TypeKind_Int8_u              = jnc_TypeKind_Int8_u,
	TypeKind_Int16               = jnc_TypeKind_Int16,
	TypeKind_Int16_u             = jnc_TypeKind_Int16_u,
	TypeKind_Int32               = jnc_TypeKind_Int32,
	TypeKind_Int32_u             = jnc_TypeKind_Int32_u,
	TypeKind_Int64               = jnc_TypeKind_Int64,
	TypeKind_Int64_u             = jnc_TypeKind_Int64_u,
	TypeKind_Int16_be            = jnc_TypeKind_Int16_be,
	TypeKind_Int16_beu           = jnc_TypeKind_Int16_beu,
	TypeKind_Int32_be            = jnc_TypeKind_Int32_be,
	TypeKind_Int32_beu           = jnc_TypeKind_Int32_beu,
	TypeKind_Int64_be            = jnc_TypeKind_Int64_be,
	TypeKind_Int64_beu           = jnc_TypeKind_Int64_beu,
	TypeKind_Float               = jnc_TypeKind_Float,
	TypeKind_Double              = jnc_TypeKind_Double,
	TypeKind_Array               = jnc_TypeKind_Array,
	TypeKind_BitField            = jnc_TypeKind_BitField,
	TypeKind_Enum                = jnc_TypeKind_Enum,
	TypeKind_Struct              = jnc_TypeKind_Struct,
	TypeKind_Union               = jnc_TypeKind_Union,
	TypeKind_Class               = jnc_TypeKind_Class,
	TypeKind_Function            = jnc_TypeKind_Function,
	TypeKind_Property            = jnc_TypeKind_Property,
	TypeKind_DataPtr             = jnc_TypeKind_DataPtr,
	TypeKind_DataRef             = jnc_TypeKind_DataRef,
	TypeKind_ClassPtr            = jnc_TypeKind_ClassPtr,
	TypeKind_ClassRef            = jnc_TypeKind_ClassRef,
	TypeKind_FunctionPtr         = jnc_TypeKind_FunctionPtr,
	TypeKind_FunctionRef         = jnc_TypeKind_FunctionRef,
	TypeKind_PropertyPtr         = jnc_TypeKind_PropertyPtr,
	TypeKind_PropertyRef         = jnc_TypeKind_PropertyRef,
	TypeKind_NamedImport         = jnc_TypeKind_NamedImport,
	TypeKind_ImportPtr           = jnc_TypeKind_ImportPtr,
	TypeKind_ImportIntMod        = jnc_TypeKind_ImportIntMod,
	TypeKind__Count              = jnc_TypeKind__Count,
	TypeKind__EndianDelta        = jnc_TypeKind__EndianDelta,
	TypeKind__PrimitiveTypeCount = jnc_TypeKind__PrimitiveTypeCount,
	TypeKind_IntPtr              = jnc_TypeKind_IntPtr,
	TypeKind_IntPtr_u            = jnc_TypeKind_IntPtr_u,
	TypeKind_IntPtr_be           = jnc_TypeKind_IntPtr_be,
	TypeKind_IntPtr_beu          = jnc_TypeKind_IntPtr_beu,
	TypeKind_SizeT               = jnc_TypeKind_SizeT,
	TypeKind_Int                 = jnc_TypeKind_Int,
	TypeKind_Int_u               = jnc_TypeKind_Int_u,
	TypeKind_Char                = jnc_TypeKind_Char,
	TypeKind_Char_u              = jnc_TypeKind_Char_u,
	TypeKind_Byte                = jnc_TypeKind_Byte,
	TypeKind_Short               = jnc_TypeKind_Short,
	TypeKind_Short_u             = jnc_TypeKind_Short_u,
	TypeKind_Word                = jnc_TypeKind_Word,
	TypeKind_DWord               = jnc_TypeKind_Int32_u,
	TypeKind_QWord               = jnc_TypeKind_Int64_u,
	TypeKind_Long                = jnc_TypeKind_Int32,
	TypeKind_Long_u              = jnc_TypeKind_Int32_u;


//.............................................................................

} // namespace jnc

#endif // __cplusplus
