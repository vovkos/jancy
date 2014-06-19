#include "pch.h"
#include "jnc_Decl.h"
#include "jnc_DeclTypeCalc.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

void
CTypeModifiers::Clear ()
{
	m_TypeModifiers = 0;
}

void
CTypeModifiers::TakeOver (CTypeModifiers* pSrc)
{
	m_TypeModifiers = pSrc->m_TypeModifiers;
	pSrc->Clear ();
}

bool
CTypeModifiers::SetTypeModifier (ETypeModifier Modifier)
{
	static
	uint_t
	AntiModifierTable [] = 
	{		
		0,                          // ETypeModifier_Unsigned         = 0x00000001,
		0,                          // ETypeModifier_BigEndian        = 0x00000002,
		ETypeModifierMask_Const,    // ETypeModifier_Const            = 0x00000004,
		ETypeModifierMask_Const,    // ETypeModifier_DConst           = 0x00000008,
		0,                          // ETypeModifier_Volatile         = 0x00000010,
		ETypeModifierMask_PtrKind,  // ETypeModifier_Weak             = 0x00000020,
		ETypeModifierMask_PtrKind,  // ETypeModifier_Thin             = 0x00000040,
		0,                          // ETypeModifier_Unused           = 0x00000080,
		ETypeModifierMask_CallConv, // ETypeModifier_Cdecl            = 0x00000100,
		ETypeModifierMask_CallConv, // ETypeModifier_Stdcall          = 0x00000200,
		ETypeModifierMask_TypeKind, // ETypeModifier_Array            = 0x00000400,
		ETypeModifierMask_TypeKind, // ETypeModifier_Function         = 0x00000800,
		ETypeModifierMask_TypeKind, // ETypeModifier_Property         = 0x00001000,
		0,                          // ETypeModifier_Bindable         = 0x00002000,
		ETypeModifier_Indexed,      // ETypeModifier_AutoGet          = 0x00004000,
		ETypeModifier_AutoGet,      // ETypeModifier_Indexed          = 0x00008000,
		ETypeModifierMask_TypeKind, // ETypeModifier_Multicast        = 0x00010000,
		ETypeModifierMask_Event,    // ETypeModifier_Event            = 0x00020000,
		ETypeModifierMask_Event,    // ETypeModifier_DEvent           = 0x00040000,
		ETypeModifierMask_TypeKind, // ETypeModifier_Reactor          = 0x00080000,
	};

	// check duplicates

	if (m_TypeModifiers & Modifier)
	{
		err::SetFormatStringError ("type modifier '%s' used more than once", GetTypeModifierString (Modifier));
		return false;
	}

	size_t i = rtl::GetLoBitIdx32 (Modifier);
	if (i >= countof (AntiModifierTable))
	{
		m_TypeModifiers |= Modifier;
		return true; // allow adding new modifiers without changing table
	}

	// check anti-modifiers

	if (m_TypeModifiers & AntiModifierTable [i])
	{
		ETypeModifier Modifier2 = GetFirstTypeModifier (m_TypeModifiers);
		err::SetFormatStringError (
			"type modifiers '%s' and '%s' cannot be used together",
			GetTypeModifierString (Modifier2),
			GetTypeModifierString (Modifier)
			);

		return false;
	}

	m_TypeModifiers |= Modifier;
	return true;
}

int
CTypeModifiers::ClearTypeModifiers (int ModifierMask)
{
	uint_t TypeModifiers = m_TypeModifiers & ModifierMask;
	m_TypeModifiers &= ~ModifierMask;
	return TypeModifiers;
}

bool
CTypeModifiers::CheckAntiTypeModifiers (int ModifierMask)
{
	uint_t Modifiers = m_TypeModifiers;

	Modifiers &= ModifierMask;
	if (!Modifiers)
		return true;

	ETypeModifier FirstModifier = GetFirstTypeModifier (Modifiers);
	Modifiers &= ~FirstModifier;
	if (!Modifiers)
		return true;

	// more than one

	ETypeModifier SecondModifier = GetFirstTypeModifier (Modifiers);
	err::SetFormatStringError (
		"type modifiers '%s' and '%s' cannot be used together",
		GetTypeModifierString (FirstModifier),
		GetTypeModifierString (SecondModifier)
		);

	return false;
}

