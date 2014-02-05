// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_DataPtrType.h"
#include "jnc_FunctionPtrType.h"
#include "jnc_PropertyPtrType.h"

namespace jnc {

//.............................................................................

struct TVariant
{
	CType* m_pType;

	union
	{
		int8_t m_Int8;
		uint8_t m_Int8_u;
		int16_t m_Int16;
		uint16_t m_Int16_u;
		int32_t m_Int32;
		uint32_t m_Int32_u;
		int64_t m_Int64;
		uint64_t m_Int64_u;

		float m_Float;
		double m_Double;

		void* m_p;
		intptr_t (* m_pf) (...);

		TDataPtr m_DataPtr;
		TIfaceHdr* m_pClassPtr;
		TFunctionPtr m_FunctionPtr;
		TPropertyPtr m_PropertyPtr;
	};
};

//.............................................................................

} // namespace jnc {
