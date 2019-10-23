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

#pragma once

#include "jnc_rtl_ModuleItem.h"
#include "jnc_rtl_Namespace.h"
#include "jnc_ct_Type.h"
#include "jnc_ct_ArrayType.h"
#include "jnc_ct_DataPtrType.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(Type)
JNC_DECLARE_OPAQUE_CLASS_TYPE(NamedType)
JNC_DECLARE_OPAQUE_CLASS_TYPE(DataPtrType)
JNC_DECLARE_OPAQUE_CLASS_TYPE(Typedef)

//..............................................................................

template <typename T>
class TypeBase: public ModuleItemBase<T>
{
protected:
	struct Cache
	{
		DataPtr m_signaturePtr;
		DataPtr m_typeStringPtr;
		DataPtr m_typeStringPrefixPtr;
		DataPtr m_typeStringSuffixPtr;
	};

protected:
	Cache* m_cache;

public:
	TypeBase(T* type):
		ModuleItemBase<T>(type)
	{
	}

	~TypeBase()
	{
		if (m_cache)
			AXL_MEM_DELETE(m_cache);
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Type: public TypeBase<ct::Type>
{
public:
	Type(ct::Type* type):
		TypeBase(type)
	{
	}

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	TypeKind
	JNC_CDECL
	getTypeKind()
	{
		return m_item->getTypeKind();
	}

	size_t
	JNC_CDECL
	getSize()
	{
		return m_item->getSize();
	}

	size_t
	JNC_CDECL
	getAlignment()
	{
		return m_item->getAlignment();
	}

	static
	DataPtr
	JNC_CDECL
	getSignature(Type* self);

	static
	DataPtr
	JNC_CDECL
	getTypeString(Type* self);

	static
	DataPtr
	JNC_CDECL
	getTypeStringPrefix(Type* self);

	static
	DataPtr
	JNC_CDECL
	getTypeStringSuffix(Type* self);

	int
	JNC_CDECL
	cmp(Type* type)
	{
		return m_item->cmp(type->m_item);
	}

	static
	DataPtr
	JNC_CDECL
	getValueString(
		Type* self,
		DataPtr valuePtr
		);

	ArrayType*
	JNC_CDECL
	getArrayType(size_t elementCount)
	{
		return (ArrayType*)rtl::getType(m_item->getArrayType(elementCount));
	}

	DataPtrType*
	JNC_CDECL
	getDataPtrType(
		TypeKind typeKind,
		DataPtrTypeKind ptrTypeKind,
		uint_t flags
		)
	{
		return (DataPtrType*)rtl::getType(m_item->getDataPtrType(typeKind, ptrTypeKind, flags));
	}

protected:
	Cache*
	getCache()
	{
		return m_cache ? m_cache : m_cache = AXL_MEM_ZERO_NEW(Cache);
	}
};

//..............................................................................

template <typename T>
class NamedTypeBase:
	public TypeBase<T>,
	public Namespace
{
public:
	NamedTypeBase(T* type):
		TypeBase<T>(type),
		Namespace(type)
	{
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class NamedType: public NamedTypeBase<ct::NamedType>
{
public:
	NamedType(ct::NamedType* type):
		NamedTypeBase(type)
	{
	}
};

//..............................................................................

class DataPtrType: public TypeBase<ct::DataPtrType>
{
public:
	DataPtrType(ct::DataPtrType* type):
		TypeBase(type)
	{
	}

	DataPtrTypeKind
	JNC_CDECL
	getPtrTypeKind()
	{
		return m_item->getPtrTypeKind();
	}

	Type*
	JNC_CDECL
	getTargetType()
	{
		return rtl::getType(m_item->getTargetType());
	}
};

//..............................................................................

class Typedef:
	public ModuleItemBase<ct::Typedef>,
	public ModuleItemDecl
{
public:
	Typedef(ct::Typedef* tdef):
		ModuleItemBase(tdef),
		ModuleItemDecl(tdef)
	{
	}

	Type*
	JNC_CDECL
	getType()
	{
		return rtl::getType(m_item->getType());
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
