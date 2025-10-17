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

#include "pch.h"
#include "jnc_ct_Decl.h"
#include "jnc_ct_DeclTypeCalc.h"
#include "jnc_ct_TemplateType.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

const char*
getTypeModifierString(TypeModifier modifier) {
	static const char* stringTable[] = {
		"unsigned",     // TypeModifier_Unsigned    = 0x00000001,
		"bigendian",    // TypeModifier_BigEndian   = 0x00000002,
		"const",        // TypeModifier_Const       = 0x00000004,
		"readonly",     // TypeModifier_ReadOnly    = 0x00000008,
		"volatile",     // TypeModifier_Volatile    = 0x00000010,
		"weak",         // TypeModifier_Weak        = 0x00000020,
		"thin",         // TypeModifier_Thin        = 0x00000040,
		"safe",         // TypeModifier_Safe        = 0x00000080,
		"cdecl",        // TypeModifier_Cdecl       = 0x00000100,
		"stdcall",      // TypeModifier_Stdcall     = 0x00000200,
		"array",        // TypeModifier_Array       = 0x00000400,
		"function",     // TypeModifier_Function    = 0x00000800,
		"property",     // TypeModifier_Property    = 0x00001000,
		"bindable",     // TypeModifier_Bindable    = 0x00002000,
		"autoget",      // TypeModifier_AutoGet     = 0x00004000,
		"indexed",      // TypeModifier_Indexed     = 0x00008000,
		"multicast",    // TypeModifier_Multicast   = 0x00010000,
		"event",        // TypeModifier_Event       = 0x00020000,
		"<unused>",     // TypeModifier_Unused      = 0x00040000,
		"reactor",      // TypeModifier_Reactor     = 0x00080000,
		"thiscall",     // TypeModifier_Thiscall    = 0x00100000,
		"jnccall",      // TypeModifier_Jnccall     = 0x00200000,
		"unsafe",       // TypeModifier_Unsafe      = 0x00400000,
		"errorcode",    // TypeModifier_ErrorCode   = 0x00800000,
		"cmut",         // TypeModifier_CMut        = 0x01000000,
		"async",        // TypeModifier_Async       = 0x02000000,
	};

	size_t i = sl::getLoBitIdx32(modifier);
	return i < countof(stringTable) ?
		stringTable[i] :
		"undefined-type-modifier";
}

sl::StringRef
getTypeModifierString(uint_t modifiers) {
	if (!modifiers)
		return sl::StringRef();

	TypeModifier modifier = getFirstFlag<TypeModifier>(modifiers);
	sl::StringRef string0 = getTypeModifierString(modifier);

	modifiers &= ~modifier;
	if (!modifiers)
		return string0;

	sl::String string = string0;
	while (modifiers) {
		modifier = getFirstFlag<TypeModifier>(modifiers);
		string += ' ';
		string += getTypeModifierString(modifier);
		modifiers &= ~modifier;
	}

	return string;
}

//..............................................................................

bool
TypeModifiers::addTypeModifier(TypeModifier modifier) {
	static
	uint_t
	antiModifierTable[] = {
		0,                              // TypeModifier_Unsigned   = 0x00000001,
		0,                              // TypeModifier_BigEndian  = 0x00000002,
		TypeModifierMaskKind_Const,     // TypeModifier_Const      = 0x00000004,
		TypeModifierMaskKind_Const,     // TypeModifier_ReadOnly   = 0x00000008,
		0,                              // TypeModifier_Volatile   = 0x00000010,
		TypeModifierMaskKind_PtrKind,   // TypeModifier_Weak       = 0x00000020,
		TypeModifierMaskKind_PtrKind,   // TypeModifier_Thin       = 0x00000040,
		TypeModifier_Unsafe,            // TypeModifier_Safe       = 0x00000080,
		TypeModifierMaskKind_CallConv,  // TypeModifier_Cdecl      = 0x00000100,
		TypeModifierMaskKind_CallConv,  // TypeModifier_Stdcall    = 0x00000200,
		TypeModifierMaskKind_TypeKind,  // TypeModifier_Array      = 0x00000400,
		TypeModifierMaskKind_TypeKind & // TypeModifier_Function   = 0x00000800,
			~TypeModifier_Function,
		TypeModifierMaskKind_TypeKind,  // TypeModifier_Property   = 0x00001000,
		0,                              // TypeModifier_Bindable   = 0x00002000,
		TypeModifier_Indexed,           // TypeModifier_AutoGet    = 0x00004000,
		TypeModifier_AutoGet,           // TypeModifier_Indexed    = 0x00008000,
		TypeModifierMaskKind_TypeKind,  // TypeModifier_Multicast  = 0x00010000,
		TypeModifierMaskKind_Event,     // TypeModifier_Event      = 0x00020000,
		TypeModifierMaskKind_Event,     // TypeModifier_DualEvent  = 0x00040000,
		TypeModifierMaskKind_TypeKind,  // TypeModifier_Reactor    = 0x00080000,
		TypeModifierMaskKind_CallConv,  // TypeModifier_Thiscall   = 0x00100000,
		TypeModifierMaskKind_CallConv,  // TypeModifier_Jnccall    = 0x00200000,
		TypeModifier_Safe,              // TypeModifier_Unsafe     = 0x00400000,
		0,                              // TypeModifier_ErrorCode  = 0x00800000,
		TypeModifierMaskKind_Const,     // TypeModifier_CMut       = 0x01000000,
		0,                              // TypeModifier_Async      = 0x02000000,
	};

	// check duplicates

	if (m_typeModifiers & modifier) {
		err::setFormatStringError("type modifier '%s' used more than once", getTypeModifierString(modifier));
		return false;
	}

	size_t i = sl::getLoBitIdx32(modifier);
	if (i >= countof(antiModifierTable)) {
		m_typeModifiers |= modifier;
		return true; // allow adding new modifiers without changing table
	}

	// check anti-modifiers

	uint_t antiModifiers = m_typeModifiers & antiModifierTable[i];
	if (antiModifiers) {
		TypeModifier antiModifier = getFirstFlag<TypeModifier>(antiModifiers);
		err::setFormatStringError(
			"type modifiers '%s' and '%s' cannot be used together",
			getTypeModifierString(antiModifier),
			getTypeModifierString(modifier)
		);

		return false;
	}

	m_typeModifiers |= modifier;
	return true;
}

