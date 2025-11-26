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
#include "jnc_ct_FunctionOverload.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

size_t
FunctionOverload::addOverload(Function* function) {
	size_t overloadIdx = m_typeOverload.addOverload(function->getType());
	if (overloadIdx == -1)
		return -1;

	if (function->isMember())
		m_flags |= FunctionOverloadFlag_HasMembers;

	ASSERT(overloadIdx == m_overloadArray.getCount());
	m_overloadArray.append(function);
	return overloadIdx;
}

bool
FunctionOverload::require() {
	size_t count = m_overloadArray.getCount();
	for (size_t i = 0; i < count; i++)
		m_overloadArray[i]->require();

	return true;
}

bool
FunctionOverload::generateDocumentation(
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
) {
	sl::String overloadXml;

	size_t overloadCount = m_overloadArray.getCount();
	for (size_t i = 0; i < overloadCount; i++) {
		Function* overload = m_overloadArray[i];
		overload->generateDocumentation(outputDir, &overloadXml, indexXml);
		itemXml->append('\n');
		itemXml->append(overloadXml);
	}

	return true;
}

sl::StringRef
FunctionOverload::createItemString(size_t index) {
	if (index != ModuleItemStringKind_Synopsis)
		return createItemStringImpl(index, this);

	sl::String string = createItemStringImpl(index, this);
	string.appendFormat(" (%d overloads)", m_overloadArray.getCount());
	return string;
}

//..............................................................................

} // namespace ct
} // namespace jnc
