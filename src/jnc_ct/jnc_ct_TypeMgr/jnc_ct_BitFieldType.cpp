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

#include "pch.h"
#include "jnc_ct_BitFieldType.h"

namespace jnc {
namespace ct {

//..............................................................................

BitFieldType::BitFieldType()
{
	m_typeKind = TypeKind_BitField;
	m_flags = TypeFlag_Pod;
	m_baseType = NULL;
	m_bitOffset = 0;
	m_bitCount = 0;
}

void
BitFieldType::prepareTypeString()
{
	TypeStringTuple* tuple = getTypeStringTuple();

	tuple->m_typeStringPrefix = m_baseType->getTypeStringPrefix();
	tuple->m_typeStringSuffix.format(":%d:%d", m_bitOffset, m_bitOffset + m_bitCount);
}

void
BitFieldType::prepareDoxyLinkedText()
{
	TypeStringTuple* tuple = getTypeStringTuple();

	tuple->m_typeStringPrefix = m_baseType->getDoxyLinkedTextPrefix();
	tuple->m_typeStringSuffix = getTypeStringSuffix();
}

bool
BitFieldType::calcLayout()
{
	bool result = m_baseType->ensureLayout();
	if (!result)
		return false;

	TypeKind typeKind = m_baseType->getTypeKind();
	if (typeKind < TypeKind_Int8 || typeKind > TypeKind_Int64_beu)
	{
		err::setFormatStringError("bit field can only be used with integer types");
		return false;
	}

	m_size = m_baseType->getSize();
	m_alignment = m_baseType->getAlignment();
	return true;
}

sl::String
BitFieldType::getValueString(
	const void* p,
	const char* formatSpec
	)
{
	uint64_t value = 0;
	memcpy(&value, p, m_baseType->getSize());
	value >>= m_bitOffset;
	value &= (1 << m_bitCount) - 1;
	return m_baseType->getValueString(&value, formatSpec);
}

//..............................................................................

} // namespace ct
} // namespace jnc
