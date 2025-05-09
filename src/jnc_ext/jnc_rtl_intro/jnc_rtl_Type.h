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
class TypeBase: public ModuleItemBase<T> {
protected:
	struct Cache {
		String m_signature;
		String m_typeString;
		String m_typeStringPrefix;
		String m_typeStringSuffix;
	};

protected:
	CachePtr<Cache> m_cache;

public:
	TypeBase(T* type):
		ModuleItemBase<T>(type) {}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Type: public TypeBase<ct::Type> {
public:
	Type(ct::Type* type):
		TypeBase(type) {}

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	TypeKind
	JNC_CDECL
	getTypeKind() {
		return m_item->getTypeKind();
	}

	uint_t
	JNC_CDECL
	getTypeKindFlags() {
		return m_item->getTypeKindFlags();
	}

	size_t
	JNC_CDECL
	getSize() {
		return m_item->getSize();
	}

	size_t
	JNC_CDECL
	getAlignment() {
		return m_item->getAlignment();
	}

	static
	String
	JNC_CDECL
	getSignature(Type* self);

	static
	String
	JNC_CDECL
	getTypeString(Type* self);

	static
	String
	JNC_CDECL
	getTypeStringPrefix(Type* self);

	static
	String
	JNC_CDECL
	getTypeStringSuffix(Type* self);

	int
	JNC_CDECL
	cmp(Type* type) {
		return m_item->cmp(type->m_item);
	}

	static
	String
	JNC_CDECL
	getValueString_0(
		Type* self,
		DataPtr valuePtr,
		String formatSpec
	);

	static
	String
	JNC_CDECL
	getValueString_1(
		Type* self,
		Variant value,
		String formatSpec
	);

	ArrayType*
	JNC_CDECL
	getArrayType(size_t elementCount) {
		return (ArrayType*)rtl::getType(m_item->getArrayType(elementCount));
	}

	DataPtrType*
	JNC_CDECL
	getDataPtrType_0(
		uint_t bitOffset,
		uint_t bitCount,
		TypeKind typeKind,
		DataPtrTypeKind ptrTypeKind,
		uint_t flags
	) {
		return (DataPtrType*)rtl::getType(m_item->getDataPtrType(bitOffset, bitCount, typeKind, ptrTypeKind, flags));
	}

	DataPtrType*
	JNC_CDECL
	getDataPtrType_1(
		TypeKind typeKind,
		DataPtrTypeKind ptrTypeKind,
		uint_t flags
	) {
		return (DataPtrType*)rtl::getType(m_item->getDataPtrType(typeKind, ptrTypeKind, flags));
	}

protected:
	static
	bool
	tryGetStringableValueString(
		String* string,
		ct::DerivableType* type,
		DataPtr valuePtr
	);
};

//..............................................................................

template <typename T>
class NamedTypeBase:
	public TypeBase<T>,
	public Namespace {
public:
	NamedTypeBase(T* type):
		TypeBase<T>(type),
		Namespace(type) {}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class NamedType: public NamedTypeBase<ct::NamedType> {
public:
	NamedType(ct::NamedType* type):
		NamedTypeBase(type) {}
};

//..............................................................................

class DataPtrType: public TypeBase<ct::DataPtrType> {
public:
	DataPtrType(ct::DataPtrType* type):
		TypeBase(type) {}

	DataPtrTypeKind
	JNC_CDECL
	getPtrTypeKind() {
		return m_item->getPtrTypeKind();
	}

	Type*
	JNC_CDECL
	getTargetType() {
		return rtl::getType(m_item->getTargetType());
	}

	uint_t
	JNC_CDECL
	getBitOffset() {
		return m_item->getBitOffset();
	}

	uint_t
	JNC_CDECL
	getBitCount() {
		return m_item->getBitCount();
	}

	static
	String
	JNC_CDECL
	getTargetValueString(
		DataPtrType* self,
		DataPtr valuePtr,
		String formatSpec
	);
};

//..............................................................................

class Typedef:
	public ModuleItemBase<ct::Typedef>,
	public ModuleItemDecl {
public:
	Typedef(ct::Typedef* tdef):
		ModuleItemBase(tdef),
		ModuleItemDecl(tdef) {}

	Type*
	JNC_CDECL
	getType() {
		return rtl::getType(m_item->getType());
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
