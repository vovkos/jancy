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

class TemplateType : public Type {
};

//..............................................................................

class TemplateArgType: public TemplateType {
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
		return sl::formatString("XA%d%s", index, name.sz());
	}

	virtual
	bool
	deduceTemplateArgs(
		sl::Array<Type*>* templateArgTypeArray,
		Type* referenceType
	);

protected:
	virtual
	bool
	resolveImports() {
		return true;
	}

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

	Type*
	selectTemplateArg(
		Type* type1,
		Type* type2
	);
};

//..............................................................................

template <
	typename T,
	TypeKind typeKind,
	uint16_t signaturePrefix
>
class TemplateModType: public ModType<
	TemplateType,
	T,
	typeKind,
	signaturePrefix
> {
protected:
	virtual
	bool
	resolveImports() {
		return true;
	}

	void
	setTemplateArgDeductionError(Type* argValueType) {
		err::setFormatStringError(
			"incompatible types while deducing template argument '%s': '%s' vs '%s'",
			m_baseType->getName().sz(),
			argValueType->getTypeString().sz(),
			getTypeString().sz()
		);
	}
};

//..............................................................................

class TemplatePtrType: public TemplateModType<
	TemplateArgType,
	TypeKind_TemplatePtr,
	'PX'
> {
public:
	virtual
	bool
	deduceTemplateArgs(
		sl::Array<Type*>* templateArgTypeArray,
		Type* referenceType
	);

protected:
	virtual
	void
	prepareTypeString();
};

//..............................................................................

class TemplateIntModType: public TemplateModType<
	TemplateArgType,
	TypeKind_TemplateIntMod,
	'IX'
> {
public:
	virtual
	bool
	deduceTemplateArgs(
		sl::Array<Type*>* templateArgTypeArray,
		Type* referenceType
	);

protected:
	virtual
	void
	prepareTypeString();
};

//..............................................................................

class TemplateDeclType: public TemplateType {
	friend class TypeMgr;
	friend class Parser;

protected:
	Declarator m_declarator;
	Type* m_deductionType; // includes TemplateType-s
	size_t m_id;

public:
	TemplateDeclType();

	Declarator*
	getDeclarator() {
		return &m_declarator;
	}

	Type*
	getDeductionType() {
		return m_deductionType;
	}

	Type*
	createDeductionType() {
		ASSERT(!m_deductionType);
		return m_deductionType = instantiate(*(sl::ArrayRef<Type*>*)&m_declarator.getTemplateArgArray());
	}

	Type*
	instantiate(const sl::ArrayRef<Type*>& argArray);

protected:
	virtual
	void
	prepareSignature() {
		m_signature = sl::formatString("XD%d", m_id);
		m_flags |= TypeFlag_SignatureReady;
	}

	virtual
	void
	prepareTypeString() {
		getTypeStringTuple()->m_typeStringPrefix = sl::formatString("template-decl-%d", m_id);
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
TemplateDeclType::TemplateDeclType() {
	m_typeKind = TypeKind_TemplateDecl;
	m_deductionType = NULL;
	m_id = 0;
}

//..............................................................................

} // namespace ct
} // namespace jnc
