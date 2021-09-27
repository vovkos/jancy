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
#include "jnc_ct_CastOp_Struct.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

CastKind
Cast_Struct::getCastKind(
	const Value& opValue,
	Type* type
) {
	if (opValue.getType()->getTypeKind() == TypeKind_Struct) {
		StructType* srcStructType = (StructType*)opValue.getType();
		if (srcStructType->findBaseType(type))
			return CastKind_Implicit;
	}

	ASSERT(type->getTypeKind() == TypeKind_Struct);
	StructType* structType = (StructType*)type;

	OverloadableFunction constructor = structType->getConstructor();
	if (!constructor || m_recursionStopper)
		return CastKind_None;

	CastKind castKind;
	Value argValueArray[2];
	argValueArray[0].setType(structType->getDataPtrType());
	argValueArray[1] = opValue;

	m_recursionStopper = true;

	if (constructor->getItemKind() == ModuleItemKind_Function) {
		FunctionTypeOverload type = constructor.getFunction()->getType();
		bool result = type.chooseOverload(argValueArray, 2, &castKind) != -1;
		if (!result)
			return CastKind_None;
	} else {
		Function* overload = constructor.getFunctionOverload()->chooseOverload(argValueArray, 2, &castKind);
		if (!overload)
			return CastKind_None;
	}

	m_recursionStopper = false;

	return AXL_MIN(castKind, CastKind_ImplicitCrossFamily);
}

bool
Cast_Struct::constCast(
	const Value& opValue,
	Type* type,
	void* dst
) {
	if (opValue.getType()->getTypeKind() != TypeKind_Struct)
		return false;

	StructType* srcStructType = (StructType*)opValue.getType();

	BaseTypeCoord coord;
	bool result = srcStructType->findBaseTypeTraverse(type, &coord);
	if (!result)
		return false;

	memcpy(dst, (char*)opValue.getConstData() + coord.m_offset, type->getSize());
	return true;
}

bool
Cast_Struct::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	bool result;

	if (opValue.getType()->getTypeKind() == TypeKind_Struct) {
		StructType* srcStructType = (StructType*)opValue.getType();

		BaseTypeCoord coord;
		result = srcStructType->findBaseTypeTraverse(type, &coord);
		if (result) {
			m_module->m_llvmIrBuilder.createExtractValue(
				opValue,
				coord.m_llvmIndexArray,
				coord.m_llvmIndexArray.getCount(),
				type,
				resultValue
			);

			return true;
		}
	}

	ASSERT(type->getTypeKind() == TypeKind_Struct);
	StructType* structType = (StructType*)type;

	OverloadableFunction constructor = structType->getConstructor();
	if (!constructor) {
		setCastError(opValue, type);
		return false;
	}

	if (m_recursionStopper) {
		setCastError(opValue, type);
		return false;
	}

	m_recursionStopper = true;

	Variable* tmpVariable = m_module->m_variableMgr.createSimpleStackVariable("tmpStruct", type);

	Value tmpValue;
	result =
		m_module->m_operatorMgr.unaryOperator(UnOpKind_Addr, tmpVariable, &tmpValue) &&
		m_module->m_operatorMgr.callOperator(constructor, tmpValue, opValue) &&
		m_module->m_operatorMgr.loadDataRef(tmpVariable, resultValue);

	m_recursionStopper = false;

	return result;
}

//..............................................................................

} // namespace ct
} // namespace jnc
