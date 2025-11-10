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

#include "pch.h"
#include "jnc_Type.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_ct_Type.h"
#	include "jnc_ct_DataPtrType.h"
#	include "jnc_ct_Module.h"
#endif

//..............................................................................

JNC_EXTERN_C
JNC_EXPORT_O
uint_t
jnc_getTypeKindFlags(jnc_TypeKind typeKind) {
	static uint_t flagTable[jnc_TypeKind__Count] = {
		0,                              // jnc_TypeKind_Void
		jnc_TypeKindFlag_Nullable |
		jnc_TypeKindFlag_ErrorCode,     // jnc_TypeKind_Variant

		jnc_TypeKindFlag_Nullable |
		jnc_TypeKindFlag_ErrorCode,     // jnc_TypeKind_String

		jnc_TypeKindFlag_Integer |      // jnc_TypeKind_Bool
		jnc_TypeKindFlag_Numeric |
		jnc_TypeKindFlag_ErrorCode,

		jnc_TypeKindFlag_Integer |      // jnc_TypeKind_Int8
		jnc_TypeKindFlag_Signed |
		jnc_TypeKindFlag_Numeric |
		jnc_TypeKindFlag_ErrorCode,

		jnc_TypeKindFlag_Integer |      // jnc_TypeKind_Int8_u
		jnc_TypeKindFlag_Unsigned |
		jnc_TypeKindFlag_Numeric |
		jnc_TypeKindFlag_ErrorCode,

		jnc_TypeKindFlag_Integer |      // jnc_TypeKind_Int16
		jnc_TypeKindFlag_Signed |
		jnc_TypeKindFlag_Numeric |
		jnc_TypeKindFlag_ErrorCode,

		jnc_TypeKindFlag_Integer |      // jnc_TypeKind_Int16_u
		jnc_TypeKindFlag_Unsigned |
		jnc_TypeKindFlag_Numeric |
		jnc_TypeKindFlag_ErrorCode,

		jnc_TypeKindFlag_Integer |      // jnc_TypeKind_Int32
		jnc_TypeKindFlag_Signed |
		jnc_TypeKindFlag_Numeric |
		jnc_TypeKindFlag_ErrorCode,

		jnc_TypeKindFlag_Integer |      // jnc_TypeKind_Int32_u
		jnc_TypeKindFlag_Unsigned |
		jnc_TypeKindFlag_Numeric |
		jnc_TypeKindFlag_ErrorCode,

		jnc_TypeKindFlag_Integer |      // jnc_TypeKind_Int64
		jnc_TypeKindFlag_Signed |
		jnc_TypeKindFlag_Numeric |
		jnc_TypeKindFlag_ErrorCode,

		jnc_TypeKindFlag_Integer |      // jnc_TypeKind_Int64_u
		jnc_TypeKindFlag_Unsigned |
		jnc_TypeKindFlag_Numeric |
		jnc_TypeKindFlag_ErrorCode,

		jnc_TypeKindFlag_Fp |           // jnc_TypeKind_Float
		jnc_TypeKindFlag_Numeric,

		jnc_TypeKindFlag_Fp |           // jnc_TypeKind_Double
		jnc_TypeKindFlag_Numeric,

		jnc_TypeKindFlag_Aggregate |
		jnc_TypeKindFlag_Nullable,      // jnc_TypeKind_Array

		jnc_TypeKindFlag_Named |        // jnc_TypeKind_Enum
		jnc_TypeKindFlag_Integer |
		jnc_TypeKindFlag_Numeric |
		jnc_TypeKindFlag_ErrorCode,

		jnc_TypeKindFlag_Aggregate |    // jnc_TypeKind_Struct
		jnc_TypeKindFlag_Derivable |
		jnc_TypeKindFlag_Named |
		jnc_TypeKindFlag_Nullable,

		jnc_TypeKindFlag_Aggregate |    // jnc_TypeKind_Union
		jnc_TypeKindFlag_Derivable |
		jnc_TypeKindFlag_Named |
		jnc_TypeKindFlag_Nullable,

		jnc_TypeKindFlag_Aggregate |    // jnc_TypeKind_Class
		jnc_TypeKindFlag_Derivable |
		jnc_TypeKindFlag_Named,

		jnc_TypeKindFlag_Code,          // jnc_TypeKind_Function
		jnc_TypeKindFlag_Code,          // jnc_TypeKind_Property

		jnc_TypeKindFlag_DataPtr |      // jnc_TypeKind_DataPtr
		jnc_TypeKindFlag_Ptr |
		jnc_TypeKindFlag_Nullable |
		jnc_TypeKindFlag_ErrorCode,

		jnc_TypeKindFlag_DataPtr |      // jnc_TypeKind_DataRef
		jnc_TypeKindFlag_Ptr |
		jnc_TypeKindFlag_Ref,

		jnc_TypeKindFlag_ClassPtr |     // jnc_TypeKind_ClassPtr
		jnc_TypeKindFlag_Ptr |
		jnc_TypeKindFlag_Nullable |
		jnc_TypeKindFlag_ErrorCode,

		jnc_TypeKindFlag_ClassPtr |     // jnc_TypeKind_ClassRef
		jnc_TypeKindFlag_Ptr |
		jnc_TypeKindFlag_Ref,

		jnc_TypeKindFlag_FunctionPtr |  // jnc_TypeKind_FunctionPtr
		jnc_TypeKindFlag_Ptr |
		jnc_TypeKindFlag_Nullable |
		jnc_TypeKindFlag_ErrorCode,

		jnc_TypeKindFlag_FunctionPtr |  // jnc_TypeKind_FunctionRef
		jnc_TypeKindFlag_Ptr |
		jnc_TypeKindFlag_Ref,

		jnc_TypeKindFlag_PropertyPtr |  // jnc_TypeKind_PropertyPtr
		jnc_TypeKindFlag_Ptr |
		jnc_TypeKindFlag_Nullable |
		jnc_TypeKindFlag_ErrorCode,

		jnc_TypeKindFlag_PropertyPtr |  // jnc_TypeKind_PropertyRef
		jnc_TypeKindFlag_Ptr |
		jnc_TypeKindFlag_Ref,

		jnc_TypeKindFlag_Import,        // jnc_TypeKind_NamedImport

		jnc_TypeKindFlag_Import |       // jnc_TypeKind_ImportPtr
		jnc_TypeKindFlag_Ptr |
		jnc_TypeKindFlag_Nullable |
		jnc_TypeKindFlag_ErrorCode,

		jnc_TypeKindFlag_Import |       // jnc_TypeKind_ImportIntMod
		jnc_TypeKindFlag_Integer |
		jnc_TypeKindFlag_Numeric |
		jnc_TypeKindFlag_ErrorCode,

		jnc_TypeKindFlag_ErrorCode,     // jnc_TypeKind_TypedefShadow
		// ^ don't fail due to non-error-code during doc-generation & code-assist

		jnc_TypeKindFlag_Template,     // jnc_TypeKind_TemplateArg

		jnc_TypeKindFlag_Template |    // jnc_TypeKind_TemplatePtr
		jnc_TypeKindFlag_Ptr |
		jnc_TypeKindFlag_Nullable |
		jnc_TypeKindFlag_ErrorCode,

		jnc_TypeKindFlag_Template |    // jnc_TypeKind_TemplateIntMod
		jnc_TypeKindFlag_Integer |
		jnc_TypeKindFlag_Numeric |
		jnc_TypeKindFlag_ErrorCode,

		jnc_TypeKindFlag_Template,     // jnc_TypeKind_TemplateDecl
	};

	return typeKind < jnc_TypeKind__Count ? flagTable[typeKind] : 0;
}

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_getDataPtrTypeKindString(jnc_DataPtrTypeKind ptrTypeKind) {
	static const char* stringTable[jnc_DataPtrTypeKind__Count] = {
		"normal", // DataPtrTypeKind_Normal = 0,
		"lean",   // DataPtrTypeKind_Lean,
		"thin",   // DataPtrTypeKind_Thin,
	};

	return (size_t)ptrTypeKind < jnc_DataPtrTypeKind__Count ?
		stringTable[ptrTypeKind] :
		"undefined-data-ptr-kind";
}

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_TypeKind
jnc_Type_getTypeKind(jnc_Type* type) {
	return jnc_g_dynamicExtensionLibHost->m_typeFuncTable->m_getTypeKindFunc(type);
}

