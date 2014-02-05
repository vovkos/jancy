#include "pch.h"
#include "jnc_AttributeBlock.h"

namespace jnc {

//.............................................................................

CAttribute*
CAttributeBlock::CreateAttribute (
	const rtl::CString& Name,
	CValue* pValue
	)
{
	rtl::CStringHashTableMapIteratorT <CAttribute*> It = m_AttributeMap.Goto (Name);
	if (It->m_Value)
	{
		err::SetFormatStringError ("redefinition of attribute '%s'", Name.cc ()); // thanks a lot gcc
		return NULL;
	}

	CAttribute* pAttribute = AXL_MEM_NEW (CAttribute);
	pAttribute->m_pModule = m_pModule;
	pAttribute->m_Name = Name;
	pAttribute->m_pValue = pValue;
	m_AttributeList.InsertTail (pAttribute);
	It->m_Value = pAttribute;
	return pAttribute;
}

//.............................................................................

} // namespace jnc {
