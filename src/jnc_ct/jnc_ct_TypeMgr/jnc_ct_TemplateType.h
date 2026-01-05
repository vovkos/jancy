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

class TemplateDeclType;

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
	public ModuleItemPos {
	friend class TypeMgr;
	friend class Parser;

protected:
	QualifiedName m_name;

public:
	TemplateTypeName() {
		m_typeKind = TypeKind_TemplateTypeName;
	}

	const QualifiedName&
	getName() {
		return m_name;
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
	);

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

class TemplateDeclType: public TemplateType {
	friend class TypeMgr;
	friend class Parser;

protected:
	Declarator m_declarator;
	Type* m_deductionType; // includes TemplateType-s

public:
	TemplateDeclType() {
		m_typeKind = TypeKind_TemplateDecl;
		m_deductionType = NULL;
	}

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
	prepareSignature();

	virtual
	sl::StringRef
	createItemString(size_t index) {
		return getDeductionType()->getItemString(index);
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
