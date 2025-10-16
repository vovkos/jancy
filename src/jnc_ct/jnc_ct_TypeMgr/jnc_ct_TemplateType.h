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

#include "jnc_ct_Type.h"
#include "jnc_ct_Decl.h"

namespace jnc {
namespace ct {

//..............................................................................

class TemplateArgType: public Type {
	friend class TypeMgr;
	friend class Parser;

protected:
	sl::StringRef m_name;
	size_t m_index;

public:
	TemplateArgType() {
		m_typeKind = TypeKind_TemplateArg;
		m_index = 0;
	}

	const sl::StringRef&
	getName() {
		return m_name;
	}

	size_t
	getIndex() {
		return m_index;
	}

	static
	sl::String
	createSignature(
		const sl::StringRef& name,
		size_t index
	) {
		return sl::formatString("X%s$%d", name.sz(), index);
	}

protected:
	virtual
	void
	prepareSignature() {
		m_signature = createSignature(m_name, m_index);
		m_flags |= TypeFlag_SignatureReady;
	}

	virtual
	void
	prepareTypeString() {
		getTypeStringTuple()->m_typeStringPrefix = m_name;
	}
};

//..............................................................................

class TemplateDeclType: public Type {
	friend class TypeMgr;
	friend class Parser;

protected:
	size_t m_id;
	Declarator m_declarator;

public:
	TemplateDeclType() {
		m_typeKind = TypeKind_TemplateDecl;
		m_id = 0;
	}

	Declarator*
	getDeclarator() {
		return &m_declarator;
	}

	Type*
	instantiate(const sl::ArrayRef<Type*>& argArray);

protected:
	virtual
	void
	prepareSignature() {
		m_signature = sl::formatString("D%d", m_id);
		m_flags |= TypeFlag_SignatureReady;
	}

	virtual
	void
	prepareTypeString() {
		getTypeStringTuple()->m_typeStringPrefix = sl::formatString("template-decl-%d", m_id);
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
