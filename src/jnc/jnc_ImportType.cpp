#include "pch.h"
#include "jnc_ImportType.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CImportPtrType*
CNamedImportType::GetImportPtrType (
	uint_t TypeModifiers,
	uint_t Flags
	)
{
	return m_pModule->m_TypeMgr.GetImportPtrType (this, TypeModifiers, Flags);
}

//.............................................................................

CImportPtrType::CImportPtrType ()
{
	m_TypeKind = EType_ImportPtr;
	m_pTargetType = NULL;
	m_TypeModifiers = 0;
}

void
CImportPtrType::PrepareTypeString ()
{
	ASSERT (m_pTargetType);

	if (m_pActualType)
	{
		m_TypeString = m_pActualType->GetTypeString ();
		return;
	}

	m_TypeString = "import ";

	if (m_TypeModifiers)
	{
		m_TypeString += GetTypeModifierString (m_TypeModifiers);
		m_TypeString += ' ';
	}

	m_TypeString += m_pTargetType->GetQualifiedName ();
	m_TypeString += '*';
}

//.............................................................................

CImportIntModType::CImportIntModType ()
{
	m_TypeKind = EType_ImportPtr;
	m_pImportType = NULL;
	m_TypeModifiers = 0;
}

void
CImportIntModType::PrepareTypeString ()
{
	if (m_pActualType)
	{
		m_TypeString = m_pActualType->GetTypeString ();
		return;
	}

	m_TypeString = "import ";

	if (m_TypeModifiers)
	{
		m_TypeString += GetTypeModifierString (m_TypeModifiers);
		m_TypeString += ' ';
	}

	m_TypeString += m_pImportType->GetQualifiedName ();
}

//.............................................................................

} // namespace jnc {

