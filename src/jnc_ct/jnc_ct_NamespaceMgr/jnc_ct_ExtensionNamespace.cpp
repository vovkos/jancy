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
#include "jnc_ct_ExtensionNamespace.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
ExtensionNamespace::addMethod(Function* function)
{
	if (function->isVirtual())
	{
		err::setFormatStringError("invalid storage '%s' in type extension", getStorageKindString (function->getStorageKind ()));
		return false;
	}

	if (function->getFunctionKind() != FunctionKind_Normal)
	{
		err::setFormatStringError("'%s' cannot be a part of type extension", getFunctionKindString (function->getFunctionKind ()));
		return false;
	}

	bool result = addItem(function);
	if (!result)
		return false;

	if (m_type->getTypeKindFlags() & TypeKindFlag_Derivable)
		fixupMethod(function);
	else
		m_fixupMethodArray.append(function);

	return true;
}

bool
ExtensionNamespace::addProperty(Property* prop)
{
	if (prop->isVirtual())
	{
		err::setFormatStringError("invalid storage '%s' in type extension", getStorageKindString (prop->m_storageKind));
		return false;
	}

	bool result = addItem(prop);
	if (!result)
		return false;

	if (m_type->getTypeKindFlags() & TypeKindFlag_Derivable)
		fixupProperty(prop);
	else
		m_fixupPropertyArray.append(prop);

	return true;
}

bool
ExtensionNamespace::calcLayout()
{
	if (!(m_type->getTypeKindFlags() & TypeKindFlag_Derivable))
	{
		err::setFormatStringError("'%s' cannot have a type extension", m_type->getTypeString().sz());
		return false;
	}

	size_t count = m_fixupMethodArray.getCount();
	for (size_t i = 0; i < count; i++)
		fixupMethod(m_fixupMethodArray[i]);

	count = m_fixupPropertyArray.getCount();
	for (size_t i = 0; i < count; i++)
		fixupProperty(m_fixupPropertyArray[i]);

	return true;
}

void
ExtensionNamespace::fixupMethod(Function* function)
{
	ASSERT(m_type->getTypeKindFlags() & TypeKindFlag_Derivable);
	DerivableType* derivableType = (DerivableType*)m_type;

	if (function->m_storageKind != StorageKind_Static)
	{
		function->m_storageKind = StorageKind_Member;
		function->convertToMemberMethod(derivableType);
	}

	function->m_parentNamespace = derivableType;
	function->m_extensionNamespace = this;
}

void
ExtensionNamespace::fixupProperty(Property* prop)
{
	ASSERT(m_type->getTypeKindFlags() & TypeKindFlag_Derivable);
	DerivableType* derivableType = (DerivableType*)m_type;

	if (prop->m_storageKind != StorageKind_Static)
	{
		prop->m_storageKind = StorageKind_Member;
		prop->m_parentType = derivableType;
	}

	prop->m_parentNamespace = derivableType;
	prop->m_extensionNamespace = this;
}

//..............................................................................

} // namespace ct
} // namespace jnc