JNC_EXTERN_C
size_t
jnc_Type_getSize(jnc_Type* type) {
	return jnc_g_dynamicExtensionLibHost->m_typeFuncTable->m_getSizeFunc(type);
}

JNC_EXTERN_C
const char*
jnc_Type_getTypeString(jnc_Type* type) {
	return jnc_g_dynamicExtensionLibHost->m_typeFuncTable->m_getTypeStringFunc(type);
}

JNC_EXTERN_C
bool_t
jnc_Type_isEqual(
	jnc_Type* type,
	jnc_Type* type2
) {
	return jnc_g_dynamicExtensionLibHost->m_typeFuncTable->m_isEqualFunc(type, type2);
}

JNC_EXTERN_C
jnc_ArrayType*
jnc_Type_getArrayType(
	jnc_Type* type,
	size_t elementCount
) {
	return jnc_g_dynamicExtensionLibHost->m_typeFuncTable->m_getArrayTypeFunc(type, elementCount);
}

JNC_EXTERN_C
jnc_DataPtrType*
jnc_Type_getBitFieldDataPtrType(
	jnc_Type* type,
	uint_t bitOffset,
	uint_t bitCount,
	jnc_TypeKind typeKind,
	jnc_DataPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	return jnc_g_dynamicExtensionLibHost->m_typeFuncTable->m_getBitFieldDataPtrTypeFunc(type, bitOffset, bitCount, typeKind, ptrTypeKind, flags);
}

