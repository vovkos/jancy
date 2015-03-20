// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_DataPtrType.h"
#include "jnc_FunctionPtrType.h"
#include "jnc_PropertyPtrType.h"

namespace jnc {

//.............................................................................

struct Variant
{
	Type* m_type;

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
		intptr_t (* m_func) (...);

		DataPtr m_dataPtr;
		IfaceHdr* m_classPtr;
		FunctionPtr m_functionPtr;
		PropertyPtr m_propertyPtr;
	};
};

//.............................................................................

} // namespace jnc {
