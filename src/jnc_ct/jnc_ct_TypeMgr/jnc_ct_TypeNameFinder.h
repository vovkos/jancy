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

namespace jnc {
namespace ct {

class QualifiedName;

//..............................................................................

class TypeNameFinder: public ModuleItemPos {
protected:
	uint_t m_compileFlags;

public:
	TypeNameFinder(const ModuleItemPos& origin) {
		setup(origin.getParentUnit(), origin.getParentNamespace(), origin.getPos());
	}

	TypeNameFinder(
		Unit* unit,
		Namespace* nspace,
		const lex::LineCol& pos
	) {
		setup(unit, nspace, pos);
	}

	Type*
	find(const QualifiedName& name) {
		return findImpl(m_parentNamespace, name, false);
	}

protected:
	void
	setup(
		Unit* parentUnit,
		Namespace* parentNamespace,
		const lex::LineCol& pos
	);

	Type*
	findImpl(
		Namespace* nspace,
		const QualifiedName& name,
		bool isResolvingRecursion
	);
};

//..............................................................................

} // namespace ct
} // namespace jnc
