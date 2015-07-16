// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_StdNamespace.h"

namespace jnc {

//.............................................................................

enum StdType
{
	StdType_BytePtr,
	StdType_ByteConstPtr,
	StdType_SimpleIfaceHdr,
	StdType_SimpleIfaceHdrPtr,
	StdType_Box,
	StdType_BoxPtr,
	StdType_DataBox,
	StdType_DataBoxPtr,
	StdType_DynamicArrayBox,
	StdType_DynamicArrayBoxPtr,
	StdType_AbstractClass,
	StdType_AbstractClassPtr,
	StdType_SimpleFunction,
	StdType_SimpleMulticast,
	StdType_SimpleEventPtr,
	StdType_Binder,
	StdType_ReactorBindSite,
	StdType_Scheduler,
	StdType_Recognizer,
	StdType_AutomatonResult,
	StdType_Library,
	StdType_FmtLiteral,
	StdType_Guid,
	StdType_Error,
	StdType_String,
	StdType_StringRef,
	StdType_StringBuilder,
	StdType_StringHashTable,
	StdType_VariantHashTable,
	StdType_ListEntry,
	StdType_List,
	StdType_ConstBuffer,
	StdType_ConstBufferRef,
	StdType_BufferRef,
	StdType_Buffer,
	StdType_Int64Int64, // for system V coercion
	StdType_Fp64Fp64,   // for system V coercion
	StdType_Int64Fp64,  // for system V coercion
	StdType_Fp64Int64,  // for system V coercion
	StdType_DataPtrValidator,
	StdType_DataPtrStruct,
	StdType_FunctionPtrStruct,
	StdType_PropertyPtrStruct = StdType_FunctionPtrStruct,
	StdType_VariantStruct,
	StdType__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getStdTypeName (StdType stdType);

const StdItemSource*
getStdTypeSource (StdType stdType);

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

} // namespace jnc {