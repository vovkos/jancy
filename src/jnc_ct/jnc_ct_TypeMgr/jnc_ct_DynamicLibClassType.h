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

#include "jnc_ct_ClassType.h"

namespace jnc {
namespace ct {

class DynamicLibNamespace;

//..............................................................................

enum DynamicLibClassTypeFlag {
	DynamicLibClassTypeFlag_FunctionTableReady = 0x01000000,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DynamicLibClassType: public ClassType {
protected:
	DynamicLibNamespace* m_libNamespace;

public:
	DynamicLibClassType();

	DynamicLibNamespace*
	getLibNamespace() {
		return m_libNamespace;
	}

	DynamicLibNamespace*
	createLibNamespace();

	bool
	ensureFunctionTable();

protected:
	virtual
	bool
	calcLayout() {
		return ensureFunctionTable() && ClassType::calcLayout();
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
