// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Type.h"
#include "jnc_Namespace.h"

namespace jnc {

class FunctionType;
class PropertyType;
class StructType;
class Variable;

//.............................................................................

class NamedType:
	public Type,
	public Namespace
{
	friend class Parser;

protected:
	Namespace* m_extensionNamespace;

public:
	NamedType ();

	Namespace*
	getExtensionNamespace ()
	{
		return m_extensionNamespace;
	}

	virtual
	Type*
	getThisArgType (uint_t ptrTypeFlags)
	{
		return (Type*) getDataPtrType (DataPtrTypeKind_Normal, ptrTypeFlags);
	}

	FunctionType*
	getMemberMethodType (
		FunctionType* shortType,
		uint_t thisArgTypeFlags = 0
		);

	PropertyType*
	getMemberPropertyType (PropertyType* shortType);

protected:
	virtual
	bool
	calcLayout ();

	void
	applyExtensionNamespace ();

	virtual
	ModuleItem*
	findItemTraverseImpl (
		const char* name,
		MemberCoord* coord,
		uint_t flags
		);
};
//.............................................................................

} // namespace jnc {
