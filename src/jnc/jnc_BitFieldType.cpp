#include "pch.h"
#include "jnc_BitFieldType.h"

namespace jnc {

//.............................................................................

BitFieldType::BitFieldType ()
{
	m_typeKind = TypeKind_BitField;
	m_flags = TypeFlagKind_Pod;
	m_baseType = NULL;
	m_baseType_i = NULL;
	m_bitOffset = 0;
	m_bitCount = 0;
}

void
BitFieldType::prepareTypeString ()
{
	m_typeString.format (
		"%s:%d:%d",
		m_baseType->getTypeString ().cc (), // thanks a lot gcc
		m_bitOffset,
		m_bitOffset + m_bitCount
		);
}

bool
BitFieldType::calcLayout ()
{
	if (m_baseType_i)
		m_baseType = m_baseType_i->getActualType ();

	TypeKind typeKind = m_baseType->getTypeKind ();
	if (typeKind < TypeKind_Int8 || typeKind > TypeKind_Int64_u)
	{
		err::setFormatStringError ("bit field can only be used with little-endian integer types");
		return NULL;
	}

	m_size = m_baseType->getSize ();
	m_alignFactor = m_baseType->getAlignFactor ();
	return true;
}

//.............................................................................

} // namespace jnc {

