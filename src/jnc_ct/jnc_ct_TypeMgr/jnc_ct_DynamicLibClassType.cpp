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
#include "jnc_ct_DynamicLibClassType.h"
#include "jnc_ct_DynamicLibNamespace.h"
#include "jnc_ct_ArrayType.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

DynamicLibClassType::DynamicLibClassType()
{
	m_libNamespace = NULL;
	m_classTypeKind = ClassTypeKind_DynamicLib;
	m_namespaceStatus = NamespaceStatus_Ready;
}

DynamicLibNamespace*
DynamicLibClassType::createLibNamespace()
{
	m_libNamespace = m_module->m_namespaceMgr.createGlobalNamespace<DynamicLibNamespace>("lib", this);

	bool result = addItem(m_libNamespace);
	ASSERT(result);

	return m_libNamespace;
}

bool
DynamicLibClassType::ensureFunctionTable()
{
	if (m_flags & DynamicLibClassTypeFlag_FunctionTableReady)
		return true;

	bool result = m_libNamespace->ensureNamespaceReady();
	if (!result)
		return false;

	size_t functionCount = m_libNamespace->getFunctionCount();
	if (!functionCount)
	{
		err::setFormatStringError("dynamiclib '%s' has no functions", getQualifiedName().sz());
		return false;
	}

	ArrayType* functionTableType = m_module->m_typeMgr.getStdType(StdType_BytePtr)->getArrayType(functionCount);
	createField(functionTableType);

	m_flags |= DynamicLibClassTypeFlag_FunctionTableReady;
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
