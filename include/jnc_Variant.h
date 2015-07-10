// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_RuntimeStructs.h"
#include "jnc_BinOp.h"

namespace jnc {

//.............................................................................

struct Variant
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
		IfaceHdr* m_classPtr;
		DataPtr m_dataPtr;
		FunctionPtr m_functionPtr;
		PropertyPtr m_propertyPtr;
	};

#if (_AXL_PTR_SIZE == 4)
	char m_padding [4]; // ensure the same layout regardless of pack factor
#endif

	Type* m_type;
};

AXL_SELECT_ANY Variant g_nullVariant = { 0 };

//.............................................................................

bool
variantRelationalOperator (
	BinOpKind opKind,
	const Variant& op1,
	const Variant& op2
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// the most used op

inline
bool
isVariantEqual (
	const Variant& op1,
	const Variant& op2
	)
{
	return variantRelationalOperator (BinOpKind_Eq, op1, op2);
}

//.............................................................................

class HashVariant
{
public:
	uintptr_t
	operator () (const Variant& variant);
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
		return !isVariantEqual (variant1, variant2);
	}
};

//.............................................................................

} // namespace jnc {
