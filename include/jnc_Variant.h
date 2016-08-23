#pragma once

#define _JNC_VARIANT_H

#include "jnc_RuntimeStructs.h"
#include "jnc_OpKind.h"

/// \addtogroup variant
/// @{

typedef struct jnc_Variant jnc_Variant;

//.............................................................................

JNC_EXTERN_C
int
jnc_Variant_cast (
	const jnc_Variant* variant,
	jnc_Type* type,
	void* buffer
	);

JNC_EXTERN_C
int
jnc_Variant_unaryOperator (
	const jnc_Variant* variant,
	jnc_UnOpKind opKind,
	jnc_Variant* result
	);

JNC_EXTERN_C
int
jnc_Variant_binaryOperator (
	const jnc_Variant* variant,
	const jnc_Variant* variant2,
	jnc_BinOpKind opKind,
	jnc_Variant* result
	);

JNC_EXTERN_C
int
jnc_Variant_relationalOperator (
	const jnc_Variant* variant,
	const jnc_Variant* variant2,
	jnc_BinOpKind opKind,
	int* result
	);

JNC_INLINE
int
jnc_Variant_isEqual (
	const jnc_Variant* variant,
	const jnc_Variant* variant2
	)
{
	int result = 0;
	return jnc_Variant_relationalOperator (variant, variant2, jnc_BinOpKind_Eq, &result) && result;
}

JNC_EXTERN_C
size_t
jnc_Variant_getHash (const jnc_Variant* variant);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_Variant
{
	union
	{
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
	};

#if (_JNC_PTR_SIZE == 4)
	char m_padding [4]; // ensure the same layout regardless of pack factor
#endif

	jnc_Type* m_type;

#ifdef __cplusplus
	bool
	cast (
		jnc_Type* type,
		void* buffer
		) const
	{
		return jnc_Variant_cast (this, type, buffer) != 0;
	}

	bool
	unaryOperator (
		jnc_UnOpKind opKind,
		jnc_Variant* result
		) const
	{
		return jnc_Variant_unaryOperator (this, opKind, result) != 0;
	}

	bool
	unaryOperator (jnc_UnOpKind opKind)
	{
		return jnc_Variant_unaryOperator (this, opKind, this) != 0;
	}

	bool
	binaryOperator (
		const jnc_Variant* variant2,
		jnc_BinOpKind opKind,
		jnc_Variant* result
		) const
	{
		return jnc_Variant_binaryOperator (this, variant2, opKind, result) != 0;
	}

	bool
	binaryOperator (
		const jnc_Variant* variant2,
		jnc_BinOpKind opKind
		)
	{
		return jnc_Variant_binaryOperator (this, variant2, opKind, this) != 0;
	}

	bool
	relationalOperator (
		const jnc_Variant* variant2,
		jnc_BinOpKind opKind,
		bool* result
		) const;

	bool
	isEqual (const jnc_Variant* variant2) const
	{
		return jnc_Variant_isEqual (this, variant2) != 0;
	}

	size_t
	getHash () const
	{
		return jnc_Variant_getHash (this);
	}
#endif // __cplusplus
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_SELECT_ANY jnc_Variant jnc_g_nullVariant = { 0 };

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

typedef jnc_Variant Variant;

JNC_SELECT_ANY Variant g_nullVariant = { 0 };

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class HashVariant
{
public:
	size_t
	operator () (const Variant& variant)
	{
		return variant.getHash ();
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CmpVariant
{
public:
	int operator () (
		const Variant& variant1,
		const Variant& variant2
		)
	{
		return !variant1.isEqual (&variant2);
	}
};

//.............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
