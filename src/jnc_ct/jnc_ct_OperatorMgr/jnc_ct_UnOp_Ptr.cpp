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

#include "pch.h"
#include "jnc_ct_UnOp_Ptr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

Type*
UnOp_Addr::getResultType(const Value& opValue) {
	if (opValue.getValueKind() == ValueKind_Variable &&
		opValue.getVariable()->getStorageKind() == StorageKind_Tls) {
		err::setFormatStringError("cannot take address of a 'threadlocal' variable");
		return NULL;
	}

	union {
		Type* m_type;
		DataPtrType* m_dataPtrType;
		ClassPtrType* m_classPtrType;
		FunctionPtrType* m_functionPtrType;
		PropertyPtrType* m_propertyPtrType;
	} t = { opValue.getType() };

	TypeKind opTypeKind = t.m_type->getTypeKind();
	switch (opTypeKind) {
	case TypeKind_DataRef:
		return t.m_dataPtrType->getTargetType()->getDataPtrType(
			TypeKind_DataPtr,
			t.m_dataPtrType->getPtrTypeKind(),
			t.m_dataPtrType->getFlags()
		);

	case TypeKind_ClassRef:
		return t.m_classPtrType->getTargetType()->getClassPtrType(
			TypeKind_ClassPtr,
			t.m_classPtrType->getPtrTypeKind(),
			t.m_classPtrType->getFlags()
		);

	case TypeKind_FunctionRef:
		return t.m_functionPtrType->getTargetType()->getFunctionPtrType(
			TypeKind_FunctionPtr,
			t.m_functionPtrType->getPtrTypeKind(),
			t.m_functionPtrType->getFlags()
		);

	case TypeKind_PropertyRef:
		return t.m_propertyPtrType->getTargetType()->getPropertyPtrType(
			TypeKind_PropertyPtr,
			t.m_propertyPtrType->getPtrTypeKind(),
			t.m_propertyPtrType->getFlags()
		);

	default:
		err::setFormatStringError("can only apply unary '&' to a reference");
		return NULL;
	}
}

bool
UnOp_Addr::op(
	const Value& opValue,
	Value* resultValue
) {
	Type* resultType = getResultType(opValue);
	if (!resultType)
		return false;

	resultValue->overrideType(opValue, resultType);
	return true;
}

//..............................................................................

Type*
UnOp_Indir::getResultType(const Value& opValue) {
	union {
		Type* m_type;
		DataPtrType* m_dataPtrType;
		ClassPtrType* m_classPtrType;
		FunctionPtrType* m_functionPtrType;
		PropertyPtrType* m_propertyPtrType;
	} t = { opValue.getType() };

	TypeKind opTypeKind = t.m_type->getTypeKind();
	switch (opTypeKind) {
	case TypeKind_DataPtr:
		return t.m_dataPtrType->getTargetType()->getDataPtrType(
			TypeKind_DataRef,
			t.m_dataPtrType->getPtrTypeKind(),
			t.m_dataPtrType->getFlags()
		);

	case TypeKind_ClassPtr:
		return t.m_classPtrType->getTargetType()->getClassPtrType(
			TypeKind_ClassRef,
			t.m_classPtrType->getPtrTypeKind(),
			t.m_classPtrType->getFlags()
		);

	case TypeKind_FunctionPtr:
		return t.m_functionPtrType->getTargetType()->getFunctionPtrType(
			TypeKind_FunctionRef,
			t.m_functionPtrType->getPtrTypeKind(),
			t.m_functionPtrType->getFlags()
		);

	case TypeKind_PropertyPtr:
		return t.m_propertyPtrType->getTargetType()->getPropertyPtrType(
			TypeKind_PropertyRef,
			t.m_propertyPtrType->getPtrTypeKind(),
			t.m_propertyPtrType->getFlags()
		);

	default:
		err::setFormatStringError("can only apply unary '*' to a pointer");
		return NULL;
	}
}

bool
UnOp_Indir::op(
	const Value& opValue,
	Value* resultValue
) {
	Type* resultType = getResultType(opValue);
	if (!resultType)
		return false;

	ASSERT(resultType->getTypeKindFlags() & TypeKindFlag_Ref);
	resultValue->overrideType(opValue, resultType);
	return true;
}

//..............................................................................

bool
UnOp_Ptr::op(
	const Value& opValue,
	Value* resultValue
) {
	return isClassPtrType(opValue.getType(), ClassTypeKind_DynamicLib) ?
		m_module->m_operatorMgr.memberOperator(opValue, "lib", resultValue) :
		UnOp_Indir::op(opValue, resultValue);
}

//..............................................................................

} // namespace ct
} // namespace jnc
