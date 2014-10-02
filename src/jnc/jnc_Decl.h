// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Type.h"
#include "jnc_Function.h"
#include "jnc_UnOp.h"
#include "jnc_BinOp.h"

namespace jnc {

//.............................................................................

class TypeModifiers
{
protected:
	uint_t m_typeModifiers;

public:
	TypeModifiers ()
	{
		m_typeModifiers = 0;
	}

	void
	clear ();

	void
	takeOver (TypeModifiers* src);

	int 
	getTypeModifiers ()
	{
		return m_typeModifiers;
	}

	bool
	setTypeModifier (TypeModifierKind modifier);

	int
	clearTypeModifiers (int modifierMask);

protected:
	bool
	checkAntiTypeModifiers (int modifierMask);
};

//.............................................................................

class TypeSpecifier: public TypeModifiers
{
protected:
	Type* m_type;

public:
	TypeSpecifier ()
	{
		m_type = NULL;
	}

	Type* 
	getType ()
	{
		return m_type;
	}

	bool
	setType (Type* type);
};

//.............................................................................

class DeclPointerPrefix:
	public TypeModifiers,
	public rtl::ListLink
{
	friend class Declarator;
};

//.............................................................................

enum DeclSuffixKind
{
	DeclSuffixKind_Undefined = 0,
	DeclSuffixKind_Array,
	DeclSuffixKind_Function,
	DeclSuffixKind_Throw,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DeclSuffix: public rtl::ListLink
{
	friend class Declarator;

protected:
	DeclSuffixKind m_suffixKind;

public:
	DeclSuffix ()
	{
		m_suffixKind = DeclSuffixKind_Undefined;
	}

	virtual
	~DeclSuffix ()
	{
	}

	DeclSuffixKind
	getSuffixKind ()
	{
		return m_suffixKind;
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DeclArraySuffix: public DeclSuffix
{
	friend class Declarator;

protected:
	size_t m_elementCount;
	rtl::BoxList <Token> m_elementCountInitializer;

public:
	DeclArraySuffix ()
	{
		m_suffixKind = DeclSuffixKind_Array;
		m_elementCount = 0;
	}

	size_t
	getElementCount ()
	{
		return m_elementCount;
	}

	rtl::BoxList <Token>*
	getElementCountInitializer ()
	{
		return &m_elementCountInitializer;
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DeclFunctionSuffix: public DeclSuffix
{
	friend class Declarator;
	friend class Parser;

protected:
	rtl::Array <FunctionArg*> m_argArray;
	uint_t m_functionTypeFlags;

public:
	DeclFunctionSuffix ()
	{
		m_suffixKind = DeclSuffixKind_Function;
		m_functionTypeFlags = 0;
	}

	rtl::Array <FunctionArg*>
	getArgArray ()
	{
		return m_argArray;
	}

	int 
	getFunctionTypeFlags ()
	{
		return m_functionTypeFlags;
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DeclThrowSuffix: public DeclSuffix
{
	friend class Declarator;
	friend class Parser;

protected:
	rtl::BoxList <Token> m_throwCondition;

public:
	DeclThrowSuffix ()
	{
		m_suffixKind = DeclSuffixKind_Throw;
	}

	rtl::BoxList <Token>*
	getThrowCondition ()
	{
		return &m_throwCondition;
	}
};

//.............................................................................

enum PostDeclaratorModifierKind
{
	PostDeclaratorModifierKind_Const = 0x01,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
PostDeclaratorModifierKind
getFirstPostDeclaratorModifier (uint_t modifiers)
{
	return (PostDeclaratorModifierKind) (1 << rtl::getLoBitIdx (modifiers));
}

const char* 
getPostDeclaratorModifierString (PostDeclaratorModifierKind modifier);

rtl::String
getPostDeclaratorModifierString (uint_t modifiers);

inline
const char* 
getFirstPostDeclaratorModifierString (uint_t modifiers)
{
	return getPostDeclaratorModifierString (getFirstPostDeclaratorModifier (modifiers));
}

//.............................................................................

enum DeclaratorKind
{
	DeclaratorKind_Undefined = 0,
	DeclaratorKind_Name,
	DeclaratorKind_UnnamedMethod,
	DeclaratorKind_UnaryBinaryOperator,
	DeclaratorKind_CastOperator,
	DeclaratorKind_OperatorNew,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Declarator: public TypeModifiers
{
	friend class Parser;
	friend class DeclTypeCalc;

protected:
	DeclaratorKind m_declaratorKind;
	FunctionKind m_functionKind;
	UnOpKind m_unOpKind;
	BinOpKind m_binOpKind;
	Type* m_castOpType;
	QualifiedName m_name;
	Token::Pos m_pos;
	size_t m_bitCount;
	uint_t m_postDeclaratorModifiers;
	Type* m_baseType;

	rtl::StdList <DeclPointerPrefix> m_pointerPrefixList;
	rtl::StdList <DeclSuffix> m_suffixList;
	rtl::BoxList <Token> m_constructor;
	rtl::BoxList <Token> m_initializer;

public:
	Declarator ();

	bool
	isSimple ()
	{
		return m_declaratorKind == DeclaratorKind_Name && m_name.isSimple ();
	}

	bool
	isQualified ()
	{
		return m_declaratorKind == DeclaratorKind_Name ? !m_name.isSimple () : !m_name.isEmpty ();
	}

	DeclaratorKind
	getDeclaratorKind ()
	{
		return m_declaratorKind;
	}

	FunctionKind
	getFunctionKind ()
	{
		return m_functionKind;
	}

	UnOpKind
	getUnOpKind ()
	{
		return m_unOpKind;
	}

	BinOpKind
	getBinOpKind ()
	{
		return m_binOpKind;
	}

	Type*
	getCastOpType ()
	{
		return m_castOpType;
	}

	void
	setTypeSpecifier (TypeSpecifier* typeSpecifier);

	const QualifiedName*
	getName ()
	{
		return &m_name;
	}

	const Token::Pos&
	getPos ()
	{
		return m_pos;
	}

	size_t 
	getBitCount ()
	{
		return m_bitCount;
	}

	int 
	getPostDeclaratorModifiers ()
	{
		return m_postDeclaratorModifiers;
	}

	Type* 
	getBaseType ()
	{
		return m_baseType;
	}

	rtl::ConstList <DeclPointerPrefix> 
	getPointerPrefixList ()
	{
		return m_pointerPrefixList;
	}

	rtl::ConstList <DeclSuffix> 
	getSuffixList ()
	{
		return m_suffixList;
	}

	bool
	setPostDeclaratorModifier (PostDeclaratorModifierKind modifier);

	DeclFunctionSuffix*
	getFunctionSuffix ()
	{
		rtl::Iterator <DeclSuffix> suffix = m_suffixList.getHead ();
		return suffix && suffix->getSuffixKind () == DeclSuffixKind_Function ? (DeclFunctionSuffix*) *suffix : NULL;
	}

	Type*
	calcType ()
	{
		return calcTypeImpl (NULL, NULL);
	}

	Type*
	calcType (Value* elementCountValue)
	{
		return calcTypeImpl (elementCountValue, NULL);
	}

	Type*
	calcType (uint_t* flags)
	{
		return calcTypeImpl (NULL, flags);
	}

	bool
	addName (rtl::String name);

	bool
	addUnnamedMethod (FunctionKind functionKind);

	bool
	addCastOperator (Type* type);

	bool
	addUnaryBinaryOperator (
		UnOpKind unOpKind, 
		BinOpKind binOpKind
		);

	bool
	addOperatorNew ();

	void
	addPointerPrefix ();

	DeclArraySuffix*
	addArraySuffix (rtl::BoxList <Token>* elementCountInitializer);

	DeclArraySuffix*
	addArraySuffix (size_t elementCount);

	DeclFunctionSuffix*
	addFunctionSuffix ();

	DeclThrowSuffix*
	addThrowSuffix (rtl::BoxList <Token>* throwCondition = NULL);
	
	bool
	addBitFieldSuffix (size_t bitCount);

	DeclThrowSuffix*
	getThrowSuffix ()
	{
		return !m_suffixList.isEmpty () && m_suffixList.getTail ()->getSuffixKind () == DeclSuffixKind_Throw ? 
			(DeclThrowSuffix*) *m_suffixList.getTail () : 
			NULL;
	}

	void
	deleteSuffix (DeclSuffix* suffix)
	{
		m_suffixList.erase (suffix);
	}

protected:
	Type*
	calcTypeImpl (
		Value* elementCountValue,
		uint_t* flags
		);
};

//.............................................................................

} // namespace jnc {
