#pragma once

#define _JNC_CLASSTYPE_H

#include "jnc_DerivableType.h"
#include "jnc_FunctionType.h"

//.............................................................................

enum jnc_ClassTypeKind
{
	jnc_ClassTypeKind_Normal = 0,
	jnc_ClassTypeKind_Abstract, // class*
	jnc_ClassTypeKind_Multicast,
	jnc_ClassTypeKind_McSnapshot,
	jnc_ClassTypeKind_Reactor,
	jnc_ClassTypeKind_ReactorIface,
	jnc_ClassTypeKind_FunctionClosure,
	jnc_ClassTypeKind_PropertyClosure,
	jnc_ClassTypeKind_DataClosure,
};

//.............................................................................

enum jnc_ClassTypeFlag
{
	jnc_ClassTypeFlag_HasAbstractMethods = 0x010000,
	jnc_ClassTypeFlag_Closure            = 0x020000,
	jnc_ClassTypeFlag_Opaque             = 0x100000,
	jnc_ClassTypeFlag_OpaqueNonCreatable = 0x200000,
};

//.............................................................................

enum jnc_ClassPtrTypeKind
{
	jnc_ClassPtrTypeKind_Normal = 0,
	jnc_ClassPtrTypeKind_Weak,
	jnc_ClassPtrTypeKind__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
const char*
jnc_getClassPtrTypeKindString (jnc_ClassPtrTypeKind ptrTypeKind);

//.............................................................................

JNC_EXTERN_C
jnc_ClassTypeKind
jnc_ClassType_getClassTypeKind (jnc_ClassType* type);

JNC_EXTERN_C
jnc_StructType*
jnc_ClassType_getIfaceStructType (jnc_ClassType* type);

JNC_EXTERN_C
jnc_ClassPtrType*
jnc_ClassType_getClassPtrType (
	jnc_ClassType* type,
	jnc_ClassPtrTypeKind ptrTypeKind,
	uint_t flags
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_ClassType: jnc_DerivableType
{
	jnc_ClassTypeKind
	getClassTypeKind ()
	{
		return jnc_ClassType_getClassTypeKind (this);
	}

	jnc_StructType*
	getIfaceStructType ()
	{
		return jnc_ClassType_getIfaceStructType (this);
	}

	jnc_ClassPtrType*
	getClassPtrType (
		jnc_ClassPtrTypeKind ptrTypeKind = jnc_ClassPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return jnc_ClassType_getClassPtrType (this, ptrTypeKind, flags);
	}
};

#endif // _JNC_CORE

//.............................................................................

enum jnc_MulticastFieldKind
{
	jnc_MulticastFieldKind_Lock,
	jnc_MulticastFieldKind_PtrArray,
	jnc_MulticastFieldKind_Count,
	jnc_MulticastFieldKind_MaxCount,
	jnc_MulticastFieldKind_HandleTable,

	jnc_MulticastFieldKind__Count,
};

enum jnc_MulticastMethodKind
{
	jnc_MulticastMethodKind_Clear,
	jnc_MulticastMethodKind_Setup,
	jnc_MulticastMethodKind_Add,
	jnc_MulticastMethodKind_Remove,
	jnc_MulticastMethodKind_GetSnapshot,
	jnc_MulticastMethodKind_Call,
	jnc_MulticastMethodKind__Count,
};

enum jnc_MulticastMethodFlag
{
	jnc_MulticastMethodFlag_InaccessibleViaEventPtr = 0x010000,
};

//.............................................................................

JNC_EXTERN_C
jnc_FunctionPtrType*
jnc_MulticastClassType_getTargetType (jnc_MulticastClassType* type);

inline
jnc_FunctionType*
jnc_MulticastClassType_getFunctionType (jnc_MulticastClassType* type)
{
	return jnc_FunctionPtrType_getTargetType (jnc_MulticastClassType_getTargetType (type));
}

JNC_EXTERN_C
jnc_Function*
jnc_MulticastClassType_getMethod (
	jnc_MulticastClassType* type,
	jnc_MulticastMethodKind method
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_MulticastClassType: jnc_ClassType
{
	jnc_FunctionPtrType*
	getTargetType ()
	{
		return jnc_MulticastClassType_getTargetType (this);
	}

	jnc_FunctionType*
	getFunctionType ()
	{
		return jnc_MulticastClassType_getFunctionType (this);
	}

	jnc_Function*
	getMethod (jnc_MulticastMethodKind method)
	{
		return jnc_MulticastClassType_getMethod (this, method);
	}
};

#endif // _JNC_CORE

//.............................................................................

enum jnc_McSnapshotFieldKind
{
	jnc_McSnapshotFieldKind_PtrArray,
	jnc_McSnapshotFieldKind_Count,

	jnc_McSnapshotFieldKind__Count,
};

enum jnc_McSnapshotMethodKind
{
	jnc_McSnapshotMethodKind_Call,

	jnc_McSnapshotMethodKind__Count,
};

//.............................................................................

JNC_EXTERN_C
jnc_FunctionPtrType*
jnc_McSnapshotClassType_getTargetType (jnc_McSnapshotClassType* type);

inline
jnc_FunctionType*
jnc_McSnapshotClassType_getFunctionType (jnc_McSnapshotClassType* type)
{
	return jnc_FunctionPtrType_getTargetType (jnc_McSnapshotClassType_getTargetType (type));
}

JNC_EXTERN_C
jnc_Function*
jnc_McSnapshotClassType_getMethod (
	jnc_McSnapshotClassType* type,
	jnc_McSnapshotMethodKind method
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_McSnapshotClassType: jnc_ClassType
{
	jnc_FunctionPtrType*
	getTargetType ()
	{
		return jnc_McSnapshotClassType_getTargetType (this);
	}

	jnc_FunctionType*
	getFunctionType ()
	{
		return jnc_McSnapshotClassType_getFunctionType (this);
	}

	jnc_Function*
	getMethod (jnc_McSnapshotMethodKind method)
	{
		return jnc_McSnapshotClassType_getMethod (this, method);
	}
};

#endif // _JNC_CORE

//.............................................................................

inline
int
jnc_isClassType (
	jnc_Type* type,
	jnc_ClassTypeKind classTypeKind
	)
{
	return
		jnc_Type_getTypeKind (type) == jnc_TypeKind_Class &&
		jnc_ClassType_getClassTypeKind ((jnc_ClassType*) type) == classTypeKind;
}

inline
int
jnc_isOpaqueClassType (jnc_Type* type)
{
	return
		jnc_Type_getTypeKind (type) == jnc_TypeKind_Class &&
		(jnc_ModuleItem_getFlags ((jnc_ModuleItem*) type) & jnc_ClassTypeFlag_Opaque);
}

inline
int
jnc_isClosureClassType (jnc_Type* type)
{
	return
		jnc_Type_getTypeKind (type) == jnc_TypeKind_Class &&
		(jnc_ModuleItem_getFlags ((jnc_ModuleItem*) type) & jnc_ClassTypeFlag_Closure);
}

inline
int
jnc_isDestructibleClassType (jnc_Type* type)
{
	return
		jnc_Type_getTypeKind (type) == jnc_TypeKind_Class &&
		jnc_DerivableType_getDestructor ((jnc_DerivableType*) type) != NULL;
}

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

typedef jnc_ClassTypeKind ClassTypeKind;

const ClassTypeKind
	ClassTypeKind_Normal          = jnc_ClassTypeKind_Normal,
	ClassTypeKind_Abstract        = jnc_ClassTypeKind_Abstract,
	ClassTypeKind_Multicast       = jnc_ClassTypeKind_Multicast,
	ClassTypeKind_McSnapshot      = jnc_ClassTypeKind_McSnapshot,
	ClassTypeKind_Reactor         = jnc_ClassTypeKind_Reactor,
	ClassTypeKind_ReactorIface    = jnc_ClassTypeKind_ReactorIface,
	ClassTypeKind_FunctionClosure = jnc_ClassTypeKind_FunctionClosure,
	ClassTypeKind_PropertyClosure = jnc_ClassTypeKind_PropertyClosure,
	ClassTypeKind_DataClosure     = jnc_ClassTypeKind_DataClosure;

//.............................................................................

typedef jnc_ClassTypeFlag ClassTypeFlag;

const ClassTypeFlag
	ClassTypeFlag_HasAbstractMethods = jnc_ClassTypeFlag_HasAbstractMethods,
	ClassTypeFlag_Closure            = jnc_ClassTypeFlag_Closure,
	ClassTypeFlag_Opaque             = jnc_ClassTypeFlag_Opaque,
	ClassTypeFlag_OpaqueNonCreatable = jnc_ClassTypeFlag_OpaqueNonCreatable;

//.............................................................................

typedef jnc_ClassPtrTypeKind ClassPtrTypeKind;

const ClassPtrTypeKind
	ClassPtrTypeKind_Normal  = jnc_ClassPtrTypeKind_Normal,
	ClassPtrTypeKind_Weak    = jnc_ClassPtrTypeKind_Weak,
	ClassPtrTypeKind__Count  = jnc_ClassPtrTypeKind__Count;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
const char*
getClassPtrTypeKindString (ClassPtrTypeKind ptrTypeKind)
{
	return jnc_getClassPtrTypeKindString (ptrTypeKind);
}

//.............................................................................

typedef jnc_MulticastFieldKind MulticastFieldKind;
typedef jnc_MulticastMethodKind MulticastMethodKind;
typedef jnc_MulticastMethodFlag MulticastMethodFlag;
	
const MulticastFieldKind
	MulticastFieldKind_Lock        = jnc_MulticastFieldKind_Lock,
	MulticastFieldKind_PtrArray    = jnc_MulticastFieldKind_PtrArray,
	MulticastFieldKind_Count       = jnc_MulticastFieldKind_Count,
	MulticastFieldKind_MaxCount    = jnc_MulticastFieldKind_MaxCount,
	MulticastFieldKind_HandleTable = jnc_MulticastFieldKind_HandleTable,
	MulticastFieldKind__Count      = jnc_MulticastFieldKind__Count;

const MulticastMethodKind
	MulticastMethodKind_Clear       = jnc_MulticastMethodKind_Clear,
	MulticastMethodKind_Setup       = jnc_MulticastMethodKind_Setup,
	MulticastMethodKind_Add         = jnc_MulticastMethodKind_Add,
	MulticastMethodKind_Remove      = jnc_MulticastMethodKind_Remove,
	MulticastMethodKind_GetSnapshot = jnc_MulticastMethodKind_GetSnapshot,
	MulticastMethodKind_Call        = jnc_MulticastMethodKind_Call,
	MulticastMethodKind__Count      = jnc_MulticastMethodKind__Count;

const MulticastMethodFlag
	MulticastMethodFlag_InaccessibleViaEventPtr = jnc_MulticastMethodFlag_InaccessibleViaEventPtr;

//.............................................................................

typedef jnc_McSnapshotFieldKind McSnapshotFieldKind;
typedef jnc_McSnapshotMethodKind McSnapshotMethodKind;

const McSnapshotFieldKind
	McSnapshotFieldKind_PtrArray = jnc_McSnapshotFieldKind_PtrArray,
	McSnapshotFieldKind_Count    = jnc_McSnapshotFieldKind_Count,
	McSnapshotFieldKind__Count   = jnc_McSnapshotFieldKind__Count;

const McSnapshotMethodKind
	McSnapshotMethodKind_Call   = jnc_McSnapshotMethodKind_Call,
	McSnapshotMethodKind__Count = jnc_McSnapshotMethodKind__Count;

//.............................................................................

inline
bool
isClassType (
	Type* type,
	ClassTypeKind classTypeKind
	)
{
	return jnc_isClassType (type, classTypeKind) != 0;
}

inline
bool
isOpaqueClassType (Type* type)
{
	return jnc_isOpaqueClassType (type) != 0;
}

inline
bool
isClosureClassType (Type* type)
{
	return jnc_isClosureClassType (type) != 0;
}

inline
bool
isDestructibleClassType (Type* type)
{
	return jnc_isDestructibleClassType (type) != 0;
}

//.............................................................................

} // namespace jnc

#endif // __cplusplus
