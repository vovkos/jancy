#include "pch.h"
#include "jnc_CastOp_Array.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CastKind
Cast_Array::getCastKind (
	const Value& opValue,
	Type* type
	)
{
	return CastKind_Explicit;
}

bool
Cast_Array::constCast (
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	return false;
}

bool
Cast_Array::llvmCast (
	StorageKind storageKind,
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	err::setFormatStringError ("CCast_Array::LlvmCast is not yet implemented");
	return false;
}

//.............................................................................

} // namespace jnc {
