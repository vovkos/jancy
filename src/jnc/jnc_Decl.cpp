#include "pch.h"
#include "jnc_Decl.h"
#include "jnc_DeclTypeCalc.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

void
TypeModifiers::clear ()
{
	m_typeModifiers = 0;
}

void
TypeModifiers::takeOver (TypeModifiers* src)
{
	m_typeModifiers = src->m_typeModifiers;
	src->clear ();
}

bool
TypeModifiers::setTypeModifier (TypeModifier modifier)
{
	static
	uint_t
	antiModifierTable [] = 
	{		
		0,                          // ETypeModifier_Unsigned         = 0x00000001,
		0,                          // ETypeModifier_BigEndian        = 0x00000002,
		TypeModifierMaskKind_Const,    // ETypeModifier_Const            = 0x00000004,
		TypeModifierMaskKind_Const,    // ETypeModifier_DConst           = 0x00000008,
		0,                          // ETypeModifier_Volatile         = 0x00000010,
		TypeModifierMaskKind_PtrKind,  // ETypeModifier_Weak             = 0x00000020,
		TypeModifierMaskKind_PtrKind,  // ETypeModifier_Thin             = 0x00000040,
		0,                          // ETypeModifier_Unused           = 0x00000080,
		TypeModifierMaskKind_CallConv, // ETypeModifier_Cdecl            = 0x00000100,
		TypeModifierMaskKind_CallConv, // ETypeModifier_Stdcall          = 0x00000200,
		TypeModifierMaskKind_TypeKind, // ETypeModifier_Array            = 0x00000400,
		TypeModifierMaskKind_TypeKind, // ETypeModifier_Function         = 0x00000800,
		TypeModifierMaskKind_TypeKind, // ETypeModifier_Property         = 0x00001000,
		0,                          // ETypeModifier_Bindable         = 0x00002000,
		TypeModifier_Indexed,      // ETypeModifier_AutoGet          = 0x00004000,
		TypeModifier_AutoGet,      // ETypeModifier_Indexed          = 0x00008000,
		TypeModifierMaskKind_TypeKind, // ETypeModifier_Multicast        = 0x00010000,
		TypeModifierMaskKind_Event,    // ETypeModifier_Event            = 0x00020000,
		TypeModifierMaskKind_Event,    // ETypeModifier_DEvent           = 0x00040000,
		TypeModifierMaskKind_TypeKind, // ETypeModifier_Reactor          = 0x00080000,
		TypeModifierMaskKind_CallConv, // ETypeModifier_Thiscall         = 0x00100000,
		TypeModifierMaskKind_CallConv, // ETypeModifier_Jnccall          = 0x00200000,
	};

	// check duplicates

	if (m_typeModifiers & modifier)
	{
		err::setFormatStringError ("type modifier '%s' used more than once", getTypeModifierString (modifier));
		return false;
	}

	size_t i = rtl::getLoBitIdx32 (modifier);
	if (i >= countof (antiModifierTable))
	{
		m_typeModifiers |= modifier;
		return true; // allow adding new modifiers without changing table
	}

	// check anti-modifiers

	if (m_typeModifiers & antiModifierTable [i])
	{
		TypeModifier modifier2 = getFirstTypeModifier (m_typeModifiers);
		err::setFormatStringError (
			"type modifiers '%s' and '%s' cannot be used together",
			getTypeModifierString (modifier2),
			getTypeModifierString (modifier)
			);

		return false;
	}

	m_typeModifiers |= modifier;
	return true;
}

int
TypeModifiers::clearTypeModifiers (int modifierMask)
{
	uint_t typeModifiers = m_typeModifiers & modifierMask;
	m_typeModifiers &= ~modifierMask;
	return typeModifiers;
}

