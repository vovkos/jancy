#include "pch.h"
#include "jnc_StdType.h"

namespace jnc {

//.............................................................................

const char*
getStdTypeName (StdType stdType)
{
	const char* nameTable [StdType__Count] =
	{
		NULL,               // StdType_BytePtr,
		NULL,               // StdType_ByteConstPtr,
		NULL,               // StdType_SimpleIfaceHdr,
		NULL,               // StdType_SimpleIfaceHdrPtr,
		NULL,               // StdType_Box,
		NULL,               // StdType_BoxPtr,
		NULL,               // StdType_DataBox,
		NULL,               // StdType_DataBoxPtr,
		NULL,               // StdType_DynamicArrayBox,
		NULL,               // StdType_DynamicArrayBoxPtr,
		NULL,               // StdType_StaticDataBox,
		NULL,               // StdType_StaticDataBoxPtr,
		NULL,               // StdType_AbstractClass,
		NULL,               // StdType_AbstractClassPtr,
		NULL,               // StdType_SimpleFunction,
		NULL,               // StdType_SimpleMulticast,
		NULL,               // StdType_SimpleEventPtr,
		NULL,               // StdType_Binder,
		NULL,               // StdType_ReactorBindSite,
		"Scheduler",        // StdType_Scheduler,
		"Recognizer",       // StdType_Recognizer,
		"AutomatonResult",  // StdType_AutomatonResult,
		"Library",          // StdType_Library,
		NULL,               // StdType_FmtLiteral,
		"Guid",             // StdType_Guid
		"Error",            // StdType_Error,
		"String",           // StdType_String,
		"StringRef",        // StdType_StringRef,
		"StringBuilder",    // StdType_StringBuilder,
		"StringHashTable",  // StdType_StringHashTable,
		"VariantHashTable", // StdType_VariantHashTable,
		"ListEntry",        // StdType_ListEntry,
		"List",             // StdType_List,
		"ConstArray",       // StdType_ConstArray,
		"ConstArrayRef",    // StdType_ConstArrayRef,
		"ArrayRef",         // StdType_ArrayRef,
		"Array",            // StdType_DynamicArray,
	};

	ASSERT ((size_t) stdType < StdType__Count);
	return nameTable [stdType];
}

const StdItemSource*
getStdTypeSource (StdType stdType)
{
	#include "jnc_StdTypes.jnc.cpp"
	#include "jnc_Buffer.jnc.cpp"
	#include "jnc_String.jnc.cpp"
	#include "jnc_HashTable.jnc.cpp"
	#include "jnc_List.jnc.cpp"
	#include "jnc_Recognizer.jnc.cpp"
	#include "jnc_Library.jnc.cpp"

	static StdItemSource sourceTable [StdType__Count] =
	{
		{ NULL },                            // StdType_BytePtr,
		{ NULL },                            // StdType_ByteConstPtr,
		{ NULL },                            // StdType_SimpleIfaceHdr,
		{ NULL },                            // StdType_SimpleIfaceHdrPtr,
		{ NULL },                            // StdType_Box,
		{ NULL },                            // StdType_BoxPtr,
		{ NULL },                            // StdType_DataBox,
		{ NULL },                            // StdType_DataBoxPtr,
		{ NULL },                            // StdType_DynamicArrayBox,
		{ NULL },                            // StdType_DynamicArrayBoxPtr,
		{ NULL },                            // StdType_StaticDataBox,
		{ NULL },                            // StdType_StaticDataBoxPtr,
		{ NULL },                            // StdType_AbstractClass,
		{ NULL },                            // StdType_AbstractClassPtr,
		{ NULL },                            // StdType_SimpleFunction,
		{ NULL },                            // StdType_SimpleMulticast,
		{ NULL },                            // StdType_SimpleEventPtr,
		{ NULL },                            // StdType_Binder,
		{                                    // StdType_ReactorBindSite,
			reactorBindSiteTypeSrc,
			lengthof (reactorBindSiteTypeSrc),
			StdNamespace_Internal,
		},
		{                                    // StdType_Scheduler,
			schedulerTypeSrc,
			lengthof (schedulerTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_Recognizer,
			recognizerTypeSrc,
			lengthof (recognizerTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_AutomatonResult,
			automatonResultTypeSrc,
			lengthof (automatonResultTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_Library,
			libraryTypeSrc,
			lengthof (libraryTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_FmtLiteral,
			fmtLiteralTypeSrc,
			lengthof (fmtLiteralTypeSrc),
			StdNamespace_Internal,
		},
		{                                    // StdType_Guid,
			guidTypeSrc,
			lengthof (guidTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_Error,
			errorTypeSrc,
			lengthof (errorTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_String,
			stringTypeSrc,
			lengthof (stringTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_StringRef,
			stringRefTypeSrc,
			lengthof (stringRefTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_StringBuilder,
			stringBuilderTypeSrc,
			lengthof (stringBuilderTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_StringHashTable,
			stringHashTableTypeSrc,
			lengthof (stringHashTableTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_VariantHashTable,
			variantHashTableTypeSrc,
			lengthof (variantHashTableTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_ListEntry,
			listEntryTypeSrc,
			lengthof (listEntryTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_List,
			listTypeSrc,
			lengthof (listTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_ConstBuffer,
			constBufferTypeSrc,
			lengthof (constBufferTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_ConstBufferRef,
			constBufferRefTypeSrc,
			lengthof (constBufferRefTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_BufferRef,
			bufferRefTypeSrc,
			lengthof (bufferRefTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_Buffer,
			bufferTypeSrc,
			lengthof (bufferTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_Int64Int64,
			int64Int64TypeSrc,
			lengthof (int64Int64TypeSrc),
			StdNamespace_Internal,
		},
		{                                    // StdType_Fp64Fp64,
			fp64Fp64TypeSrc,
			lengthof (fp64Fp64TypeSrc),
			StdNamespace_Internal,
		},
		{                                    // StdType_Int64Fp64,
			int64Fp64TypeSrc,
			lengthof (int64Fp64TypeSrc),
			StdNamespace_Internal,
		},

		{                                    // StdType_Fp64Int64,
			fp64Int64TypeSrc,
			lengthof (fp64Int64TypeSrc),
			StdNamespace_Internal,
		},
	};

	ASSERT ((size_t) stdType < StdType__Count);
	return &sourceTable [stdType];
}

//.............................................................................

} // namespace jnc {
