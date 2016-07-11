// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_ModuleItem.h"
#include "jnc_ct_StdNamespace.h"

namespace jnc {
namespace ct {

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
	StdType_StaticDataBox,
	StdType_StaticDataBoxPtr,
	StdType_AbstractClass,
	StdType_AbstractClassPtr,
	StdType_AbstractData,
	StdType_AbstractDataPtr,
	StdType_SimpleFunction,
	StdType_SimpleMulticast,
	StdType_SimpleEventPtr,
	StdType_Binder,
	StdType_ReactorBindSite,
	StdType_Scheduler,
	StdType_Recognizer,
	StdType_AutomatonResult,
	StdType_AutomatonFunc,
	StdType_DynamicLib,
	StdType_FmtLiteral,
	StdType_Int64Int64, // for system V coercion
	StdType_Fp64Fp64,   // for system V coercion
	StdType_Int64Fp64,  // for system V coercion
	StdType_Fp64Int64,  // for system V coercion
	StdType_DataPtrValidator,
	StdType_DataPtrValidatorPtr,
	StdType_DataPtrStruct,
	StdType_FunctionPtrStruct,
	StdType_PropertyPtrStruct = StdType_FunctionPtrStruct,
	StdType_VariantStruct,
	StdType_GcShadowStackFrame,
	StdType_SjljFrame,
	StdType__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

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

} // namespace ct
} // namespace jnc