bool
TypeModifiers::checkAntiTypeModifiers (int modifierMask)
{
	uint_t modifiers = m_typeModifiers;

	modifiers &= modifierMask;
	if (!modifiers)
		return true;

	TypeModifier firstModifier = getFirstTypeModifier (modifiers);
	modifiers &= ~firstModifier;
	if (!modifiers)
		return true;

	// more than one

	TypeModifier secondModifier = getFirstTypeModifier (modifiers);
	err::setFormatStringError (
		"type modifiers '%s' and '%s' cannot be used together",
		getTypeModifierString (firstModifier),
		getTypeModifierString (secondModifier)
		);

	return false;
}

//.............................................................................

bool
TypeSpecifier::setType (Type* type)
{
	if (m_type)
	{
		err::setFormatStringError (
			"more than one type specifiers ('%s' and '%s')", 
			m_type->getTypeString ().cc (), // thanks a lot gcc
			type->getTypeString ().cc ()
			);

		return false;
	}

	m_type = type;
	return true;
}

//.............................................................................

const char* 
getPostDeclaratorModifierString (PostDeclaratorModifier modifier)
{
	static const char* stringTable [] = 
	{
		"const",    // EPostDeclaratorModifier_Const  = 0x01,
	};

	size_t i  = rtl::getLoBitIdx32 (modifier);
	return i < countof (stringTable) ? 
		stringTable [i] : 
		"undefined-post-declarator-modifier";
}

rtl::String
getPostDeclaratorModifierString (uint_t modifiers)
{
	if (!modifiers)
		return rtl::String ();

	PostDeclaratorModifier modifier = getFirstPostDeclaratorModifier (modifiers);
	rtl::String string = getPostDeclaratorModifierString (modifier);
	modifiers &= ~modifier;

	while (modifiers)
	{
		modifier = getFirstPostDeclaratorModifier (modifiers);

		string += ' ';
		string += getPostDeclaratorModifierString (modifier);

		modifiers &= ~modifier;
	}

	return string;
}

//.............................................................................

Declarator::Declarator ()
{
	m_declaratorKind = DeclaratorKind_Undefined;
	m_functionKind = FunctionKind_Undefined;
	m_unOpKind = UnOpKind_Undefined;
	m_binOpKind = BinOpKind_Undefined;
	m_castOpType = NULL;
	m_bitCount = 0;
	m_postDeclaratorModifiers = 0;
	m_baseType = NULL;
}

void
Declarator::setTypeSpecifier (TypeSpecifier* typeSpecifier)
{
	Module* module = getCurrentThreadModule ();
	ASSERT (module);

	if (!typeSpecifier)
	{
		m_baseType = module->m_typeMgr.getPrimitiveType (TypeKind_Void);
	}
	else
	{
		takeOver (typeSpecifier);

		m_baseType = typeSpecifier->getType ();	
		if (!m_baseType)
		{
			m_baseType = (m_typeModifiers & TypeModifier_Unsigned) ? 
				module->m_typeMgr.getPrimitiveType (TypeKind_Int) : 
				module->m_typeMgr.getPrimitiveType (TypeKind_Void);
		}
	}
}

bool
Declarator::addName (rtl::String name)
{
	if (m_functionKind && m_functionKind != FunctionKind_Named)
	{
		err::setFormatStringError ("cannot further qualify '%s' declarator", getFunctionKindString (m_functionKind));
		return false;
	}

	m_declaratorKind = DeclaratorKind_Name;
	m_functionKind = FunctionKind_Named;
	m_name.addName (name);
	return true;
}

bool
Declarator::addUnnamedMethod (FunctionKind functionKind)
{
	if (m_functionKind && m_functionKind != FunctionKind_Named)
	{
		err::setFormatStringError ("cannot further qualify '%s' declarator", getFunctionKindString (m_functionKind));
		return false;
	}

	m_declaratorKind = DeclaratorKind_UnnamedMethod;
	m_functionKind = functionKind;
	return true;
}

