// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_DataPtrType.h"
#include "jnc_FunctionPtrType.h"
#include "jnc_PropertyPtrType.h"
#include "jnc_BinOp.h"

namespace jnc {

//.............................................................................

struct Variant
{
	union 
	{
		int8_t m_int8;
		uint8_t m_int8_u;
		int16_t m_int16;
		uint16_t m_int16_u;
		int32_t m_int32;
		uint32_t m_int32_u;
		int64_t m_int64;
		uint64_t m_int64_u;

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

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
variantRelationalOperator (
	BinOpKind opKind,
	const Variant& op1,
	const Variant& op2
	);

// the most used op

inline
bool
isEqualVariant (
	const Variant& op1,
	const Variant& op2
	)
{
	return variantRelationalOperator (BinOpKind_Eq, op1, op2);
}

//.............................................................................

} // namespace jnc {
