#include "pch.h"
#include "jnc_CastOp_Struct.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CastKind
Cast_Struct::getCastKind (
	const Value& opValue,
	Type* type
	)
{
	if (opValue.getType ()->getTypeKind () != TypeKind_Struct)
		return CastKind_None;

	StructType* structType = (StructType*) opValue.getType ();
	return structType->findBaseType (type) ? CastKind_Implicit : CastKind_None;
}

bool
Cast_Struct::constCast (
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	if (opValue.getType ()->getTypeKind () != TypeKind_Struct)
	{
		setCastError (opValue, type);
		return false;
	}

	StructType* structType = (StructType*) opValue.getType ();

	BaseTypeCoord coord;
	bool result = structType->findBaseTypeTraverse (type, &coord);
	if (!result)
	{
		setCastError (opValue, type);
		return false;
	}
	
	memcpy (dst, (char*) opValue.getConstData () + coord.m_offset, type->getSize ());
	return true;
}

bool
Cast_Struct::llvmCast (
	StorageKind storageKind,
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	if (opValue.getType ()->getTypeKind () != TypeKind_Struct)
	{
		setCastError (opValue, type);
		return false;
	}

	StructType* structType = (StructType*) opValue.getType ();

	BaseTypeCoord coord;
	bool result = structType->findBaseTypeTraverse (type, &coord);
	if (!result)
	{
		setCastError (opValue, type);
		return false;
	}

	m_module->m_llvmIrBuilder.createExtractValue (
		opValue, 
		coord.m_llvmIndexArray, 
		coord.m_llvmIndexArray.getCount (), 
		type, 
		resultValue
		);

	return true;
}

//.............................................................................

} // namespace jnc {
