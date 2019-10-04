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

namespace jnc {
namespace ct {

//..............................................................................

class ExtensionNamespace: public GlobalNamespace
{
	friend class Parser;

protected:
	Type* m_type;
	sl::Array<Function*> m_fixupMethodArray;
	sl::Array<Property*> m_fixupPropertyArray;

public:
	ExtensionNamespace()
	{
		m_namespaceKind = NamespaceKind_Extension;
		m_type = NULL;
	}

	Type* getType()
	{
		return m_type;
	}

	bool
	addMethod(Function* function);

	bool
	addProperty(Property* prop);

protected:
	virtual
	bool
	parseBody();

	void
	fixupMethod(Function* function);

	void
	fixupProperty(Property* prop);
};

//..............................................................................

} // namespace ct
} // namespace jnc
