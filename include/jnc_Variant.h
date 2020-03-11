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

#define _JNC_VARIANT_H

#include "jnc_RuntimeStructs.h"
#include "jnc_OpKind.h"

/**

\defgroup variant Variant
	\ingroup runtime-structs
	\import{jnc_Variant.h}

\addtogroup variant
@{
*/

typedef struct jnc_Variant jnc_Variant;

//..............................................................................

JNC_EXTERN_C
bool_t
jnc_Variant_cast(
	const jnc_Variant* variant,
	jnc_Type* type,
	void* buffer
	);

JNC_EXTERN_C
bool_t
jnc_Variant_unaryOperator(
	const jnc_Variant* variant,
	jnc_UnOpKind opKind,
	jnc_Variant* result
	);

JNC_EXTERN_C
bool_t
jnc_Variant_binaryOperator(
	const jnc_Variant* variant,
	const jnc_Variant* variant2,
	jnc_BinOpKind opKind,
	jnc_Variant* result
	);

JNC_EXTERN_C
bool_t
jnc_Variant_relationalOperator(
	const jnc_Variant* variant,
	const jnc_Variant* variant2,
	jnc_BinOpKind opKind,
	bool_t* result
	);

JNC_EXTERN_C
bool_t
jnc_Variant_getMember(
	const jnc_Variant* variant,
	const char* name,
	jnc_Variant* result
	);

JNC_EXTERN_C
bool_t
jnc_Variant_setMember(
	jnc_Variant* variant,
	const char* name,
	jnc_Variant value
	);

JNC_EXTERN_C
bool_t
jnc_Variant_getElement(
	const jnc_Variant* variant,
	size_t index,
	jnc_Variant* result
	);

JNC_EXTERN_C
bool_t
jnc_Variant_setElement(
	jnc_Variant* variant,
	size_t index,
	jnc_Variant value
	);

JNC_INLINE
bool_t
jnc_Variant_isNull(const jnc_Variant* variant);

JNC_INLINE
bool_t
jnc_Variant_isEqual(
	const jnc_Variant* variant,
	const jnc_Variant* variant2
	)
{
	bool_t result = 0;
	return jnc_Variant_relationalOperator(variant, variant2, jnc_BinOpKind_Eq, &result) && result;
}

JNC_EXTERN_C
size_t
jnc_Variant_hash(const jnc_Variant* variant);

JNC_EXTERN_C
bool_t
jnc_Variant_create(
	jnc_Variant* variant,
	const void* p,
	jnc_Type* type
	);

JNC_EXTERN_C
const char*
jnc_Variant_format_v(
	const jnc_Variant* variant,
	const char* fmtSpecifier
	);

#ifdef _JNC_CORE

JNC_EXTERN_C
size_t
jnc_Variant_format(
	const jnc_Variant* variant,
	sl::String* string,
	const char* fmtSpecifier
	);

#endif

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_Variant
{
	union
	{
		/// \unnamed{union}
		int8_t m_int8;
		uint8_t m_uint8;
		int16_t m_int16;
		uint16_t m_uint16;
		int32_t m_int32;
		uint32_t m_uint32;
		int64_t m_int64;
		uint64_t m_uint64;
		intptr_t m_intptr;
		uintptr_t m_uintptr;

		float m_float;
		double m_double;

		void* m_p;
		jnc_IfaceHdr* m_classPtr;
		jnc_DataPtr m_dataPtr;
		jnc_FunctionPtr m_functionPtr;
		jnc_PropertyPtr m_propertyPtr;
	} JNC_GCC_ALIGN(8);

#if (JNC_PTR_SIZE == 4)
	char m_padding[4]; // ensure the same layout regardless of pack factor
#endif

	jnc_Type* m_type;

#ifdef __cplusplus
	bool
	isNull() const
	{
		return jnc_Variant_isNull(this) != 0;
	}

	bool
	create(
		const void* p,
		jnc_Type* type
		)
	{
		return jnc_Variant_create(this, p, type) != 0;
	}

	bool
	cast(
		jnc_Type* type,
		void* buffer
		) const
	{
		return jnc_Variant_cast(this, type, buffer) != 0;
	}

	bool
	unaryOperator(
		jnc_UnOpKind opKind,
		jnc_Variant* result
		) const
	{
		return jnc_Variant_unaryOperator(this, opKind, result) != 0;
	}

	bool
	unaryOperator(jnc_UnOpKind opKind)
	{
		return jnc_Variant_unaryOperator(this, opKind, this) != 0;
	}

	bool
	binaryOperator(
		const jnc_Variant* variant2,
		jnc_BinOpKind opKind,
		jnc_Variant* result
		) const
	{
		return jnc_Variant_binaryOperator(this, variant2, opKind, result) != 0;
	}

	bool
	binaryOperator(
		const jnc_Variant* variant2,
		jnc_BinOpKind opKind
		)
	{
		return jnc_Variant_binaryOperator(this, variant2, opKind, this) != 0;
	}

	bool
	relationalOperator(
		const jnc_Variant* variant2,
		jnc_BinOpKind opKind,
		bool* result
		) const;

	bool
	getMember(
		const char* name,
		jnc_Variant* result
		) const
	{
		return jnc_Variant_getMember(this, name, result) != 0;
	}

	bool
	setMember(
		const char* name,
		jnc_Variant value
		)
	{
		return jnc_Variant_setMember(this, name, value) != 0;
	}

	bool
	getElement(
		size_t index,
		jnc_Variant* result
		) const
	{
		return jnc_Variant_getElement(this, index, result) != 0;
	}

	bool
	setElement(
		size_t index,
		jnc_Variant value
		)
	{
		return jnc_Variant_setElement(this, index, value) != 0;
	}

	bool
	isEqual(const jnc_Variant* variant2) const
	{
		return jnc_Variant_isEqual(this, variant2) != 0;
	}

	bool
	isEqual(const jnc_Variant& variant2) const
	{
		return jnc_Variant_isEqual(this, &variant2) != 0;
	}

	size_t
	hash() const
	{
		return jnc_Variant_hash(this);
	}

	const char*
	format_v(const char* fmtSpecifier = NULL) const
	{
		return jnc_Variant_format_v(this, fmtSpecifier);
	}

	#ifdef _JNC_CORE

	size_t
	format(
		sl::String* string,
		const char* fmtSpecifier
		)
	{
		return jnc_Variant_format(this, string, fmtSpecifier);
	}

	#endif
#endif // __cplusplus
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_INLINE
bool_t
jnc_Variant_isNull(const jnc_Variant* variant)
{
	return variant->m_type == NULL;
}

JNC_SELECT_ANY jnc_Variant jnc_g_nullVariant = { 0 };

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_Variant Variant;

JNC_SELECT_ANY Variant g_nullVariant = { 0 };

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
