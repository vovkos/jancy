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

struct ReactorBody;

//..............................................................................

enum ParseContextKind {
	ParseContextKind_Body,
	ParseContextKind_Expression,
	ParseContextKind_TypeName,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class ParseContext {
protected:
	Module* m_module;
	Unit* m_prevUnit;
	ReactorBody* m_prevReactorBody;
	bool m_isNamespaceOpened;

public:
	ParseContext(
		ParseContextKind contextKind,
		Module* module,
		Unit* unit,
		Namespace* nspace
	) {
		set(contextKind, module, unit, nspace);
	}

	ParseContext(
		ParseContextKind contextKind,
		Module* module,
		const ModuleItemContext& itemContext
	) {
		set(contextKind, module, itemContext.getParentUnit(), itemContext.getParentNamespace());
	}

	~ParseContext() {
		restore();
	}

protected:
	void
	set(
		ParseContextKind contextKind,
		Module* module,
		Unit* unit,
		Namespace* nspace
	);

	void
	restore();
};

//..............................................................................

} // namespace ct
} // namespace jnc
