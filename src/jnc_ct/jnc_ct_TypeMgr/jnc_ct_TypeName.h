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

#include "jnc_ct_ModuleItem.h"
#include "jnc_ct_QualifiedName.h"

namespace jnc {
namespace ct {

class Type;

//..............................................................................

class TypeName: public ModuleItemPos {
protected:
	QualifiedName m_name;

public:
	const QualifiedName&
	getName() const {
		return m_name;
	}

	Type*
	lookupType() const {
		return lookupType(m_parentNamespace);
	}

	Type*
	lookupType(Namespace* nspace) const;

	static
	sl::String
	createSignature(
		uint16_t signaturePrefix,
		Namespace* parentNamespace,
		const QualifiedName& name
	);

protected:
	Type*
	lookupTypeImpl(
		const ModuleItemContext& context,
		Namespace* nspace,
		uint_t compileFlags,
		bool isResolvingRecursion
	) const;

	sl::String
	createTypeString(const sl::StringRef& prefix);
};

//..............................................................................

} // namespace ct
} // namespace jnc
