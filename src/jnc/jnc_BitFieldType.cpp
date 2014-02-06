#include "pch.h"
#include "jnc_BitFieldType.h"

namespace jnc {

//.............................................................................

CBitFieldType::CBitFieldType ()
{
	m_TypeKind = EType_BitField;
	m_Flags = ETypeFlag_Pod;
	m_pBaseType = NULL;
	m_pBaseType_i = NULL;
	m_BitOffset = 0;
	m_BitCount = 0;
}

void
CBitFieldType::PrepareTypeString ()
{
	m_TypeString.Format (
		"%s:%d:%d",
		m_pBaseType->GetTypeString ().cc (), // thanks a lot gcc
		m_BitOffset,
		m_BitOffset + m_BitCount
		);
}

bool
CBitFieldType::CalcLayout ()
{
	if (m_pBaseType_i)
		m_pBaseType = m_pBaseType_i->GetActualType ();

	EType TypeKind = m_pBaseType->GetTypeKind ();
	if (TypeKind < EType_Int8 || TypeKind > EType_Int64_u)
	{
		err::SetFormatStringError ("bit field can only be used with little-endian integer types");
		return NULL;
	}

	m_Size = m_pBaseType->GetSize ();
	m_AlignFactor = m_pBaseType->GetAlignFactor ();
	return true;
}

//.............................................................................

} // namespace jnc {