int
TypeModifiers::clearTypeModifiers(int modifierMask) {
	uint_t typeModifiers = m_typeModifiers & modifierMask;
	m_typeModifiers &= ~modifierMask;
	return typeModifiers;
}

bool
TypeModifiers::checkAntiTypeModifiers(int modifierMask) {
	uint_t modifiers = m_typeModifiers;

	modifiers &= modifierMask;
	if (!modifiers)
		return true;

	TypeModifier firstModifier = getFirstFlag<TypeModifier>(modifiers);
	modifiers &= ~firstModifier;
	if (!modifiers)
		return true;

	// more than one

	TypeModifier secondModifier = getFirstFlag<TypeModifier>(modifiers);
	err::setFormatStringError(
		"type modifiers '%s' and '%s' cannot be used together",
		getTypeModifierString(firstModifier),
		getTypeModifierString(secondModifier)
	);

	return false;
}

//..............................................................................

bool
TypeSpecifier::setType(Type* type) {
	if (m_type) {
		err::setFormatStringError(
			"more than one type specifiers ('%s' and '%s')",
			m_type->getTypeString().sz(),
			type->getTypeString().sz()
		);

		return false;
	}

	m_type = type;
	return true;
}

//..............................................................................

const char*
getPostDeclaratorModifierString(PostDeclaratorModifier modifier) {
	static const char* stringTable[] = {
		"const",    // PostDeclaratorModifier_Const  = 0x01,
	};

	size_t i = sl::getLoBitIdx8((uint8_t)modifier);
	return i < countof(stringTable) ?
		stringTable[i] :
		"undefined-post-declarator-modifier";
}

sl::StringRef
getPostDeclaratorModifierString(uint_t modifiers) {
	if (!modifiers)
		return sl::String();

	PostDeclaratorModifier modifier = getFirstFlag<PostDeclaratorModifier>(modifiers);
	sl::StringRef string0 = getPostDeclaratorModifierString(modifier);
	modifiers &= ~modifier;
	if (!modifiers)
		return string0;

	sl::String string = string0;
	while (modifiers) {
		modifier = getFirstFlag<PostDeclaratorModifier>(modifiers);
		string += ' ';
		string += getPostDeclaratorModifierString(modifier);
		modifiers &= ~modifier;
	}

	return string;
}

//..............................................................................

bool
DeclFunctionSuffix::addFunctionTypeFlag(FunctionTypeFlag flag) {
	if (m_functionTypeFlags & flag) {
		err::setFormatStringError("function modifier '%s' used more than once", getFunctionTypeFlagString(flag));
		return false;
	}

	m_functionTypeFlags |= flag;
	return true;
}

//..............................................................................

Declarator::Declarator() {
	m_declaratorKind = DeclaratorKind_Undefined;
	m_functionKind = FunctionKind_Undefined;
	m_unOpKind = UnOpKind_Undefined;
	m_binOpKind = BinOpKind_Undefined;
	m_castOpType = NULL;
	m_bitCount = 0;
	m_postDeclaratorModifiers = 0;
	m_baseType = NULL;
	m_attributeBlock = NULL;
	m_doxyBlock = NULL;
}

