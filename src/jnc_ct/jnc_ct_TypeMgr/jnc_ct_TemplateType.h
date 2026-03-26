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
#include "jnc_ct_TypeName.h"
#include "jnc_ct_Decl.h"

namespace jnc {
namespace ct {

class TemplateDeclType;
class TemplateNamespace;

//..............................................................................

class TemplateType : public Type {
	virtual
	bool
	calcLayout();
};

//..............................................................................

class TemplateArgType: public TemplateType {
	friend class TypeMgr;
	friend class Parser;

protected:
	sl::StringRef m_name;
	size_t m_index;
	TemplateDeclType* m_defaultType;

public:
	TemplateArgType();

	const sl::StringRef&
	getName() {
		return m_name;
	}

	size_t
	getIndex() {
		return m_index;
	}

	TemplateDeclType*
	getDefaultType() {
		return m_defaultType;
	}

	virtual
	bool
	deduceTemplateArgs(
		sl::Array<Type*>* templateArgTypeArray,
		Type* referenceType
	);

protected:
	virtual
	void
	prepareSignature();

	virtual
	sl::StringRef
	createItemString(size_t index);

	virtual
	bool
	resolveImports() {
		return true;
	}

	Type*
	selectTemplateArg(
		Type* type1,
		Type* type2
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
TemplateArgType::TemplateArgType() {
	m_typeKind = TypeKind_TemplateArg;
	m_index = 0;
	m_defaultType = NULL;
}

//..............................................................................

class TemplateTypeName:
	public TemplateType,
	public TypeName {
	friend class TypeMgr;
	friend class Parser;

public:
	TemplateTypeName() {
		m_typeKind = TypeKind_TemplateTypeName;
	}

	virtual
	bool
	deduceTemplateArgs(
		sl::Array<Type*>* templateArgTypeArray,
		Type* referenceType
	);

	static
	sl::String
	createSignature(
		Namespace* parentNamespace,
		const QualifiedName& name
	) {
		return TypeName::createSignature('NX', parentNamespace, name);
	}

protected:
	virtual
	sl::StringRef
	createItemString(size_t index);
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
};

//..............................................................................

class TemplatePtrType: public TemplateModType<
	TemplateType,
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
	sl::StringRef
	createItemString(size_t index);
};

//..............................................................................

class TemplateIntModType: public TemplateModType<
	TemplateType,
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
	sl::StringRef
	createItemString(size_t index);
};

//..............................................................................

class TemplateDeclType:
	public TemplateType,
	public ModuleItemContext {
	friend class TypeMgr;
	friend class Parser;

protected:
	Declarator m_declarator;
	DerivableType* m_parentType;
	Type* m_deductionType; // includes TemplateType-s

public:
	TemplateDeclType();

	Declarator*
	getDeclarator() {
		return &m_declarator;
	}

	DerivableType*
	getParentType() {
		return m_parentType;
	}

	Type*
	ensureDeductionType() {
		return m_deductionType ? m_deductionType : createDeductionType();
	}

	Type*
	instantiate(
		const sl::ArrayRef<Type*>& argArray,
		uint_t* declFlags = NULL
	);

protected:
	virtual
	void
	prepareSignature();

	virtual
	sl::StringRef
	createItemString(size_t index);

	Type*
	createDeductionType();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
TemplateDeclType::TemplateDeclType() {
	m_typeKind = TypeKind_TemplateDecl;
	m_parentType = NULL;
	m_deductionType = NULL;
}

//..............................................................................

} // namespace ct
} // namespace jnc