bool
Declarator::addCastOperator (Type* type)
{
	m_declaratorKind = DeclaratorKind_CastOperator;
	m_functionKind = FunctionKind_CastOperator;
	m_castOpType = type;
	return false;
}

bool
Declarator::addUnaryBinaryOperator (
	UnOpKind unOpKind,
	BinOpKind binOpKind
	)
{
	if (m_functionKind && m_functionKind != FunctionKind_Named)
	{
		err::setFormatStringError ("cannot further qualify '%s' declarator", getFunctionKindString (m_functionKind));
		return false;
	}

	if (binOpKind == BinOpKind_Assign)
	{
		err::setFormatStringError ("assignment operator could not be overloaded");
		return false;
	}

	m_declaratorKind = DeclaratorKind_UnaryBinaryOperator;
	m_functionKind = FunctionKind_UnaryOperator; // temp; will be adjusted later in CParser::DeclareFunction
	m_unOpKind = unOpKind;
	m_binOpKind = binOpKind;
	return true;
}

bool
Declarator::addOperatorNew ()
{
	if (m_functionKind && m_functionKind != FunctionKind_Named)
	{
		err::setFormatStringError ("cannot further qualify '%s' declarator", getFunctionKindString (m_functionKind));
		return false;
	}

	m_declaratorKind = DeclaratorKind_OperatorNew;
	m_functionKind = FunctionKind_OperatorNew;
	return true;
}

bool
Declarator::setPostDeclaratorModifier (PostDeclaratorModifier modifier)
{
	if (m_postDeclaratorModifiers & modifier)
	{
		err::setFormatStringError ("type modifier '%s' used more than once", getPostDeclaratorModifierString (modifier));
		return false;
	}

	m_postDeclaratorModifiers |= modifier;
	return true;
}

void
Declarator::addPointerPrefix ()
{
	DeclPointerPrefix* prefix = AXL_MEM_NEW (DeclPointerPrefix);
	prefix->takeOver (this);
	m_pointerPrefixList.insertTail (prefix);
}

DeclArraySuffix*
Declarator::addArraySuffix (rtl::BoxList <Token>* elementCountInitializer)
{
	DeclArraySuffix* suffix = AXL_MEM_NEW (DeclArraySuffix);
	suffix->m_elementCountInitializer.takeOver (elementCountInitializer);
	m_suffixList.insertTail (suffix);
	return suffix;
}

DeclArraySuffix*
Declarator::addArraySuffix (size_t elementCount)
{
	DeclArraySuffix* suffix = AXL_MEM_NEW (DeclArraySuffix);
	suffix->m_elementCount = elementCount;
	m_suffixList.insertTail (suffix);
	return suffix;
}

DeclFunctionSuffix*
Declarator::addFunctionSuffix ()
{
	DeclFunctionSuffix* suffix = AXL_MEM_NEW (DeclFunctionSuffix);
	m_suffixList.insertTail (suffix);
	return suffix;
}

DeclThrowSuffix*
Declarator::addThrowSuffix (rtl::BoxList <Token>* throwCondition)
{
	DeclThrowSuffix* suffix = AXL_MEM_NEW (DeclThrowSuffix);
	if (throwCondition)
		suffix->m_throwCondition.takeOver (throwCondition);
	m_suffixList.insertTail (suffix);
	return suffix;
}

bool
Declarator::addBitFieldSuffix (size_t bitCount)
{
	if (m_bitCount || !m_suffixList.isEmpty () || !m_pointerPrefixList.isEmpty ())
	{
		err::setFormatStringError ("bit field can only be applied to integer type");
		return false;
	}

	m_bitCount = bitCount;
	return true;
}

Type*
Declarator::calcTypeImpl (
	Value* elementCountValue,
	uint_t* flags
	)
{
	DeclTypeCalc typeCalc;
	return typeCalc.calcType (this, elementCountValue, flags);
}

//.............................................................................

} // namespace jnc {
