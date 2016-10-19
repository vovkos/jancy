#include "pch.h"
#include "jnc_ct_CastOp_Array.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

CastKind
Cast_Array::getCastKind (
	const Value& opValue,
	Type* type
	)
{
	ASSERT (type->getTypeKind () == TypeKind_Array);
	ArrayType* dstArrayType = (ArrayType*) type;
	Type* dstElementType = dstArrayType->getElementType ();
	size_t dstElementCount = dstArrayType->getElementCount ();

	Type* opType = opValue.getType ();

	if (opType->getTypeKind () == TypeKind_DataRef)
		opType = ((DataPtrType*) opType)->getTargetType ();

	if (opType->getTypeKind () != TypeKind_Array)
		return CastKind_None;

	ArrayType* srcArrayType = (ArrayType*) opType;
	Type* srcElementType = srcArrayType->getElementType ();
	size_t srcElementCount = srcArrayType->getElementCount ();

	return
		dstElementType->cmp (srcElementType) == 0 ||
		(dstElementType->getTypeKindFlags () & TypeKindFlag_Integer) &&
		(srcElementType->getTypeKindFlags () & TypeKindFlag_Integer) &&
		dstElementType->getSize () == srcElementType->getSize() ?
		srcElementCount <= dstElementCount ?
		CastKind_Implicit :
		CastKind_Explicit :
		CastKind_None;
}

bool
Cast_Array::constCast (
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	ASSERT (type->getTypeKind () == TypeKind_Array);
	ArrayType* dstArrayType = (ArrayType*) type;
	Type* dstElementType = dstArrayType->getElementType ();

	Type* opType = opValue.getType ();

	if (opType->getTypeKind () != TypeKind_Array)
		return false;

	ArrayType* srcArrayType = (ArrayType*) opType;
	Type* srcElementType = srcArrayType->getElementType ();

	if (dstElementType->cmp (srcElementType) == 0 ||
		(dstElementType->getTypeKindFlags () & TypeKindFlag_Integer) &&
		(srcElementType->getTypeKindFlags () & TypeKindFlag_Integer) &&
		dstElementType->getSize () == srcElementType->getSize())
	{
		size_t dstSize = type->getSize ();
		size_t srcSize = srcArrayType->getSize ();
		memcpy (dst, opValue.getConstData (), AXL_MIN (srcSize, dstSize));
		return true;
	}

	return false;
}

bool
Cast_Array::llvmCast (
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	err::setFormatStringError ("CCast_Array::LlvmCast is not yet implemented");
	return false;
}

//..............................................................................

} // namespace ct
} // namespace jnc
