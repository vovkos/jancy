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

#include "jnc_rtl_DerivableType.h"
#include "jnc_ct_StructType.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(StructField)
JNC_DECLARE_OPAQUE_CLASS_TYPE(StructType)

//..............................................................................

class StructField:
	public ModuleItemBase<ct::StructField>,
	public ModuleItemDecl,
	public ModuleItemInitializer
{
public:
	StructField(ct::StructField* field):
		ModuleItemBase(field),
		ModuleItemDecl(field),
		ModuleItemInitializer(field)
	{
	}

	Type*
	JNC_CDECL
	getType()
	{
		return rtl::getType(m_item->getType());
	}

	uint_t
	JNC_CDECL
	getPtrTypeFlags()
	{
		return m_item->getPtrTypeFlags();
	}

	size_t
	JNC_CDECL
	getOffset()
	{
		return m_item->getOffset();
	}
};

//..............................................................................

class StructType: public DerivableTypeBase<ct::StructType>
{
public:
	StructType(ct::StructType* type):
		DerivableTypeBase(type)
	{
	}

	size_t
	JNC_CDECL
	getFieldAlignment()
	{
		return m_item->getFieldAlignment();
	}

	size_t
	JNC_CDECL
	getFieldActualSize()
	{
		return m_item->getFieldActualSize();
	}

	size_t
	JNC_CDECL
	getFieldAlignedSize()
	{
		return m_item->getFieldAlignedSize();
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
