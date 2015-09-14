#include "pch.h"
#include "jnc_Multicast.h"
#include "jnc_Runtime.h"
#include "jnc_StdLib.h"

namespace jnc {

//.............................................................................

void
StdOpaqueClassTypeDb::setOpaqueClassTypeInfo (
	const char* name,
	const OpaqueClassTypeInfo* typeInfo
	)
{
	name = m_nameCache.getString (name);
	rtl::StringHashTableMapIterator <OpaqueClassTypeInfo*> mapIt = m_typeInfoMap.visit (name);
	if (mapIt->m_value)
	{
		*mapIt->m_value = *typeInfo;
	}
	else
	{
		rtl::BoxIterator <OpaqueClassTypeInfo> listIt = m_typeInfoList.insertTail (*typeInfo);
		mapIt->m_value = listIt.p ();
	}
}

//.............................................................................

} // namespace jnc {
