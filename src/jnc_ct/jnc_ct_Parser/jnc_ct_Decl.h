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

#include "jnc_ct_FunctionName.h"
#include "jnc_ct_QualifiedName.h"
#include "jnc_ct_Value.h"

namespace jnc {
namespace ct {

class Declarator;
class TemplateArgType;
struct NamedImportAnchor;

//..............................................................................

enum TypeModifier {
	TypeModifier_Unsigned    = 0x00000001,
	TypeModifier_BigEndian   = 0x00000002,
	TypeModifier_Const       = 0x00000004,
	TypeModifier_ReadOnly    = 0x00000008,
	TypeModifier_Volatile    = 0x00000010,
	TypeModifier_Weak        = 0x00000020,
	TypeModifier_Thin        = 0x00000040,
	TypeModifier_Safe        = 0x00000080,
	TypeModifier_Cdecl       = 0x00000100,
	TypeModifier_Stdcall     = 0x00000200,
	TypeModifier_Array       = 0x00000400,
	TypeModifier_Function    = 0x00000800,
	TypeModifier_Property    = 0x00001000,
	TypeModifier_Bindable    = 0x00002000,
	TypeModifier_AutoGet     = 0x00004000,
	TypeModifier_Indexed     = 0x00008000,
	TypeModifier_Multicast   = 0x00010000,
	TypeModifier_Event       = 0x00020000,
	TypeModifier_DualEvent   = 0x00040000,
	TypeModifier_Reactor     = 0x00080000,
	TypeModifier_Thiscall    = 0x00100000,
	TypeModifier_Jnccall     = 0x00200000,
	TypeModifier_Unsafe      = 0x00400000,
	TypeModifier_ErrorCode   = 0x00800000,
	TypeModifier_CMut        = 0x01000000,
	TypeModifier_Async       = 0x02000000,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum TypeModifierMaskKind {
	TypeModifierMaskKind_Integer =
		TypeModifier_Unsigned,

	TypeModifierMaskKind_CallConv =
		TypeModifier_Cdecl |
		TypeModifier_Stdcall |
		TypeModifier_Thiscall |
		TypeModifier_Jnccall,

	TypeModifierMaskKind_Function =
		TypeModifier_Function |
		TypeModifier_ErrorCode |
		TypeModifier_Unsafe |
		TypeModifier_Async |
		TypeModifierMaskKind_CallConv,

	TypeModifierMaskKind_Property =
		TypeModifier_Property |
		TypeModifier_ErrorCode |
		TypeModifier_Const |
		TypeModifier_ReadOnly |
		TypeModifier_CMut |
		TypeModifier_AutoGet |
		TypeModifier_Bindable |
		TypeModifier_Indexed |
		TypeModifier_BigEndian |
		TypeModifier_Volatile |
		TypeModifierMaskKind_CallConv,

	TypeModifierMaskKind_DataPtr =
		TypeModifier_BigEndian |
		TypeModifier_Safe |
		TypeModifier_Const |
		TypeModifier_ReadOnly |
		TypeModifier_CMut |
		TypeModifier_Volatile |
		TypeModifier_Thin,

	TypeModifierMaskKind_ClassPtr =
		TypeModifier_Safe |
		TypeModifier_Const |
		TypeModifier_ReadOnly |
		TypeModifier_CMut |
		TypeModifier_Volatile |
		TypeModifier_Event |
		TypeModifier_DualEvent |
		TypeModifier_Weak,

	TypeModifierMaskKind_FunctionPtr =
		TypeModifier_Safe |
		TypeModifier_Weak |
		TypeModifier_Thin,

	TypeModifierMaskKind_PropertyPtr =
		TypeModifier_Safe |
		TypeModifier_Weak |
		TypeModifier_Thin,

	TypeModifierMaskKind_ImportPtr =
		TypeModifierMaskKind_DataPtr |
		TypeModifierMaskKind_ClassPtr |
		TypeModifierMaskKind_FunctionPtr |
		TypeModifierMaskKind_PropertyPtr,

	TypeModifierMaskKind_TemplatePtr = TypeModifierMaskKind_ImportPtr,

	TypeModifierMaskKind_PtrKind =
		TypeModifier_Weak |
		TypeModifier_Thin,

	TypeModifierMaskKind_TypeKind =
		TypeModifier_Function |
		TypeModifier_Property |
		TypeModifier_Multicast |
		TypeModifier_Reactor,

	TypeModifierMaskKind_Const =
		TypeModifier_Const |
		TypeModifier_ReadOnly |
		TypeModifier_CMut |
		TypeModifier_Event |
		TypeModifier_DualEvent,

	TypeModifierMaskKind_Event =
		TypeModifierMaskKind_Const |
		TypeModifierMaskKind_TypeKind,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getTypeModifierString(TypeModifier modifier);

sl::StringRef
getTypeModifierString(uint_t modifiers);

//..............................................................................

class TypeModifiers {
protected:
	uint_t m_typeModifiers;

public:
	TypeModifiers() {
		m_typeModifiers = 0;
	}

	int
	getTypeModifiers() const {
		return m_typeModifiers;
	}

	bool
	addTypeModifier(TypeModifier modifier);

	void
	clearTypeModifiers() {
		m_typeModifiers = 0;
	}

	int
	clearTypeModifiers(int modifierMask);

	void
	takeOverTypeModifiers(TypeModifiers* modifiers) {
		m_typeModifiers = modifiers->m_typeModifiers;
		modifiers->m_typeModifiers = 0;
	}

protected:
	bool
	checkAntiTypeModifiers(int modifierMask);
};

//..............................................................................

class TypeSpecifier: public TypeModifiers {
protected:
	Type* m_type;

public:
	TypeSpecifier() {
		m_type = NULL;
	}

	Type*
	getType() {
		return m_type;
	}

	bool
	setType(Type* type);
};

//..............................................................................

class DeclPointerPrefix:
	public TypeModifiers,
	public sl::ListLink {
	friend class Declarator;
};

//..............................................................................

enum DeclSuffixKind {
	DeclSuffixKind_Undefined = 0,
	DeclSuffixKind_Array,
	DeclSuffixKind_Function,
	DeclSuffixKind_Getter,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DeclSuffix: public sl::ListLink {
	friend class Declarator;
	friend class DeclTypeCalc;

protected:
	DeclSuffixKind m_suffixKind;
	Declarator* m_declarator;

public:
	DeclSuffix() {
		m_suffixKind = DeclSuffixKind_Undefined;
		m_declarator = NULL;
	}

	virtual
	~DeclSuffix() {}

	DeclSuffixKind
	getSuffixKind() const {
		return m_suffixKind;
	}

	Declarator*
	getDeclarator() const {
		return m_declarator;
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DeclArraySuffix: public DeclSuffix {
	friend class Declarator;

protected:
	size_t m_elementCount;
	sl::List<Token> m_elementCountInitializer;

public:
	DeclArraySuffix() {
		m_suffixKind = DeclSuffixKind_Array;
		m_elementCount = 0;
	}

	size_t
	getElementCount() {
		return m_elementCount;
	}

	sl::List<Token>*
	getElementCountInitializer() {
		return &m_elementCountInitializer;
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DeclFunctionSuffix: public DeclSuffix {
	friend class Declarator;
	friend class DeclTypeCalc;
	friend class Parser;

protected:
	sl::Array<FunctionArg*> m_argArray;
	uint_t m_functionTypeFlags;

public:
	DeclFunctionSuffix() {
		m_suffixKind = DeclSuffixKind_Function;
		m_functionTypeFlags = 0;
	}

	const sl::Array<FunctionArg*>&
	getArgArray() {
		return m_argArray;
	}

	int
	getFunctionTypeFlags() {
		return m_functionTypeFlags;
	}

	bool
	addFunctionTypeFlag(FunctionTypeFlag flag);
};

//..............................................................................

enum DeclaratorKind {
	DeclaratorKind_Undefined = 0,
	DeclaratorKind_Name,
	DeclaratorKind_UnnamedMethod,
	DeclaratorKind_UnaryBinaryOperator,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Declarator: public TypeModifiers {
	friend class DeclTypeCalc;
	friend class Parser;
	friend class NamespaceMgr;
	friend class TypeMgr;

protected:
	DeclaratorKind m_declaratorKind;
	FunctionKind m_functionKind;
	UnOpKind m_unOpKind;
	BinOpKind m_binOpKind;
	Type* m_castOpType;
	QualifiedName m_name;
	lex::LineCol m_pos;
	size_t m_bitCount;
	Type* m_baseType;
	AttributeBlock* m_attributeBlock;
	dox::Block* m_doxyBlock;
	sl::List<DeclPointerPrefix> m_pointerPrefixList;
	sl::List<DeclSuffix> m_suffixList;
	sl::List<Token> m_constructor;
	sl::List<Token> m_initializer;

public:
	Declarator();

	bool
	isSimple() {
		return m_declaratorKind == DeclaratorKind_Name && m_name.isSimple();
	}

	bool
	isQualified() {
		return m_declaratorKind == DeclaratorKind_Name ? m_name.isQualified() : !m_name.isEmpty();
	}

	DeclaratorKind
	getDeclaratorKind() {
		return m_declaratorKind;
	}

	FunctionKind
	getFunctionKind() {
		return m_functionKind;
	}

	UnOpKind
	getUnOpKind() {
		return m_unOpKind;
	}

	BinOpKind
	getBinOpKind() {
		return m_binOpKind;
	}

	Type*
	getCastOpType() {
		return m_castOpType;
	}

	void
	setTypeSpecifier(
		TypeSpecifier* typeSpecifier,
		Module* module
	);

	const QualifiedName&
	getName() {
		return m_name;
	}

	const sl::StringRef&
	getSimpleName() {
		ASSERT(isSimple());
		return m_name.getFirstAtom().m_name;
	}

	const sl::StringRef&
	getShortName() {
		return m_name.getShortName();
	}

	const lex::LineCol&
	getPos() {
		return m_pos;
	}

	size_t
	getBitCount() {
		return m_bitCount;
	}

	Type*
	getBaseType() {
		return m_baseType;
	}

	AttributeBlock*
	getAttributeBlock() {
		return m_attributeBlock;
	}

	dox::Block*
	getDoxyBlock() {
		return m_doxyBlock;
	}

	const sl::Array<TemplateArgType*>&
	getTemplateArgArray() {
		ASSERT(m_name.getLastAtom()->m_atomKind == QualifiedNameAtomKind_TemplateDeclSuffix);
		return m_name.getLastAtom()->m_templateDeclArgArray;
	}

	const sl::List<DeclPointerPrefix>&
	getPointerPrefixList() {
		return m_pointerPrefixList;
	}

	const sl::List<DeclSuffix>&
	getSuffixList() {
		return m_suffixList;
	}

	DeclFunctionSuffix*
	getFunctionSuffix() {
		sl::Iterator<DeclSuffix> suffix = m_suffixList.getHead();
		return suffix && suffix->getSuffixKind() == DeclSuffixKind_Function ? (DeclFunctionSuffix*)*suffix : NULL;
	}

	Type*
	calcType() {
		return calcTypeImpl(NULL, NULL);
	}

	Type*
	calcType(Value* elementCountValue) {
		return calcTypeImpl(elementCountValue, NULL);
	}

	Type*
	calcType(uint_t* flags) {
		return calcTypeImpl(NULL, flags);
	}

	bool
	addName(sl::String name);

	bool
	addUnnamedMethod(FunctionKind functionKind);

	bool
	addCastOperator(Type* type);

	bool
	addUnaryBinaryOperator(
		UnOpKind unOpKind,
		BinOpKind binOpKind
	);

	void
	addPointerPrefix(uint_t modifiers = 0);

	DeclArraySuffix*
	addArraySuffix(sl::List<Token>* elementCountInitializer);

	DeclArraySuffix*
	addArraySuffix(size_t elementCount);

	DeclFunctionSuffix*
	addFunctionSuffix();

	DeclFunctionSuffix*
	addGetterSuffix();

	bool
	addBitFieldSuffix(size_t bitCount);

	void
	deleteSuffix(DeclSuffix* suffix) {
		m_suffixList.erase(suffix);
	}

protected:
	Type*
	calcTypeImpl(
		Value* elementCountValue,
		uint_t* flags
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Declarator::Declarator() {
	m_declaratorKind = DeclaratorKind_Undefined;
	m_functionKind = FunctionKind_Undefined;
	m_unOpKind = UnOpKind_Undefined;
	m_binOpKind = BinOpKind_Undefined;
	m_castOpType = NULL;
	m_bitCount = 0;
	m_baseType = NULL;
	m_attributeBlock = NULL;
	m_doxyBlock = NULL;
}

//..............................................................................

} // namespace ct
} // namespace jnc
