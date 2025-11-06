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
#include "jnc_ct_FunctionArg.h"
#include "jnc_ct_Type.h"

namespace jnc {
namespace ct {

//..............................................................................

sl::String
FunctionArg::getArgString() {
	sl::String string;

	string = m_type->getTypeStringPrefix();

	if (m_storageKind == StorageKind_This) {
		string += " this";
	} else if (!m_name.isEmpty()) {
		string += ' ';
		string += m_name;
	}

	sl::String suffix = m_type->getTypeStringSuffix();
	if (!suffix.isEmpty())
		string += suffix;

	if (!m_initializer.isEmpty()) {
		string += " = ";
		string += getInitializerString();
	}

	return string;
}

sl::String
FunctionArg::getArgDoxyLinkedText() {
	sl::String string;

	string = m_type->getDoxyLinkedTextPrefix();

	if (m_storageKind == StorageKind_This) {
		string += " this";
	} else if (!m_name.isEmpty()) {
		string += ' ';
		string += m_name;
	}

	sl::String suffix = m_type->getDoxyLinkedTextSuffix();
	if (!suffix.isEmpty()) {
		string += ' ';
		string += suffix;
	}

	if (!m_initializer.isEmpty()) {
		string += ' ';
		string += getInitializerString();
	}

	return string;
}

//..............................................................................

} // namespace ct
} // namespace jnc
