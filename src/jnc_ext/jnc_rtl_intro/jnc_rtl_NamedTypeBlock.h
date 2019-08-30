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

#include "jnc_ExtensionLib.h"
#include "jnc_rtl_IntrospectionLib.h"
#include "jnc_ct_NamedTypeBlock.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(NamedTypeBlock)

//..............................................................................

class NamedTypeBlock: public IfaceHdr
{
protected:
	ct::NamedTypeBlock* m_block;

public:
	NamedTypeBlock(ct::NamedTypeBlock* block)
	{
		m_block = block;
	}

public:
	Function*
	JNC_CDECL
	getStaticConstructor()
	{
		return rtl::getFunction(m_block->getStaticConstructor());
	}

	Function*
	JNC_CDECL
	getStaticDestructor()
	{
		return rtl::getFunction(m_block->getStaticDestructor());
	}

	Function*
	JNC_CDECL
	getConstructor()
	{
		return rtl::getFunction(m_block->getConstructor());
	}

	Function*
	JNC_CDECL
	getDestructor()
	{
		return rtl::getFunction(m_block->getDestructor());
	}

	size_t
	JNC_CDECL
	getMemberFieldCount()
	{
		return m_block->getMemberFieldArray().getCount();
	}

	StructField*
	JNC_CDECL
	getMemberField(size_t index)
	{
		size_t count = m_block->getMemberFieldArray().getCount();
		return index < count ? rtl::getStructField(m_block->getMemberFieldArray()[index]) : NULL;
	}

	size_t
	JNC_CDECL
	getMemberMethodCount()
	{
		return m_block->getMemberMethodArray().getCount();
	}

	Function*
	JNC_CDECL
	getMemberMethod(size_t index)
	{
		size_t count = m_block->getMemberMethodArray().getCount();
		return index < count ? rtl::getFunction(m_block->getMemberMethodArray()[index]) : NULL;
	}

	size_t
	JNC_CDECL
	getMemberPropertyCount()
	{
		return m_block->getMemberPropertyArray().getCount();
	}

	Property*
	JNC_CDECL
	getMemberProperty(size_t index)
	{
		size_t count = m_block->getMemberPropertyArray().getCount();
		return index < count ? rtl::getProperty(m_block->getMemberPropertyArray()[index]) : NULL;
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
