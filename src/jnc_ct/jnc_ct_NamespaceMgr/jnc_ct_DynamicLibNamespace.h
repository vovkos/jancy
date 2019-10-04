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

#include "jnc_ct_GlobalNamespace.h"
#include "jnc_ct_ClassType.h"

namespace jnc {
namespace ct {

class DynamicLibClassType;

//..............................................................................

class DynamicLibNamespace: public GlobalNamespace
{
	friend class Parser;

protected:
	size_t m_functionCount;

public:
	DynamicLibNamespace()
	{
		m_namespaceKind = NamespaceKind_DynamicLib;
		m_functionCount = 0;
	}

	DynamicLibClassType* getLibraryType()
	{
		return (DynamicLibClassType*)m_parentNamespace;
	}

	size_t
	getFunctionCount()
	{
		return m_functionCount;
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
