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
UnOp_Addr::getResultType (const Value& opValue)
{
	Type* opType = opValue.getType ();
	TypeKind opTypeKind = opType->getTypeKind ();
	switch (opTypeKind)
	{
	case TypeKind_DataRef:
		return ((DataPtrType*) opType)->getTargetType ()->getDataPtrType (
			((DataPtrType*) opType)->getAnchorNamespace (),
			TypeKind_DataPtr,
			((DataPtrType*) opType)->getPtrTypeKind (),
			opType->getFlags ()
			);

	case TypeKind_ClassRef:
		return ((ClassPtrType*) opType)->getTargetType ()->getClassPtrType (
			((ClassPtrType*) opType)->getAnchorNamespace (),
			TypeKind_ClassPtr,
			((ClassPtrType*) opType)->getPtrTypeKind (),
			opType->getFlags ()
			);

	case TypeKind_FunctionRef:
		return ((FunctionPtrType*) opType)->getTargetType ()->getFunctionPtrType (
			TypeKind_FunctionPtr,
			((FunctionPtrType*) opType)->getPtrTypeKind (),
			opType->getFlags ()
			);

	case TypeKind_PropertyRef:
		return ((PropertyPtrType*) opType)->getTargetType ()->getPropertyPtrType (
			((PropertyPtrType*) opType)->getAnchorNamespace (),
			TypeKind_PropertyPtr,
			((PropertyPtrType*) opType)->getPtrTypeKind (),
			opType->getFlags ()
			);

	default:
		err::setFormatStringError ("can only apply unary '&' to a reference");
		return NULL;
	}
}

bool
UnOp_Addr::op (
	const Value& opValue,
	Value* resultValue
	)
{
	Type* resultType = getResultType (opValue);
	if (!resultType)
		return false;

	resultValue->overrideType (opValue, resultType);
	return true;
}

//..............................................................................

Type*
UnOp_Indir::getResultType (const Value& opValue)
{
	Type* opType = opValue.getType ();
	TypeKind opTypeKind = opType->getTypeKind ();
	switch (opTypeKind)
	{
	case TypeKind_DataPtr:
		return ((DataPtrType*) opType)->getTargetType ()->getDataPtrType (
			((DataPtrType*) opType)->getAnchorNamespace (),
			TypeKind_DataRef,
			((DataPtrType*) opType)->getPtrTypeKind (),
			opType->getFlags ()
			);

	case TypeKind_ClassPtr:
		return ((ClassPtrType*) opType)->getTargetType ()->getClassPtrType (
			((ClassPtrType*) opType)->getAnchorNamespace (),
			TypeKind_ClassRef,
			((ClassPtrType*) opType)->getPtrTypeKind (),
			opType->getFlags ()
			);

	case TypeKind_FunctionPtr:
		return ((FunctionPtrType*) opType)->getTargetType ()->getFunctionPtrType (
			TypeKind_FunctionRef,
			((FunctionPtrType*) opType)->getPtrTypeKind (),
			opType->getFlags ()
			);

	case TypeKind_PropertyPtr:
		return ((PropertyPtrType*) opType)->getTargetType ()->getPropertyPtrType (
			((PropertyPtrType*) opType)->getAnchorNamespace (),
			TypeKind_PropertyRef,
			((PropertyPtrType*) opType)->getPtrTypeKind (),
			opType->getFlags ()
			);

	default:
		err::setFormatStringError ("can only apply unary '*' to a pointer");
		return NULL;
	}
}

bool
UnOp_Indir::op (
	const Value& opValue,
	Value* resultValue
	)
{
	Type* resultType = getResultType (opValue);
	if (!resultType)
		return false;

	resultValue->overrideType (opValue, resultType);
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
