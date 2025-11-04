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

#define _JNC_DERIVABLETYPE_H

#include "jnc_Function.h"
#include "jnc_OpKind.h"

/**

\defgroup derivable-type Derivable Type
	\ingroup type-subsystem
	\import{jnc_DerivableType.h}

	Derivable type is the base type for structs, unions and classes.

\addtogroup derivable-type
@{

\struct jnc_DerivableType
	\verbatim

	Opaque structure used as a handle to Jancy derivable type.

	Use functions from the `Derivable Type` group to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

enum jnc_FieldFlag {
	jnc_FieldFlag_LayoutReady = jnc_TypeFlag_LayoutReady, // so that a typo doesn't break things
};

typedef enum jnc_FieldFlag jnc_FieldFlag;

//..............................................................................

JNC_INLINE
jnc_DerivableType*
jnc_BaseTypeSlot_getType(jnc_BaseTypeSlot* baseType) {
	return (jnc_DerivableType*)jnc_ModuleItem_getType((jnc_ModuleItem*)baseType);
}

JNC_EXTERN_C
size_t
jnc_BaseTypeSlot_getOffset(jnc_BaseTypeSlot* baseType);

JNC_EXTERN_C
size_t
jnc_BaseTypeSlot_getVtableIndex(jnc_BaseTypeSlot* baseType);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_BaseTypeSlot: jnc_ModuleItem {
	jnc_DerivableType*
	getType() {
		return jnc_BaseTypeSlot_getType(this);
	}

	size_t
	getOffset() {
		return jnc_BaseTypeSlot_getOffset(this);
	}

	size_t
	getVtableIndex() {
		return jnc_BaseTypeSlot_getVtableIndex(this);
	}
};

#endif // _JNC_CORE

//..............................................................................

JNC_EXTERN_C
jnc_Type*
jnc_Field_getType(jnc_Field* field);

JNC_EXTERN_C
uint_t
jnc_Field_getBitOffset(jnc_Field* field);

JNC_EXTERN_C
uint_t
jnc_Field_getBitCount(jnc_Field* field);

JNC_EXTERN_C
uint_t
jnc_Field_getPtrTypeFlags(jnc_Field* field);

JNC_EXTERN_C
size_t
jnc_Field_getOffset(jnc_Field* field);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Field: jnc_ModuleItem {
	jnc_Type*
	getType() {
		return jnc_Field_getType(this);
	}

	uint_t
	getBitOffset() {
		return jnc_Field_getBitOffset(this);
	}

	uint_t
	getBitCount() {
		return jnc_Field_getBitCount(this);
	}

	uint_t
	getPtrTypeFlags() {
		return jnc_Field_getPtrTypeFlags(this);
	}

	size_t
	getOffset() {
		return jnc_Field_getOffset(this);
	}
};

#endif // _JNC_CORE

//..............................................................................

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getStaticConstructor(jnc_DerivableType* type);

JNC_EXTERN_C
jnc_OverloadableFunction
jnc_DerivableType_getConstructor(jnc_DerivableType* type);

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getDestructor(jnc_DerivableType* type);

JNC_EXTERN_C
jnc_OverloadableFunction
jnc_DerivableType_getUnaryOperator(
	jnc_DerivableType* type,
	jnc_UnOpKind opKind
);

JNC_EXTERN_C
jnc_OverloadableFunction
jnc_DerivableType_getBinaryOperator(
	jnc_DerivableType* type,
	jnc_BinOpKind opKind
);

JNC_EXTERN_C
jnc_OverloadableFunction
jnc_DerivableType_getCallOperator(jnc_DerivableType* type);

JNC_EXTERN_C
size_t
jnc_DerivableType_getCastOperatorCount(jnc_DerivableType* type);

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getCastOperator(
	jnc_DerivableType* type,
	size_t index
);

JNC_EXTERN_C
size_t
jnc_DerivableType_getBaseTypeCount(jnc_DerivableType* type);

JNC_EXTERN_C
jnc_BaseTypeSlot*
jnc_DerivableType_getBaseType(
	jnc_DerivableType* type,
	size_t index
);

JNC_EXTERN_C
size_t
jnc_DerivableType_findBaseTypeOffset(
	jnc_DerivableType* type,
	jnc_Type* baseType
);

JNC_EXTERN_C
size_t
jnc_DerivableType_getStaticVariableCount(jnc_DerivableType* type);

JNC_EXTERN_C
jnc_Variable*
jnc_DerivableType_getStaticVariable(
	jnc_DerivableType* type,
	size_t index
);

JNC_EXTERN_C
size_t
jnc_DerivableType_getFieldCount(jnc_DerivableType* type);

JNC_EXTERN_C
jnc_Field*
jnc_DerivableType_getField(
	jnc_DerivableType* type,
	size_t index
);

JNC_EXTERN_C
size_t
jnc_DerivableType_getMethodCount(jnc_DerivableType* type);

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getMethod(
	jnc_DerivableType* type,
	size_t index
);

JNC_EXTERN_C
size_t
jnc_DerivableType_getPropertyCount(jnc_DerivableType* type);

JNC_EXTERN_C
jnc_Property*
jnc_DerivableType_getProperty(
	jnc_DerivableType* type,
	size_t index
);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_DerivableType: jnc_NamedType {
	jnc_Function*
	getStaticConstructor() {
		return jnc_DerivableType_getStaticConstructor(this);
	}

	jnc::OverloadableFunction
	getConstructor() {
		return jnc_DerivableType_getConstructor(this);
	}

	jnc_Function*
	getDestructor() {
		return jnc_DerivableType_getDestructor(this);
	}

	jnc::OverloadableFunction
	getUnaryOperator(jnc_UnOpKind opKind) {
		return jnc_DerivableType_getUnaryOperator(this, opKind);
	}

	jnc::OverloadableFunction
	getBinaryOperator(jnc_BinOpKind opKind) {
		return jnc_DerivableType_getBinaryOperator(this, opKind);
	}

	jnc::OverloadableFunction
	getCallOperator() {
		return jnc_DerivableType_getCallOperator(this);
	}

	size_t
	getCastOperatorCount() {
		return jnc_DerivableType_getCastOperatorCount(this);
	}

	jnc_Function*
	getCastOperator(size_t index) {
		return jnc_DerivableType_getCastOperator(this, index);
	}

	size_t
	getBaseTypeCount() {
		return jnc_DerivableType_getBaseTypeCount(this);
	}

	jnc_BaseTypeSlot*
	getBaseType(size_t index) {
		return jnc_DerivableType_getBaseType(this, index);
	}

	size_t
	findBaseTypeOffset(jnc_Type* baseType) {
		return jnc_DerivableType_findBaseTypeOffset(this, baseType);
	}

	size_t
	getStaticVariableCount() {
		return jnc_DerivableType_getStaticVariableCount(this);
	}

	jnc_Variable*
	getStaticVariable(size_t index) {
		return jnc_DerivableType_getStaticVariable(this, index);
	}

	size_t
	getFieldCount() {
		return jnc_DerivableType_getFieldCount(this);
	}

	jnc_Field*
	getField(size_t index) {
		return jnc_DerivableType_getField(this, index);
	}

	size_t
	getMethodCount() {
		return jnc_DerivableType_getMethodCount(this);
	}

	jnc_Function*
	getMethod(size_t index) {
		return jnc_DerivableType_getMethod(this, index);
	}

	size_t
	getPropertyCount() {
		return jnc_DerivableType_getPropertyCount(this);
	}

	jnc_Property*
	getProperty(size_t index) {
		return jnc_DerivableType_getProperty(this, index);
	}
};

#endif // _JNC_CORE

//..............................................................................

/// @}
/// \addtogroup type
/// @{

JNC_INLINE
bool_t
jnc_isConstructibleType(jnc_Type* type) {
	return
		(jnc_Type_getTypeKindFlags(type) & jnc_TypeKindFlag_Derivable) &&
		jnc_DerivableType_getConstructor((jnc_DerivableType*)type).m_item;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_FieldFlag FieldFlag;

const FieldFlag
	FieldFlag_LayoutReady = jnc_FieldFlag_LayoutReady;

//..............................................................................

inline
bool
isConstructibleType(Type* type) {
	return jnc_isConstructibleType(type) != 0;
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
