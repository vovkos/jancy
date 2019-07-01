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
#include "jnc_ct_ImportType.h"
#include "jnc_ct_Decl.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

void
ImportType::applyFixups()
{
	ASSERT(m_actualType);

	size_t count = m_fixupArray.getCount();
	for (size_t i = 0; i < count; i++)
		*m_fixupArray[i] = m_actualType;
}

//..............................................................................

NamedImportType::NamedImportType()
{
	m_typeKind = TypeKind_NamedImport;
	m_anchorNamespace = NULL;
}

ImportPtrType*
NamedImportType::getImportPtrType(
	uint_t typeModifiers,
	uint_t flags
	)
{
	return m_module->m_typeMgr.getImportPtrType(this, typeModifiers, flags);
}

sl::String
NamedImportType::createSignature(
	const QualifiedName& name,
	Namespace* anchorNamespace,
	const QualifiedName& orphanName
	)
{
	sl::String signature = sl::formatString("ZN%s", anchorNamespace->createQualifiedName (name).sz());

	if (!orphanName.isEmpty())
	{
		signature += '-';
		signature += orphanName.getFullName();
	}

	return signature;
}

void
NamedImportType::pushImportSrcPosError()
{
	lex::pushSrcPosError(m_parentUnit->getFilePath(), m_pos);
}

Type*
NamedImportType::resolveSuperImportType()
{
	if (m_actualType->getTypeKind() != TypeKind_NamedImport)
		return m_actualType;

	if (m_flags & ImportTypeFlag_ImportLoop)
	{
		err::setFormatStringError("'%s': import loop detected", getQualifiedName().sz());
		return NULL;
	}

	m_flags |= ImportTypeFlag_ImportLoop;
	Type* result = ((NamedImportType*)m_actualType)->resolveSuperImportType();
	m_flags &= ~ImportTypeFlag_ImportLoop;

	if (result)
		m_actualType = result;

	return result;
}

//..............................................................................

ImportPtrType::ImportPtrType()
{
	m_typeKind = TypeKind_ImportPtr;
	m_targetType = NULL;
	m_typeModifiers = 0;
}

void
ImportPtrType::prepareTypeString()
{
	ASSERT(m_targetType);
	TypeStringTuple* tuple = getTypeStringTuple();

	if (m_actualType)
	{
		tuple->m_typeStringPrefix = m_actualType->getTypeStringPrefix();
		tuple->m_typeStringSuffix = m_actualType->getTypeStringSuffix();
		return;
	}

	tuple->m_typeStringPrefix = "import ";

	if (m_typeModifiers)
	{
		tuple->m_typeStringPrefix += getTypeModifierString(m_typeModifiers);
		tuple->m_typeStringPrefix += ' ';
	}

	tuple->m_typeStringPrefix += m_targetType->getQualifiedName();
	tuple->m_typeStringPrefix += '*';
}

//..............................................................................

ImportIntModType::ImportIntModType()
{
	m_typeKind = TypeKind_ImportPtr;
	m_importType = NULL;
	m_typeModifiers = 0;
}

void
ImportIntModType::prepareTypeString()
{
	TypeStringTuple* tuple = getTypeStringTuple();

	if (m_actualType)
	{
		tuple->m_typeStringPrefix = m_actualType->getTypeStringPrefix();
		return;
	}

	tuple->m_typeStringPrefix = "import ";

	if (m_typeModifiers)
	{
		tuple->m_typeStringPrefix += getTypeModifierString(m_typeModifiers);
		tuple->m_typeStringPrefix += ' ';
	}

	tuple->m_typeStringPrefix += m_importType->getQualifiedName();
}

//..............................................................................

} // namespace ct
} // namespace jnc