void
Declarator::setTypeSpecifier(
	TypeSpecifier* typeSpecifier,
	Module* module
) {
	if (!typeSpecifier) {
		m_baseType = module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		return;
	}

	takeOverTypeModifiers(typeSpecifier);

	m_baseType = typeSpecifier->getType();
	if (!m_baseType) {
		m_baseType = (m_typeModifiers & (TypeModifier_Unsigned | TypeModifier_BigEndian)) ?
			module->m_typeMgr.getPrimitiveType(TypeKind_Int) :
			module->m_typeMgr.getPrimitiveType(TypeKind_Void);
	}
}

bool
Declarator::addName(sl::String name) {
	if (m_functionKind && m_functionKind != FunctionKind_Normal) {
		err::setFormatStringError("cannot further qualify '%s' declarator", getFunctionKindString(m_functionKind));
		return false;
	}

	m_declaratorKind = DeclaratorKind_Name;
	m_functionKind = FunctionKind_Normal;
	m_name.addName(name);
	return true;
}

bool
Declarator::addUnnamedMethod(FunctionKind functionKind) {
	if (m_functionKind && m_functionKind != FunctionKind_Normal) {
		err::setFormatStringError("cannot further qualify '%s' declarator", getFunctionKindString(m_functionKind));
		return false;
	}

	m_declaratorKind = DeclaratorKind_UnnamedMethod;
	m_functionKind = functionKind;
	return true;
}

bool
Declarator::addCastOperator(Type* type) {
	m_declaratorKind = DeclaratorKind_UnnamedMethod;
	m_functionKind = FunctionKind_CastOperator;
	m_castOpType = type;
	return false;
}

bool
Declarator::addUnaryBinaryOperator(
	UnOpKind unOpKind,
	BinOpKind binOpKind
) {
	if (m_functionKind && m_functionKind != FunctionKind_Normal) {
		err::setFormatStringError("cannot further qualify '%s' declarator", getFunctionKindString(m_functionKind));
		return false;
	}

	if (binOpKind == BinOpKind_Assign) {
		err::setError("assignment operator could not be overloaded");
		return false;
	}

	m_declaratorKind = DeclaratorKind_UnaryBinaryOperator;
	m_functionKind = FunctionKind_UnaryOperator; // temp; will be adjusted later in CParser::DeclareFunction
	m_unOpKind = unOpKind;
	m_binOpKind = binOpKind;
	return true;
}

bool
Declarator::setPostDeclaratorModifier(PostDeclaratorModifier modifier) {
	if (m_postDeclaratorModifiers & modifier) {
		err::setFormatStringError("type modifier '%s' used more than once", getPostDeclaratorModifierString(modifier));
		return false;
	}

	m_postDeclaratorModifiers |= modifier;
	return true;
}

void
Declarator::addPointerPrefix(uint_t modifiers) {
	DeclPointerPrefix* prefix = new DeclPointerPrefix;
	prefix->takeOverTypeModifiers(this);
	prefix->m_typeModifiers |= modifiers;
	m_pointerPrefixList.insertTail(prefix);
}

DeclArraySuffix*
Declarator::addArraySuffix(sl::List<Token>* elementCountInitializer) {
	DeclArraySuffix* suffix = new DeclArraySuffix;
	suffix->m_declarator = this;
	sl::takeOver(&suffix->m_elementCountInitializer, elementCountInitializer);
	m_suffixList.insertTail(suffix);
	return suffix;
}

DeclArraySuffix*
Declarator::addArraySuffix(size_t elementCount) {
	DeclArraySuffix* suffix = new DeclArraySuffix;
	suffix->m_declarator = this;
	suffix->m_elementCount = elementCount;
	m_suffixList.insertTail(suffix);
	return suffix;
}

DeclFunctionSuffix*
Declarator::addFunctionSuffix() {
	DeclFunctionSuffix* suffix = new DeclFunctionSuffix;
	suffix->m_declarator = this;
	m_suffixList.insertTail(suffix);
	return suffix;
}

DeclFunctionSuffix*
Declarator::addGetterSuffix() {
	DeclFunctionSuffix* suffix = new DeclFunctionSuffix;
	suffix->m_suffixKind = DeclSuffixKind_Getter;
	suffix->m_declarator = this;
	m_suffixList.insertHead(suffix);
	return suffix;
}

bool
Declarator::addBitFieldSuffix(size_t bitCount) {
	if (m_bitCount || !m_suffixList.isEmpty() || !m_pointerPrefixList.isEmpty()) {
		err::setError("bit field can only be applied to integer type");
		return false;
	}

	m_bitCount = bitCount;
	return true;
}

Type*
Declarator::calcTypeImpl(
	Value* elementCountValue,
	uint_t* flags
) {
	DeclTypeCalc typeCalc;
	return typeCalc.calcType(this, elementCountValue, flags);
}

//..............................................................................

} // namespace ct
} // namespace jnc