//.............................................................................

bool
CTypeSpecifier::SetType (CType* pType)
{
	if (m_pType)
	{
		err::SetFormatStringError (
			"more than one type specifiers ('%s' and '%s')", 
			m_pType->GetTypeString ().cc (), // thanks a lot gcc
			pType->GetTypeString ().cc ()
			);

		return false;
	}

	m_pType = pType;
	return true;
}

//.............................................................................

const char* 
GetPostDeclaratorModifierString (EPostDeclaratorModifier Modifier)
{
	static const char* StringTable [] = 
	{
		"const",    // EPostDeclaratorModifier_Const  = 0x01,
	};

	size_t i  = rtl::GetLoBitIdx32 (Modifier);
	return i < countof (StringTable) ? 
		StringTable [i] : 
		"undefined-post-declarator-modifier";
}

rtl::CString
GetPostDeclaratorModifierString (uint_t Modifiers)
{
	if (!Modifiers)
		return rtl::CString ();

	EPostDeclaratorModifier Modifier = GetFirstPostDeclaratorModifier (Modifiers);
	rtl::CString String = GetPostDeclaratorModifierString (Modifier);
	Modifiers &= ~Modifier;

	while (Modifiers)
	{
		Modifier = GetFirstPostDeclaratorModifier (Modifiers);

		String += ' ';
		String += GetPostDeclaratorModifierString (Modifier);

		Modifiers &= ~Modifier;
	}

	return String;
}

//.............................................................................

CDeclarator::CDeclarator ()
{
	m_DeclaratorKind = EDeclarator_Undefined;
	m_FunctionKind = EFunction_Undefined;
	m_UnOpKind = EUnOp_Undefined;
	m_BinOpKind = EBinOp_Undefined;
	m_pCastOpType = NULL;
	m_BitCount = 0;
	m_PostDeclaratorModifiers = 0;
	m_pBaseType = NULL;
}

void
CDeclarator::SetTypeSpecifier (CTypeSpecifier* pTypeSpecifier)
{
	CModule* pModule = GetCurrentThreadModule ();
	ASSERT (pModule);

	if (!pTypeSpecifier)
	{
		m_pBaseType = pModule->m_TypeMgr.GetPrimitiveType (EType_Void);
	}
	else
	{
		TakeOver (pTypeSpecifier);

		m_pBaseType = pTypeSpecifier->GetType ();	
		if (!m_pBaseType)
		{
			m_pBaseType = (m_TypeModifiers & ETypeModifier_Unsigned) ? 
				pModule->m_TypeMgr.GetPrimitiveType (EType_Int) : 
				pModule->m_TypeMgr.GetPrimitiveType (EType_Void);
		}
	}
}

bool
CDeclarator::AddName (rtl::CString Name)
{
	if (m_FunctionKind && m_FunctionKind != EFunction_Named)
	{
		err::SetFormatStringError ("cannot further qualify '%s' declarator", GetFunctionKindString (m_FunctionKind));
		return false;
	}

	m_DeclaratorKind = EDeclarator_Name;
	m_FunctionKind = EFunction_Named;
	m_Name.AddName (Name);
	return true;
}

bool
CDeclarator::AddUnnamedMethod (EFunction FunctionKind)
{
	if (m_FunctionKind && m_FunctionKind != EFunction_Named)
	{
		err::SetFormatStringError ("cannot further qualify '%s' declarator", GetFunctionKindString (m_FunctionKind));
		return false;
	}

	m_DeclaratorKind = EDeclarator_UnnamedMethod;
	m_FunctionKind = FunctionKind;
	return true;
}

bool
CDeclarator::AddCastOperator (CType* pType)
{
	m_DeclaratorKind = EDeclarator_CastOperator;
	m_FunctionKind = EFunction_CastOperator;
	m_pCastOpType = pType;
	return false;
}

