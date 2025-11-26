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

#include "jnc_ct_PropertyType.h"
#include "jnc_ct_Function.h"

namespace jnc {
namespace ct {

//..............................................................................

class PropertyVerifier {
protected:
	sl::ArrayRef<FunctionArg*> m_indexArgArray;
	bool m_isInitialized;

public:
	PropertyVerifier() {
		m_isInitialized = false;
	}

	bool
	checkGetter(FunctionType* functionType) {
		return checkIndexSignature(FunctionKind_Getter, functionType);
	}

	bool
	checkSetter(FunctionType* functionType) {
		return checkIndexSignature(FunctionKind_Setter, functionType);
	}

protected:
	bool
	checkIndexSignature(
		FunctionKind functionKind,
		FunctionType* functionType
	);
};

//..............................................................................

} // namespace ct
} // namespace jnc