JNC_EXTERN_C
jnc_DataPtrType*
jnc_Type_getDataPtrType(
	jnc_Type* type,
	jnc_TypeKind typeKind,
	jnc_DataPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	return jnc_g_dynamicExtensionLibHost->m_typeFuncTable->m_getDataPtrTypeFunc(type, typeKind, ptrTypeKind, flags);
}

JNC_EXTERN_C
bool_t
jnc_Type_ensureLayout(jnc_Type* type) {
	return jnc_g_dynamicExtensionLibHost->m_typeFuncTable->m_ensureLayoutFunc(type);
}

JNC_EXTERN_C
bool_t
jnc_Type_ensureNoImports(jnc_Type* type) {
	return jnc_g_dynamicExtensionLibHost->m_typeFuncTable->m_ensureNoImportsFunc(type);
}

JNC_EXTERN_C
void
jnc_Type_markGcRoots(
	jnc_Type* type,
	const void* p,
	jnc_GcHeap* gcHeap
) {
	return jnc_g_dynamicExtensionLibHost->m_typeFuncTable->m_markGcRootsFunc(type, p, gcHeap);
}

JNC_EXTERN_C
jnc_DataPtrTypeKind
jnc_DataPtrType_getPtrTypeKind(jnc_DataPtrType* type) {
	return jnc_g_dynamicExtensionLibHost->m_dataPtrTypeFuncTable->m_getPtrTypeKindFunc(type);
}

JNC_EXTERN_C
jnc_Type*
jnc_DataPtrType_getTargetType(jnc_DataPtrType* type) {
	return jnc_g_dynamicExtensionLibHost->m_dataPtrTypeFuncTable->m_getTargetTypeFunc(type);
}

JNC_EXTERN_C
uint_t
jnc_DataPtrType_getBitOffset(jnc_DataPtrType* type) {
	return jnc_g_dynamicExtensionLibHost->m_dataPtrTypeFuncTable->m_getBitOffsetFunc(type);
}

JNC_EXTERN_C
uint_t
jnc_DataPtrType_getBitCount(jnc_DataPtrType* type) {
	return jnc_g_dynamicExtensionLibHost->m_dataPtrTypeFuncTable->m_getBitCountFunc(type);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_getPtrTypeFlagString_v(uint_t flags) {
	return *jnc::getTlsStringBuffer() = jnc::ct::getPtrTypeFlagString(flags);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_TypeKind
jnc_Type_getTypeKind(jnc_Type* type) {
	return type->getTypeKind();
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_Type_getSize(jnc_Type* type) {
	return type->getSize();
}

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_Type_getTypeString(jnc_Type* type) {
	return type->getTypeString().sz();
}

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_Type_getTypeStringPrefix(jnc_Type* type) {
	return type->getTypeStringPrefix().sz();
}

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_Type_getTypeStringSuffix(jnc_Type* type) {
	return type->getTypeStringSuffix().sz();
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Type_isEqual(
	jnc_Type* type,
	jnc_Type* type2
) {
	return type->isEqual(type2);
}

JNC_EXTERN_C
jnc_ArrayType*
jnc_Type_getArrayType(
	jnc_Type* type,
	size_t elementCount
) {
	return type->getArrayType(elementCount);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DataPtrType*
jnc_Type_getBitFieldDataPtrType(
	jnc_Type* type,
	uint_t bitOffset,
	uint_t bitCount,
	jnc_TypeKind typeKind,
	jnc_DataPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	return type->getDataPtrType(bitOffset, bitCount, typeKind, ptrTypeKind, flags);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DataPtrType*
jnc_Type_getDataPtrType(
	jnc_Type* type,
	jnc_TypeKind typeKind,
	jnc_DataPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	return type->getDataPtrType(typeKind, ptrTypeKind, flags);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Type_ensureLayout(jnc_Type* type) {
	return type->ensureLayout();
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Type_ensureNoImports(jnc_Type* type) {
	return type->ensureNoImports();
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Type_markGcRoots(
	jnc_Type* type,
	const void* p,
	jnc_GcHeap* gcHeap
) {
	return type->markGcRoots(p, gcHeap);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DataPtrTypeKind
jnc_DataPtrType_getPtrTypeKind(jnc_DataPtrType* type) {
	return type->getPtrTypeKind();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Type*
jnc_DataPtrType_getTargetType(jnc_DataPtrType* type) {
	return type->getTargetType();
}

JNC_EXTERN_C
JNC_EXPORT_O
uint_t
jnc_DataPtrType_getBitOffset(jnc_DataPtrType* type) {
	return type->getBitOffset();
}

JNC_EXTERN_C
JNC_EXPORT_O
uint_t
jnc_DataPtrType_getBitCount(jnc_DataPtrType* type) {
	return type->getBitCount();
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
