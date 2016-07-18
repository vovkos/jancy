#pragma once

#define _JNC_VARIANT_H

#include "jnc_RuntimeStructs.h"

typedef struct jnc_Variant jnc_Variant;

//.............................................................................

int
jnc_Variant_cast (
	jnc_Variant* variant,
	jnc_Type* type,
	void* buffer
	);

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

#if (_AXL_PTR_SIZE == 4)
	char m_padding [4]; // ensure the same layout regardless of pack factor
#endif

	jnc_Type* m_type;

#ifdef __cplusplus
	bool
	cast (
		jnc_Type* type,
		void* buffer
		)
	{
		return jnc_Variant_cast (this, type, buffer) != 0;
	}
#endif // __cplusplus
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

AXL_SELECT_ANY jnc_Variant jnc_g_nullVariant = { 0 };

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

typedef jnc_Variant Variant;

AXL_SELECT_ANY Variant g_nullVariant = { 0 };

//.............................................................................

} // namespace jnc

#endif // __cplusplus