bool
CDeclarator::AddUnaryBinaryOperator (
	EUnOp UnOpKind,
	EBinOp BinOpKind
	)
{
	if (m_FunctionKind && m_FunctionKind != EFunction_Named)
	{
		err::SetFormatStringError ("cannot further qualify '%s' declarator", GetFunctionKindString (m_FunctionKind));
		return false;
	}

	if (BinOpKind == EBinOp_Assign)
	{
		err::SetFormatStringError ("assignment operator could not be overloaded");
		return false;
	}

	m_DeclaratorKind = EDeclarator_UnaryBinaryOperator;
	m_FunctionKind = EFunction_UnaryOperator; // temp; will be adjusted later in CParser::DeclareFunction
	m_UnOpKind = UnOpKind;
	m_BinOpKind = BinOpKind;
	return true;
}

bool
CDeclarator::AddOperatorNew ()
{
	if (m_FunctionKind && m_FunctionKind != EFunction_Named)
	{
		err::SetFormatStringError ("cannot further qualify '%s' declarator", GetFunctionKindString (m_FunctionKind));
		return false;
	}

	m_DeclaratorKind = EDeclarator_OperatorNew;
	m_FunctionKind = EFunction_OperatorNew;
	return true;
}

bool
CDeclarator::SetPostDeclaratorModifier (EPostDeclaratorModifier Modifier)
{
	if (m_PostDeclaratorModifiers & Modifier)
	{
		err::SetFormatStringError ("type modifier '%s' used more than once", GetPostDeclaratorModifierString (Modifier));
		return false;
	}

	m_PostDeclaratorModifiers |= Modifier;
	return true;
}

void
CDeclarator::AddPointerPrefix ()
{
	CDeclPointerPrefix* pPrefix = AXL_MEM_NEW (CDeclPointerPrefix);
	pPrefix->TakeOver (this);
	m_PointerPrefixList.InsertTail (pPrefix);
}

CDeclArraySuffix*
CDeclarator::AddArraySuffix (rtl::CBoxListT <CToken>* pElementCountInitializer)
{
	CDeclArraySuffix* pSuffix = AXL_MEM_NEW (CDeclArraySuffix);
	pSuffix->m_ElementCountInitializer.TakeOver (pElementCountInitializer);
	m_SuffixList.InsertTail (pSuffix);
	return pSuffix;
}

CDeclArraySuffix*
CDeclarator::AddArraySuffix (size_t ElementCount)
{
	CDeclArraySuffix* pSuffix = AXL_MEM_NEW (CDeclArraySuffix);
	pSuffix->m_ElementCount = ElementCount;
	m_SuffixList.InsertTail (pSuffix);
	return pSuffix;
}

CDeclFunctionSuffix*
CDeclarator::AddFunctionSuffix ()
{
	CDeclFunctionSuffix* pSuffix = AXL_MEM_NEW (CDeclFunctionSuffix);
	m_SuffixList.InsertTail (pSuffix);
	return pSuffix;
}

CDeclThrowSuffix*
CDeclarator::AddThrowSuffix (rtl::CBoxListT <CToken>* pThrowCondition)
{
	CDeclThrowSuffix* pSuffix = AXL_MEM_NEW (CDeclThrowSuffix);
	if (pThrowCondition)
		pSuffix->m_ThrowCondition.TakeOver (pThrowCondition);
	m_SuffixList.InsertTail (pSuffix);
	return pSuffix;
}

bool
CDeclarator::AddBitFieldSuffix (size_t BitCount)
{
	if (m_BitCount || !m_SuffixList.IsEmpty () || !m_PointerPrefixList.IsEmpty ())
	{
		err::SetFormatStringError ("bit field can only be applied to integer type");
		return false;
	}

	m_BitCount = BitCount;
	return true;
}

CType*
CDeclarator::CalcTypeImpl (
	CValue* pElementCountValue,
	uint_t* pFlags
	)
{
	CDeclTypeCalc TypeCalc;
	return TypeCalc.CalcType (this, pElementCountValue, pFlags);
}

//.............................................................................

} // namespace jnc {
