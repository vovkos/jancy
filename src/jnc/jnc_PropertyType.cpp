#include "pch.h"
#include "jnc_PropertyType.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

const char* 
GetPropertyTypeFlagString (EPropertyTypeFlag Flag)
{
	static const char* StringTable [] = 
	{
		"const",     // EPropertyTypeFlag_Const    = 0x010000,
		"bindable",  // EPropertyTypeFlag_Bindable = 0x020000,
	};

	size_t i = rtl::GetLoBitIdx32 (Flag >> 16);

	return i < countof (StringTable) ? 
		StringTable [i] : 
		"undefined-property-type-flag";
}

rtl::CString
GetPropertyTypeFlagString (uint_t Flags)
{
	if (!Flags)
		return rtl::CString ();

	EPropertyTypeFlag Flag = GetFirstPropertyTypeFlag (Flags);
	rtl::CString String = GetPropertyTypeFlagString (Flag);
	Flags &= ~Flag;

	while (Flags)
	{
		Flag = GetFirstPropertyTypeFlag (Flags);

		String += ' ';
		String += GetPropertyTypeFlagString (Flag);

		Flags &= ~Flag;
	}

	return String;
}

uint_t
GetPropertyTypeFlagsFromModifiers (uint_t Modifiers)
{
	uint_t Flags = 0;

	if (Modifiers & ETypeModifier_Const)
		Flags |= EPropertyTypeFlag_Const;

	if (Modifiers & ETypeModifier_Bindable)
		Flags |= EPropertyTypeFlag_Bindable;

	return Flags;
}

//.............................................................................

CPropertyType::CPropertyType ()
{
	m_TypeKind = EType_Property;

	m_pGetterType = NULL;
	m_pBinderType = NULL;
	m_pStdObjectMemberPropertyType = NULL;
	m_pShortType = NULL;
	m_pVTableStructType = NULL;
	m_pPropertyPtrTypeTuple = NULL;
}

CPropertyPtrType* 
CPropertyType::GetPropertyPtrType (
	CNamespace* pNamespace,
	EType TypeKind,
	EPropertyPtrType PtrTypeKind,
	uint_t Flags
	)
{
	return m_pModule->m_TypeMgr.GetPropertyPtrType (pNamespace, this, TypeKind, PtrTypeKind, Flags);
}

CPropertyType*
CPropertyType::GetMemberPropertyType (CClassType* pClassType)
{
	return m_pModule->m_TypeMgr.GetMemberPropertyType (pClassType, this);
}

CPropertyType*
CPropertyType::GetStdObjectMemberPropertyType ()
{
	return m_pModule->m_TypeMgr.GetStdObjectMemberPropertyType (this);
}

CPropertyType*
CPropertyType::GetShortType  ()
{
	return m_pModule->m_TypeMgr.GetShortPropertyType (this);
}

CStructType*
CPropertyType::GetVTableStructType ()
{
	return m_pModule->m_TypeMgr.GetPropertyVTableStructType (this);
}

rtl::CString
CPropertyType::CreateSignature (
	CFunctionType* pGetterType,
	const CFunctionTypeOverload& SetterType,
	uint_t Flags
	)
{
	rtl::CString String = "X";
	
	if (Flags & EPropertyTypeFlag_Bindable)
		String += 'b';

	String += pGetterType->GetSignature ();

	size_t OverloadCount = SetterType.GetOverloadCount ();
	for (size_t i = 0; i < OverloadCount; i++)
	{
		CFunctionType* pOverloadType = SetterType.GetOverload (i);
		String += pOverloadType->GetSignature ();
	}

	return String;
}

rtl::CString
CPropertyType::GetTypeModifierString ()
{
	if (!m_TypeModifierString.IsEmpty ())
		return m_TypeModifierString;

	if (m_Flags & EPropertyTypeFlag_Const)
		m_TypeModifierString += "const ";

	if (m_Flags & EPropertyTypeFlag_Bindable)
		m_TypeModifierString += "bindable ";

	if (IsIndexed ())
		m_TypeModifierString += "indexed ";

	return m_TypeModifierString;
}

void
CPropertyType::PrepareTypeString ()
{
	CType* pReturnType = GetReturnType ();

	m_TypeString = pReturnType->GetTypeString ();
	m_TypeString += ' ';
	m_TypeString += GetTypeModifierString ();
	m_TypeString += "property";
	
	if (IsIndexed ())
	{
		m_TypeString += ' ';
		m_TypeString += m_pGetterType->GetArgString ();
	}
}

//.............................................................................

} // namespace jnc {
