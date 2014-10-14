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
	if (opValue.getType ()->getTypeKind () == TypeKind_Struct)
	{
		StructType* srcStructType = (StructType*) opValue.getType ();
		if (srcStructType->findBaseType (type))
			return CastKind_Implicit;
	}

	ASSERT (type->getTypeKind () == TypeKind_Struct);
	StructType* structType = (StructType*) type;

	Function* constructor = structType->getConstructor ();
	if (constructor)
	{
		rtl::StringHashTableMapIterator <bool> it = m_recursionStopperSet.visit (structType->getSignature ());
		if (it->m_value)
			return CastKind_None;

		it->m_value = true;

		Value argValueArray [2];
		argValueArray [0].setType (structType->getDataPtrType ());
		argValueArray [1] = opValue;

		CastKind castKind;
		Function* overload = constructor->chooseOverload (argValueArray, 2, &castKind);

		m_recursionStopperSet.erase (it);

		if (overload)	
			return AXL_MIN (castKind, CastKind_ImplicitCrossFamily);
	}

	return CastKind_None;
}

bool
Cast_Struct::constCast (
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	if (opValue.getType ()->getTypeKind () != TypeKind_Struct)
		return false;

	StructType* srcStructType = (StructType*) opValue.getType ();

	BaseTypeCoord coord;
	bool result = srcStructType->findBaseTypeTraverse (type, &coord);
	if (!result)
		return false;
	
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
	bool result;

	if (opValue.getType ()->getTypeKind () == TypeKind_Struct)
	{
		StructType* srcStructType = (StructType*) opValue.getType ();

		BaseTypeCoord coord;
		result = srcStructType->findBaseTypeTraverse (type, &coord);
		if (result)
		{
			m_module->m_llvmIrBuilder.createExtractValue (
				opValue, 
				coord.m_llvmIndexArray, 
				coord.m_llvmIndexArray.getCount (), 
				type, 
				resultValue
				);

			return true;
		}
	}

	ASSERT (type->getTypeKind () == TypeKind_Struct);
	StructType* structType = (StructType*) type;

	Function* constructor = structType->getConstructor ();
	if (!constructor)
	{
		setCastError (opValue, type);
		return false;	
	}

	Variable* tmpVariable = m_module->m_variableMgr.createStackVariable ("tmpStruct", type);
	
	Value tmpValue;

	rtl::StringHashTableMapIterator <bool> it = m_recursionStopperSet.visit (structType->getSignature ());
	if (it->m_value)
	{
		setCastError (opValue, type);
		return false;	
	}

	it->m_value = true;

	result = 
		m_module->m_variableMgr.allocatePrimeInitializeVariable (tmpVariable) &&
		m_module->m_operatorMgr.unaryOperator (UnOpKind_Addr, tmpVariable, &tmpValue) &&
		m_module->m_operatorMgr.callOperator (constructor, tmpValue, opValue) &&
		m_module->m_operatorMgr.loadDataRef (tmpVariable, resultValue);

	m_recursionStopperSet.erase (it);

	return result;
}

//.............................................................................

} // namespace jnc {
