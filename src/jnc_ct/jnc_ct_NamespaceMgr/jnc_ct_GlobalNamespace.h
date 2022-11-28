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

#include "jnc_ct_Namespace.h"

namespace jnc {
namespace ct {

//..............................................................................

class GlobalNamespace:
	public ModuleItem,
	public Namespace {
	friend class NamespaceMgr;

protected:
	struct ExtraBody: sl::ListLink {
		Unit* m_unit;
		const PragmaSettings* m_pragmaSettings;
		lex::LineColOffset m_pos;
		sl::StringRef m_body;
	};

protected:
	sl::List<ExtraBody> m_extraBodyList;

public:
	GlobalNamespace() {
		m_itemKind = ModuleItemKind_Namespace;
		m_namespaceKind = NamespaceKind_Global;
	}

	void
	clear() {
		Namespace::clear();
		m_extraBodyList.clear();
	}

	virtual
	sl::String
	createDoxyRefId();

	virtual
	bool
	generateDocumentation(
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
	);

	void
	addBody(
		Unit* unit,
		const PragmaSettings* pragmaSettings,
		const lex::LineColOffset& pos,
		const sl::StringRef& body
	);

protected:
	virtual
	bool
	parseBody();

	bool
	parseBodyImpl(
		Unit* unit,
		const PragmaSettings* pragmaSettings,
		const lex::LineColOffset& pos,
		const sl::StringRef& body
	);
};

//..............................................................................

} // namespace ct
} // namespace jnc
