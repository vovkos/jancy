// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_PropertyVerifier.h"

namespace jnc {
namespace ct {

//.............................................................................

// we need to keep namespaces for property templates cause other module items 
// could potentially have pointers to them via m_pParentNamespace

class PropertyTemplate:	
	public ModuleItem,
	public Namespace
{
	friend class FunctionMgr;
	friend class Parser;

protected:
	FunctionType* m_getterType;
	FunctionTypeOverload m_setterType;
	PropertyVerifier m_verifier;
	uint_t m_typeFlags; // before the type is calculated

public:
	PropertyTemplate ();

	FunctionType* 
	getGetterType ()
	{
		return m_getterType;
	}

	FunctionTypeOverload*
	getSetterType ()
	{
		return &m_setterType;
	}

	bool
	addMethod (
		FunctionKind functionKind,
		FunctionType* functionType
		);

	PropertyType*
	calcType ();
};

//.............................................................................

} // namespace ct
} // namespace jnc
