// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ClassType.h"

namespace jnc {

//.............................................................................

struct OpaqueClassTypeInfo
{
	size_t m_size;
	bool m_isNonCreatable;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class OpaqueClassTypeDb
{
public:
	virtual 
	const OpaqueClassTypeInfo*
	getClassTypeInfo (ClassType* type) = 0;
};

//.............................................................................

class StdOpaqueClassTypeDb: public OpaqueClassTypeDb
{
protected:
	rtl::BoxList <OpaqueClassTypeInfo> m_typeInfoList;
	rtl::StringHashTableMap <OpaqueClassTypeInfo*> m_typeInfoMap;
	rtl::StringCache m_nameCache;

public:
	virtual 
	const OpaqueClassTypeInfo*
	getClassTypeInfo (ClassType* type)
	{
		rtl::StringHashTableMapIterator <OpaqueClassTypeInfo*> it = m_typeInfoMap.find (type->getQualifiedName ());
		return it ? it->m_value : NULL;
	}

	void
	setOpaqueClassTypeInfo (
		const char* name,
		const OpaqueClassTypeInfo* typeInfo
		);

	void
	setOpaqueClassTypeInfo (
		ClassType* type,
		const OpaqueClassTypeInfo* typeInfo
		)
	{
		setOpaqueClassTypeInfo (type->getQualifiedName (), typeInfo);
	}
};

//.............................................................................

} // namespace jnc {
