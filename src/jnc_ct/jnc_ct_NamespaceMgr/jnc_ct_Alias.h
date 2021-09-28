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

#include "jnc_ct_ImportType.h"

namespace jnc {
namespace ct {

//..............................................................................

enum AliasFlag {
	AliasFlag_InResolve = 0x010000, // used for detection of alias loops
};

//..............................................................................

class Alias:
	public ModuleItem,
	public ModuleItemDecl,
	public ModuleItemInitializer {
	friend class NamespaceMgr;

protected:
	enum ResolveStatus {
		ResolveStatus_Undefined = 0,
		ResolveStatus_Resolved,
		ResolveStatus_Error,
	};

protected:
	ModuleItem* m_targetItem;
	err::Error m_resolveError;

public:
	Alias();

	bool
	isResolved() {
		return m_targetItem != NULL;
	}

	Type*
	getType() {
		return ensureResolved() ? m_targetItem->getType() : NULL;
	}

	ModuleItem*
	getTargetItem() {
		ASSERT(m_targetItem);
		return m_targetItem;
	}

	virtual
	bool
	generateDocumentation(
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
	);

	bool
	ensureResolved() {
		return
			m_targetItem ? true :
			m_resolveError ? err::fail(m_resolveError) :
			resolve();
	}

protected:
	bool
	resolve();

	bool
	resolveImpl();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
bool
Alias::resolve() {
	bool result = resolveImpl();
	if (!result)
		m_resolveError = err::getLastError();

	return result;
}

//..............................................................................

} // namespace ct
} // namespace jnc
