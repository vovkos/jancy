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

#include "jnc_ct_DerivableType.h"
#include "jnc_RuntimeStructs.h"
#include "jnc_ClassType.h"

namespace jnc {
namespace ct {

class ClassPtrType;
struct ClassPtrTypeTuple;
class StructType;
class Function;
class Property;

//..............................................................................

class ClassType: public DerivableType
{
	friend class TypeMgr;
	friend class Parser;
	friend class Property;
	friend class StructType;
	friend class AsyncFunction;

protected:
	ClassTypeKind m_classTypeKind;

	StructType* m_vtableStructType;
	StructType* m_ifaceStructType;
	StructType* m_classStructType;

	sl::Array<BaseTypeSlot*> m_baseTypePrimeArray;
	sl::Array<StructField*> m_classMemberFieldArray;

	MarkOpaqueGcRootsFunc* m_markOpaqueGcRootsFunc;

	sl::Array<Function*> m_virtualMethodArray;
	sl::Array<Function*> m_overrideMethodArray;
	sl::Array<Property*> m_virtualPropertyArray;

	sl::Array<Function*> m_vtable;
	Variable* m_vtableVariable;

	ClassPtrTypeTuple* m_classPtrTypeTuple;

public:
	ClassType();

	ClassTypeKind
	getClassTypeKind()
	{
		return m_classTypeKind;
	}

	StructType*
	getIfaceStructType()
	{
		ASSERT(m_ifaceStructType);
		return m_ifaceStructType;
	}

	StructType*
	getClassStructType()
	{
		ASSERT(m_classStructType);
		return m_classStructType;
	}

	ClassPtrType*
	getClassPtrType(
		TypeKind typeKind,
		ClassPtrTypeKind ptrTypeKind = ClassPtrTypeKind_Normal,
		uint_t flags = 0
		);

	ClassPtrType*
	getClassPtrType(
		ClassPtrTypeKind ptrTypeKind = ClassPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return getClassPtrType(TypeKind_ClassPtr, ptrTypeKind, flags);
	}

	virtual
	Type*
	getThisArgType(uint_t ptrTypeFlags)
	{
		return (Type*)getClassPtrType(ClassPtrTypeKind_Normal, ptrTypeFlags);
	}

	MarkOpaqueGcRootsFunc*
	getMarkOpaqueGcRootsFunc()
	{
		return m_markOpaqueGcRootsFunc;
	}

	virtual
	bool
	addMethod(Function* function);

	virtual
	bool
	addProperty(Property* prop);

	bool
	hasVtable()
	{
		return !m_vtable.isEmpty();
	}

	sl::Array<BaseTypeSlot*>
	getBaseTypePrimeArray()
	{
		return m_baseTypePrimeArray;
	}

	sl::Array<StructField*>
	getClassMemberFieldArray()
	{
		return m_classMemberFieldArray;
	}

	sl::Array<Function*>
	getVirtualMethodArray()
	{
		return m_virtualMethodArray;
	}

	sl::Array<Property*>
	getVirtualPropertyArray()
	{
		return m_virtualPropertyArray;
	}

	StructType*
	getVtableStructType();

	Variable*
	getVtableVariable()
	{
		return m_vtableVariable;
	}

	virtual
	bool
	compile();

	virtual
	void
	markGcRoots(
		const void* p,
		rt::GcHeap* gcHeap
		);

protected:
	void
	markGcRootsImpl(
		IfaceHdr* iface,
		rt::GcHeap* gcHeap
		);

	virtual
	StructField*
	createFieldImpl(
		const sl::StringRef& name,
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0,
		sl::BoxList<Token>* constructor = NULL,
		sl::BoxList<Token>* initializer = NULL
		);

	virtual
	void
	prepareLlvmType();

	virtual
	void
	prepareLlvmDiType();

	virtual
	bool
	calcLayout();

	void
	addVirtualFunction(Function* function);

	bool
	overrideVirtualFunction(Function* function);

	void
	createVtableVariable();
};

//..............................................................................

} // namespace ct
} // namespace jnc
