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
#include "jnc_ClassType.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_ct_ClassType.h"
#	include "jnc_ct_ClassPtrType.h"
#	include "jnc_ct_MulticastClassType.h"
#	include "jnc_ct_McSnapshotClassType.h"
#endif

//..............................................................................

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_getClassPtrTypeKindString(jnc_ClassPtrTypeKind ptrTypeKind) {
	static const char* stringTable[jnc_ClassPtrTypeKind__Count] = {
		"strong", // ClassPtrTypeKind_Normal = 0,
		"weak",   // ClassPtrTypeKind_Weak,
	};

	return (size_t)ptrTypeKind < jnc_ClassPtrTypeKind__Count ?
		stringTable[ptrTypeKind] :
		"undefined-class-ptr-kind";
}

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_ClassTypeKind
jnc_ClassType_getClassTypeKind(jnc_ClassType* type) {
	return jnc_g_dynamicExtensionLibHost->m_classTypeFuncTable->m_getClassTypeKindFunc(type);
}

JNC_EXTERN_C
jnc_StructType*
jnc_ClassType_getIfaceStructType(jnc_ClassType* type) {
	return jnc_g_dynamicExtensionLibHost->m_classTypeFuncTable->m_getIfaceStructTypeFunc(type);
}

JNC_EXTERN_C
jnc_ClassPtrType*
jnc_ClassType_getClassPtrType(
	jnc_ClassType* type,
	jnc_TypeKind typeKind,
	jnc_ClassPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	return jnc_g_dynamicExtensionLibHost->m_classTypeFuncTable->m_getClassPtrTypeFunc(type, typeKind, ptrTypeKind, flags);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
jnc_ClassPtrTypeKind
jnc_ClassPtrType_getPtrTypeKind(jnc_ClassPtrType* type) {
	return jnc_g_dynamicExtensionLibHost->m_classPtrTypeFuncTable->m_getPtrTypeKindFunc(type);
}

JNC_EXTERN_C
jnc_ClassType*
jnc_ClassPtrType_getTargetType(jnc_ClassPtrType* type) {
	return jnc_g_dynamicExtensionLibHost->m_classPtrTypeFuncTable->m_getTargetTypeFunc(type);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
jnc_FunctionPtrType*
jnc_MulticastClassType_getTargetType(jnc_MulticastClassType* type) {
	return jnc_g_dynamicExtensionLibHost->m_multicastClassTypeFuncTable->m_getTargetTypeFunc(type);
}

JNC_EXTERN_C
jnc_Function*
jnc_MulticastClassType_getMethod(
	jnc_MulticastClassType* type,
	jnc_MulticastMethodKind method
) {
	return jnc_g_dynamicExtensionLibHost->m_multicastClassTypeFuncTable->m_getMethodFunc(type, method);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
jnc_FunctionPtrType*
jnc_McSnapshotClassType_getTargetType(jnc_McSnapshotClassType* type) {
	return jnc_g_dynamicExtensionLibHost->m_mcSnapshotClassTypeFuncTable->m_getTargetTypeFunc(type);
}

JNC_EXTERN_C
jnc_Function*
jnc_McSnapshotClassType_getMethod(
	jnc_McSnapshotClassType* type,
	jnc_McSnapshotMethodKind method
) {
	return jnc_g_dynamicExtensionLibHost->m_mcSnapshotClassTypeFuncTable->m_getMethodFunc(type, method);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
jnc_ClassTypeKind
jnc_ClassType_getClassTypeKind(jnc_ClassType* type) {
	return type->getClassTypeKind();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_StructType*
jnc_ClassType_getIfaceStructType(jnc_ClassType* type) {
	return type->getIfaceStructType();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_ClassPtrType*
jnc_ClassType_getClassPtrType(
	jnc_ClassType* type,
	jnc_TypeKind typeKind,
	jnc_ClassPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	return type->getClassPtrType(typeKind, ptrTypeKind, flags);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
JNC_EXPORT_O
jnc_ClassPtrTypeKind
jnc_ClassPtrType_getPtrTypeKind(jnc_ClassPtrType* type) {
	return type->getPtrTypeKind();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_ClassType*
jnc_ClassPtrType_getTargetType(jnc_ClassPtrType* type) {
	return type->getTargetType();
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
JNC_EXPORT_O
jnc_FunctionPtrType*
jnc_MulticastClassType_getTargetType(jnc_MulticastClassType* type) {
	return type->getTargetType();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Function*
jnc_MulticastClassType_getMethod(
	jnc_MulticastClassType* type,
	jnc_MulticastMethodKind method
) {
	return type->getMethod(method);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
JNC_EXPORT_O
jnc_FunctionPtrType*
jnc_McSnapshotClassType_getTargetType(jnc_McSnapshotClassType* type) {
	return type->getTargetType();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Function*
jnc_McSnapshotClassType_getMethod(
	jnc_McSnapshotClassType* type,
	jnc_McSnapshotMethodKind method
) {
	return type->getMethod(method);
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
