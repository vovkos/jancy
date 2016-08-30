#include "pch.h"
#include "jnc_ct_ImportType.h"
#include "jnc_ct_Decl.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

void
ImportType::applyFixups ()
{
	ASSERT (m_actualType);

	size_t count = m_fixupArray.getCount ();
	for (size_t i = 0; i < count; i++)
		*m_fixupArray [i] = m_actualType;
}

//.............................................................................

ImportPtrType*
NamedImportType::getImportPtrType (
	uint_t typeModifiers,
	uint_t flags
	)
{
	return m_module->m_typeMgr.getImportPtrType (this, typeModifiers, flags);
}

//.............................................................................

ImportPtrType::ImportPtrType ()
{
	m_typeKind = TypeKind_ImportPtr;
	m_targetType = NULL;
	m_typeModifiers = 0;
}

void
ImportPtrType::prepareTypeString ()
{
	ASSERT (m_targetType);

	if (m_actualType)
	{
		m_typeString = m_actualType->getTypeString ();
		return;
	}

	m_typeString = "import ";

	if (m_typeModifiers)
	{
		m_typeString += getTypeModifierString (m_typeModifiers);
		m_typeString += ' ';
	}

	m_typeString += m_targetType->getQualifiedName ();
	m_typeString += '*';
}

//.............................................................................

ImportIntModType::ImportIntModType ()
{
	m_typeKind = TypeKind_ImportPtr;
	m_importType = NULL;
	m_typeModifiers = 0;
}

void
ImportIntModType::prepareTypeString ()
{
	if (m_actualType)
	{
		m_typeString = m_actualType->getTypeString ();
		return;
	}

	m_typeString = "import ";

	if (m_typeModifiers)
	{
		m_typeString += getTypeModifierString (m_typeModifiers);
		m_typeString += ' ';
	}

	m_typeString += m_importType->getQualifiedName ();
}

//.............................................................................

} // namespace ct
} // namespace jnc

