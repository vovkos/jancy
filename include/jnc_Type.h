//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#pragma once

#define _JNC_TYPE_H

#include "jnc_ModuleItem.h"

//..............................................................................

/**

\defgroup type-subsystem Type Subsystem
	\ingroup module-subsystem

	This section describes facilities used for accessing the Jancy type system.

\defgroup type Type
	\ingroup type-subsystem
	\import{jnc_Type.h}

	This type is the root of type hierarchy in Jancy. All the other types are *inherited* from it.

\addtogroup type
@{

\struct jnc_Type
	\verbatim

	Opaque structure used as a handle to Jancy root type.

	Use functions from the `Type` group to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

enum jnc_TypeKind
{
	// primitive types (completely identified by TypeKind)

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

	// when generating documentation, we want to keep typedef shadow in declarations

	jnc_TypeKind_TypedefShadow,       // T

	// meta

	jnc_TypeKind__Count,
	jnc_TypeKind__EndianDelta = jnc_TypeKind_Int16_be - jnc_TypeKind_Int16,
	jnc_TypeKind__PrimitiveTypeCount = jnc_TypeKind_Double + 1,

	// aliases

#if (JNC_PTR_BITS == 64)
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
	jnc_TypeKind_Long     = jnc_TypeKind_Int64,
	jnc_TypeKind_Long_u   = jnc_TypeKind_Int64_u,
	jnc_TypeKind_DWord    = jnc_TypeKind_Int32_u,
	jnc_TypeKind_QWord    = jnc_TypeKind_Int64_u,
};

typedef enum jnc_TypeKind jnc_TypeKind;

//..............................................................................

// useful for simple checks

enum jnc_TypeKindFlag
{
	jnc_TypeKindFlag_Integer      = 0x00000001,
	jnc_TypeKindFlag_Unsigned     = 0x00000002,
	jnc_TypeKindFlag_BigEndian    = 0x00000004,
	jnc_TypeKindFlag_Fp           = 0x00000008,
	jnc_TypeKindFlag_Numeric      = 0x00000010,
	jnc_TypeKindFlag_Aggregate    = 0x00000020,
	jnc_TypeKindFlag_Named        = 0x00000100,
	jnc_TypeKindFlag_Derivable    = 0x00000200,
	jnc_TypeKindFlag_DataPtr      = 0x00000400,
	jnc_TypeKindFlag_ClassPtr     = 0x00000800,
	jnc_TypeKindFlag_FunctionPtr  = 0x00001000,
	jnc_TypeKindFlag_PropertyPtr  = 0x00002000,
	jnc_TypeKindFlag_Ptr          = 0x00004000,
	jnc_TypeKindFlag_Ref          = 0x00008000,
	jnc_TypeKindFlag_Import       = 0x00010000,
	jnc_TypeKindFlag_Code         = 0x00020000,
	jnc_TypeKindFlag_Nullable     = 0x00040000,
	jnc_TypeKindFlag_ErrorCode    = 0x00080000,
};

typedef enum jnc_TypeKindFlag jnc_TypeKindFlag;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
uint_t
jnc_getTypeKindFlags(jnc_TypeKind typeKind);

//..............................................................................

enum jnc_TypeFlag
{
	jnc_TypeFlag_Named     = 0x0100,
	jnc_TypeFlag_Pod       = 0x0200, // plain-old-data
	jnc_TypeFlag_GcRoot    = 0x0400, // is or contains gc-traceable pointers
	jnc_TypeFlag_StructRet = 0x0800, // return through hidden 1st arg (gcc32 callconv)
	jnc_TypeFlag_NoStack   = 0x1000, // try to avoid allocation on stack
	jnc_TypeFlag_Dynamic   = 0x2000, // dynamic struct/union/array
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum jnc_PtrTypeFlag
{
	jnc_PtrTypeFlag_Safe       = 0x0010000, // all ptr
	jnc_PtrTypeFlag_Const      = 0x0020000, // class & data ptr
	jnc_PtrTypeFlag_ReadOnly   = 0x0040000, // class & data ptr
	jnc_PtrTypeFlag_CMut       = 0x0080000, // class & data ptr
	jnc_PtrTypeFlag_Volatile   = 0x0100000, // class & data ptr
	jnc_PtrTypeFlag_Event      = 0x0200000, // multicast-class only
	jnc_PtrTypeFlag_DualEvent  = 0x0400000, // multicast-class only
	jnc_PtrTypeFlag_Bindable   = 0x0800000, // multicast-class only
	jnc_PtrTypeFlag_AutoGet    = 0x1000000, // data ptr only
	jnc_PtrTypeFlag_DualTarget = 0x2000000, // data ptr only

	jnc_PtrTypeFlag__Dual =
		jnc_PtrTypeFlag_ReadOnly |
		jnc_PtrTypeFlag_CMut |
		jnc_PtrTypeFlag_DualEvent |
		jnc_PtrTypeFlag_DualTarget,
};

//..............................................................................

// commonly used non-primitive types

enum jnc_StdType
{
	jnc_StdType_BytePtr,
	jnc_StdType_CharConstPtr,
	jnc_StdType_IfaceHdr,
	jnc_StdType_IfaceHdrPtr,
	jnc_StdType_Box,
	jnc_StdType_BoxPtr,
	jnc_StdType_DataBox,
	jnc_StdType_DataBoxPtr,
	jnc_StdType_DetachedDataBox,
	jnc_StdType_DetachedDataBoxPtr,
	jnc_StdType_AbstractClass,
	jnc_StdType_AbstractClassPtr,
	jnc_StdType_AbstractData,
	jnc_StdType_AbstractDataPtr,
	jnc_StdType_SimpleFunction,
	jnc_StdType_SimpleMulticast,
	jnc_StdType_SimpleEventPtr,
	jnc_StdType_Binder,
	jnc_StdType_GcTriggers,
	jnc_StdType_GcStats,
	jnc_StdType_Scheduler,
	jnc_StdType_SchedulerPtr,
	jnc_StdType_RegexMatch,
	jnc_StdType_RegexState,
	jnc_StdType_RegexDfa,
	jnc_StdType_Promise,
	jnc_StdType_PromisePtr,
	jnc_StdType_Promisifier,
	jnc_StdType_DynamicLib,
	jnc_StdType_DynamicLayout,
	jnc_StdType_FmtLiteral,
	jnc_StdType_Int64Int64, // for system V coercion
	jnc_StdType_Fp64Fp64,   // for system V coercion
	jnc_StdType_Int64Fp64,  // for system V coercion
	jnc_StdType_Fp64Int64,  // for system V coercion
	jnc_StdType_DataPtrValidator,
	jnc_StdType_DataPtrValidatorPtr,
	jnc_StdType_DataPtrStruct,
	jnc_StdType_FunctionPtrStruct,
	jnc_StdType_PropertyPtrStruct = jnc_StdType_FunctionPtrStruct,
	jnc_StdType_VariantStruct,
	jnc_StdType_GcShadowStackFrame,
	jnc_StdType_SjljFrame,
	jnc_StdType_ReactorBase,
	jnc_StdType_ReactorClosure,
	jnc_StdType__Count,
};

typedef enum jnc_StdType jnc_StdType;

//..............................................................................

// data ptr

enum jnc_DataPtrTypeKind
{
	jnc_DataPtrTypeKind_Normal = 0,
	jnc_DataPtrTypeKind_Lean,
	jnc_DataPtrTypeKind_Thin,
	jnc_DataPtrTypeKind__Count,
};

typedef enum jnc_DataPtrTypeKind jnc_DataPtrTypeKind;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
const char*
jnc_getDataPtrTypeKindString(jnc_DataPtrTypeKind ptrTypeKind);

//..............................................................................

JNC_EXTERN_C
jnc_TypeKind
jnc_Type_getTypeKind(jnc_Type* type);

JNC_INLINE
uint_t
jnc_Type_getTypeKindFlags(jnc_Type* type)
{
	jnc_TypeKind typeKind = jnc_Type_getTypeKind(type);
	return jnc_getTypeKindFlags(typeKind);
}

JNC_EXTERN_C
size_t
jnc_Type_getSize(jnc_Type* type);

JNC_EXTERN_C
const char*
jnc_Type_getTypeString(jnc_Type* type);

JNC_EXTERN_C
const char*
jnc_Type_getTypeStringPrefix(jnc_Type* type);

JNC_EXTERN_C
const char*
jnc_Type_getTypeStringSuffix(jnc_Type* type);

JNC_EXTERN_C
int
jnc_Type_cmp(
	jnc_Type* type,
	jnc_Type* type2
	);

JNC_EXTERN_C
jnc_DataPtrType*
jnc_Type_getDataPtrType(
	jnc_Type* type,
	jnc_DataPtrTypeKind ptrTypeKind,
	uint_t flags
	);

JNC_EXTERN_C
void
jnc_Type_markGcRoots(
	jnc_Type* type,
	const void* p,
	jnc_GcHeap* gcHeap
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Type: jnc_ModuleItem
{
	jnc_TypeKind
	getTypeKind()
	{
		return jnc_Type_getTypeKind(this);
	}

	uint_t
	getTypeKindFlags()
	{
		return jnc_Type_getTypeKindFlags(this);
	}

	size_t
	getSize()
	{
		return jnc_Type_getSize(this);
	}

	const char*
	getTypeString()
	{
		return jnc_Type_getTypeString(this);
	}

	const char*
	getTypeStringPrefix()
	{
		return jnc_Type_getTypeStringPrefix(this);
	}

	const char*
	getTypeStringSuffix()
	{
		return jnc_Type_getTypeStringSuffix(this);
	}

	int
	cmp(jnc_Type* type)
	{
		return jnc_Type_cmp(this, type);
	}

	jnc_DataPtrType*
	getDataPtrType(
		jnc_DataPtrTypeKind ptrTypeKind = jnc_DataPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return jnc_Type_getDataPtrType(this, ptrTypeKind, flags);
	}

	void
	markGcRoots(
		const void* p,
		jnc_GcHeap* gcHeap
		)
	{
		jnc_Type_markGcRoots(this, p, gcHeap);
	}
};

#endif // _JNC_CORE

//..............................................................................

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_NamedType: jnc_Type
{
};

#endif // _JNC_CORE

//..............................................................................

JNC_EXTERN_C
jnc_DataPtrTypeKind
jnc_DataPtrType_getPtrTypeKind(jnc_DataPtrType* type);

JNC_EXTERN_C
jnc_Type*
jnc_DataPtrType_getTargetType(jnc_DataPtrType* type);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_DataPtrType: jnc_Type
{
	jnc_DataPtrTypeKind
	getPtrTypeKind()
	{
		return jnc_DataPtrType_getPtrTypeKind(this);
	}

	jnc_Type*
	getTargetType()
	{
		return jnc_DataPtrType_getTargetType(this);
	}
};

#endif // _JNC_CORE

//..............................................................................

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Typedef: jnc_ModuleItem
{
};

#endif // _JNC_CORE

//..............................................................................

JNC_INLINE
bool_t
jnc_isCharPtrType(jnc_Type* type)
{
	return
		jnc_Type_getTypeKind(type) == jnc_TypeKind_DataPtr &&
		jnc_Type_getTypeKind(jnc_DataPtrType_getTargetType((jnc_DataPtrType*)type)) == jnc_TypeKind_Char;
}

JNC_INLINE
bool_t
jnc_isArrayRefType(jnc_Type* type)
{
	return
		jnc_Type_getTypeKind(type) == jnc_TypeKind_DataRef &&
		jnc_Type_getTypeKind(jnc_DataPtrType_getTargetType((jnc_DataPtrType*)type)) == jnc_TypeKind_Array;
}

JNC_INLINE
bool_t
jnc_isDataPtrType(
	jnc_Type* type,
	jnc_DataPtrTypeKind kind
	)
{
	return
		(jnc_Type_getTypeKindFlags(type) & jnc_TypeKindFlag_DataPtr) &&
		jnc_DataPtrType_getPtrTypeKind(((jnc_DataPtrType*)type)) == kind;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

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
	TypeKind_TypedefShadow       = jnc_TypeKind_TypedefShadow,
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
	TypeKind_Long                = jnc_TypeKind_Long,
	TypeKind_Long_u              = jnc_TypeKind_Long_u,
	TypeKind_DWord               = jnc_TypeKind_Int32_u,
	TypeKind_QWord               = jnc_TypeKind_Int64_u;

//..............................................................................

typedef jnc_TypeKindFlag TypeKindFlag;

const TypeKindFlag
	TypeKindFlag_Integer      = jnc_TypeKindFlag_Integer,
	TypeKindFlag_Unsigned     = jnc_TypeKindFlag_Unsigned,
	TypeKindFlag_BigEndian    = jnc_TypeKindFlag_BigEndian,
	TypeKindFlag_Fp           = jnc_TypeKindFlag_Fp,
	TypeKindFlag_Numeric      = jnc_TypeKindFlag_Numeric,
	TypeKindFlag_Aggregate    = jnc_TypeKindFlag_Aggregate,
	TypeKindFlag_Named        = jnc_TypeKindFlag_Named,
	TypeKindFlag_Derivable    = jnc_TypeKindFlag_Derivable,
	TypeKindFlag_DataPtr      = jnc_TypeKindFlag_DataPtr,
	TypeKindFlag_ClassPtr     = jnc_TypeKindFlag_ClassPtr,
	TypeKindFlag_FunctionPtr  = jnc_TypeKindFlag_FunctionPtr,
	TypeKindFlag_PropertyPtr  = jnc_TypeKindFlag_PropertyPtr,
	TypeKindFlag_Ptr          = jnc_TypeKindFlag_Ptr,
	TypeKindFlag_Ref          = jnc_TypeKindFlag_Ref,
	TypeKindFlag_Import       = jnc_TypeKindFlag_Import,
	TypeKindFlag_Code         = jnc_TypeKindFlag_Code,
	TypeKindFlag_Nullable     = jnc_TypeKindFlag_Nullable,
	TypeKindFlag_ErrorCode    = jnc_TypeKindFlag_ErrorCode;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
uint_t
getTypeKindFlags(TypeKind typeKind)
{
	return jnc_getTypeKindFlags(typeKind);
}

//..............................................................................

typedef jnc_TypeFlag TypeFlag;

const TypeFlag
	TypeFlag_Named     = jnc_TypeFlag_Named,
	TypeFlag_Pod       = jnc_TypeFlag_Pod,
	TypeFlag_GcRoot    = jnc_TypeFlag_GcRoot,
	TypeFlag_StructRet = jnc_TypeFlag_StructRet,
	TypeFlag_NoStack   = jnc_TypeFlag_NoStack,
	TypeFlag_Dynamic   = jnc_TypeFlag_Dynamic;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef jnc_PtrTypeFlag PtrTypeFlag;

const PtrTypeFlag
	PtrTypeFlag_Safe       = jnc_PtrTypeFlag_Safe,
	PtrTypeFlag_Const      = jnc_PtrTypeFlag_Const,
	PtrTypeFlag_ReadOnly   = jnc_PtrTypeFlag_ReadOnly,
	PtrTypeFlag_CMut       = jnc_PtrTypeFlag_CMut,
	PtrTypeFlag_Volatile   = jnc_PtrTypeFlag_Volatile,
	PtrTypeFlag_Event      = jnc_PtrTypeFlag_Event,
	PtrTypeFlag_DualEvent  = jnc_PtrTypeFlag_DualEvent,
	PtrTypeFlag_Bindable   = jnc_PtrTypeFlag_Bindable,
	PtrTypeFlag_AutoGet    = jnc_PtrTypeFlag_AutoGet,
	PtrTypeFlag_DualTarget = jnc_PtrTypeFlag_DualTarget,

	PtrTypeFlag__Dual      = jnc_PtrTypeFlag__Dual;

//..............................................................................

typedef jnc_StdType StdType;

const StdType
	StdType_BytePtr             = jnc_StdType_BytePtr,
	StdType_CharConstPtr        = jnc_StdType_CharConstPtr,
	StdType_IfaceHdr            = jnc_StdType_IfaceHdr,
	StdType_IfaceHdrPtr         = jnc_StdType_IfaceHdrPtr,
	StdType_Box                 = jnc_StdType_Box,
	StdType_BoxPtr              = jnc_StdType_BoxPtr,
	StdType_DataBox             = jnc_StdType_DataBox,
	StdType_DataBoxPtr          = jnc_StdType_DataBoxPtr,
	StdType_DetachedDataBox     = jnc_StdType_DetachedDataBox,
	StdType_DetachedDataBoxPtr  = jnc_StdType_DetachedDataBoxPtr,
	StdType_AbstractClass       = jnc_StdType_AbstractClass,
	StdType_AbstractClassPtr    = jnc_StdType_AbstractClassPtr,
	StdType_AbstractData        = jnc_StdType_AbstractData,
	StdType_AbstractDataPtr     = jnc_StdType_AbstractDataPtr,
	StdType_SimpleFunction      = jnc_StdType_SimpleFunction,
	StdType_SimpleMulticast     = jnc_StdType_SimpleMulticast,
	StdType_SimpleEventPtr      = jnc_StdType_SimpleEventPtr,
	StdType_Binder              = jnc_StdType_Binder,
	StdType_GcTriggers          = jnc_StdType_GcTriggers,
	StdType_GcStats             = jnc_StdType_GcStats,
	StdType_Scheduler           = jnc_StdType_Scheduler,
	StdType_SchedulerPtr        = jnc_StdType_SchedulerPtr,
	StdType_RegexMatch          = jnc_StdType_RegexMatch,
	StdType_RegexState          = jnc_StdType_RegexState,
	StdType_RegexDfa            = jnc_StdType_RegexDfa,
	StdType_Promise             = jnc_StdType_Promise,
	StdType_PromisePtr          = jnc_StdType_PromisePtr,
	StdType_Promisifier         = jnc_StdType_Promisifier,
	StdType_DynamicLib          = jnc_StdType_DynamicLib,
	StdType_DynamicLayout       = jnc_StdType_DynamicLayout,
	StdType_FmtLiteral          = jnc_StdType_FmtLiteral,
	StdType_Int64Int64          = jnc_StdType_Int64Int64,
	StdType_Fp64Fp64            = jnc_StdType_Fp64Fp64,
	StdType_Int64Fp64           = jnc_StdType_Int64Fp64,
	StdType_Fp64Int64           = jnc_StdType_Fp64Int64,
	StdType_DataPtrValidator    = jnc_StdType_DataPtrValidator,
	StdType_DataPtrValidatorPtr = jnc_StdType_DataPtrValidatorPtr,
	StdType_DataPtrStruct       = jnc_StdType_DataPtrStruct,
	StdType_FunctionPtrStruct   = jnc_StdType_FunctionPtrStruct,
	StdType_PropertyPtrStruct   = jnc_StdType_PropertyPtrStruct,
	StdType_VariantStruct       = jnc_StdType_VariantStruct,
	StdType_GcShadowStackFrame  = jnc_StdType_GcShadowStackFrame,
	StdType_SjljFrame           = jnc_StdType_SjljFrame,
	StdType_ReactorBase         = jnc_StdType_ReactorBase,
	StdType_ReactorClosure      = jnc_StdType_ReactorClosure,
	StdType__Count              = jnc_StdType__Count;

//..............................................................................

typedef jnc_DataPtrTypeKind DataPtrTypeKind;

const DataPtrTypeKind
	DataPtrTypeKind_Normal = jnc_DataPtrTypeKind_Normal,
	DataPtrTypeKind_Lean   = jnc_DataPtrTypeKind_Lean,
	DataPtrTypeKind_Thin   = jnc_DataPtrTypeKind_Thin,
	DataPtrTypeKind__Count = jnc_DataPtrTypeKind__Count;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
const char*
getDataPtrTypeKindString(DataPtrTypeKind ptrTypeKind)
{
	return jnc_getDataPtrTypeKindString(ptrTypeKind);
}

//..............................................................................

inline
bool
isCharPtrType(Type* type)
{
	return jnc_isCharPtrType(type) != 0;
}

inline
bool
isArrayRefType(Type* type)
{
	return jnc_isArrayRefType(type) != 0;
}

inline
bool
isDataPtrType(
	Type* type,
	DataPtrTypeKind kind
	)
{
	return jnc_isDataPtrType(type, kind) != 0;
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
