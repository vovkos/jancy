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
#include "jnc_ct_PropertyVerifier.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
PropertyVerifier::checkIndexSignature(
	FunctionKind functionKind,
	FunctionType* functionType
) {
	ASSERT(functionKind == FunctionKind_Getter || functionKind == FunctionKind_Setter);

	if (functionType->isMemberMethodType())
		functionType = functionType->getShortType();

	sl::ArrayRef<FunctionArg*> indexArgArray;

	if (functionKind == FunctionKind_Getter)
		indexArgArray = functionType->getArgArray();
	if (functionKind == FunctionKind_Setter) {
		size_t count = functionType->getArgArray().getCount();
		if (!count) {
			err::setError("'set' must have at least one argument");
			return false;
		}

		indexArgArray = sl::ArrayRef<FunctionArg*>(functionType->getArgArray().cp(), count - 1);
	}

	if (!m_isInitialized) {
		m_indexArgArray = indexArgArray;
		m_isInitialized = true;
		return true;
	}

	size_t count = indexArgArray.getCount();
	if (count != m_indexArgArray.getCount()) {
		err::setError("index argument count mismatch in property accessors");
		return false;
	}

	for (size_t i = 0; i < count; i++) {
		Type* type1 = indexArgArray[i]->getItemType();
		Type* type2 = m_indexArgArray[i]->getItemType();
		if (!type1->isEqual(type2)) {
			err::setError("index argument type mismatch in property accessors");
			return false;
		}
	}

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
