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
#include "jnc_ct_OperatorMgr.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_UnionType.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
OperatorMgr::getField(
	const Value& opValue,
	Field* field,
	MemberCoord* coord,
	Value* resultValue
) {
	Type* type = opValue.getType();

	if (type->getTypeKindFlags() & TypeKindFlag_DataPtr)
		type = ((DataPtrType*)type)->getTargetType();
	else if (opValue.getType()->getTypeKindFlags() & TypeKindFlag_ClassPtr)
		type = ((ClassPtrType*)opValue.getType())->getTargetType();

	if (type->getFlags() & TypeFlag_Dynamic) {
		ASSERT(type->getTypeKindFlags() & TypeKindFlag_Derivable);
		return getDynamicField(opValue, (DerivableType*)type, field, resultValue);
	}

	bool result = type->ensureLayout();
	if (!result)
		return false;

	TypeKind typeKind = type->getTypeKind();
	switch (typeKind) {
	case TypeKind_Struct:
		return getStructField(opValue, field, coord, resultValue);

	case TypeKind_Union:
		return coord ?
			getStructField(opValue, field, coord, resultValue) :
			getUnionField(opValue, field, resultValue);

	case TypeKind_Class:
		return getClassField(opValue, field, coord, resultValue);

	default:
		err::setFormatStringError("cannot get a field '%s' of '%s'", field->getName().sz(), type->getTypeString().sz());
		return false;
	}
}

bool
OperatorMgr::getPromiseField(
	const Value& promiseValue,
	const sl::String& name,
	Value* resultValue
) {
	ASSERT(((ClassPtrType*)promiseValue.getType())->getTargetType()->getBaseTypeArray() [0]->getType()->getStdType() == StdType_Promise);

	ClassType* promiseType = (ClassType*)m_module->m_typeMgr.getStdType(StdType_Promise);
	Field* stateField = (Field*)promiseType->findDirectChildItem(name).m_item;
	ASSERT(stateField && stateField->getItemKind() == ModuleItemKind_Field);

	MemberCoord coord;
	coord.m_llvmIndexArray.append(0); // account for base type jnc.Promise

	return getField(promiseValue, stateField, &coord, resultValue);
}

bool
OperatorMgr::getFieldPtrImpl(
	const Value& opValueRaw,
	MemberCoord* coord,
	Type* resultType,
	Value* resultValue
) {
	AXL_TODO("double check multiple levels of nested unnamed structs/unions")

	if (coord->m_unionCoordArray.isEmpty()) {
		m_module->m_llvmIrBuilder.createGep(
			opValueRaw,
			coord->m_llvmIndexArray,
			coord->m_llvmIndexArray.getCount(),
			resultType,
			resultValue
		);

		return true;
	}

	// if LLVM were to support unions natively, the following would be not needed

	Value opValue = opValueRaw;

	int32_t* llvmIndex = coord->m_llvmIndexArray;
	int32_t* llvmIndexEnd = llvmIndex + coord->m_llvmIndexArray.getCount();
	intptr_t unionLevel = -1; // take into account initial 0 in LlvmIndexArray

	size_t unionCount = coord->m_unionCoordArray.getCount();
	UnionCoord* unionCoord = coord->m_unionCoordArray;
	for (size_t i = 0; i < unionCount; i++, unionCoord++) {
		ASSERT(unionCoord->m_level >= unionLevel);
		size_t llvmIndexDelta = unionCoord->m_level - unionLevel;

		if (llvmIndexDelta) {
			m_module->m_llvmIrBuilder.createGep(
				opValue,
				llvmIndex,
				llvmIndexDelta,
				NULL,
				&opValue
			);
		}

		Field* field = unionCoord->m_type->getFieldByIndex(llvmIndex[llvmIndexDelta]);
		Type* type = field->getType()->getDataPtrType_c();

		m_module->m_llvmIrBuilder.createBitCast(opValue, type, &opValue);

		llvmIndex += llvmIndexDelta + 1;
		unionLevel = unionCoord->m_level + 1;
	}

	if (llvmIndexEnd > llvmIndex) {
		ASSERT(llvmIndex > coord->m_llvmIndexArray);

		llvmIndex--;
		*llvmIndex = 0; // create initial 0

		m_module->m_llvmIrBuilder.createGep(
			opValue,
			llvmIndex,
			llvmIndexEnd - llvmIndex,
			resultType,
			resultValue
		);
	} else {
		resultValue->overrideType(opValue, resultType);
	}

	return true;
}

bool
OperatorMgr::getStructField(
	const Value& opValue,
	Field* field,
	MemberCoord* coord,
	Value* resultValue
) {
	MemberCoord dummyCoord;
	if (!coord)
		coord = &dummyCoord;

	coord->m_llvmIndexArray.append(field->getLlvmIndex());
	coord->m_offset += field->getOffset();

	ValueKind opValueKind = opValue.getValueKind();
	if (opValueKind == ValueKind_Const) {
		Type* type = opValue.getType();
		if (!(type->getTypeKindFlags() & TypeKindFlag_Ptr)) {
			resultValue->createConst(
				(char*)opValue.getConstData() + coord->m_offset,
				field->getType()
			);
		} else {
			ASSERT(type->getTypeKindFlags() & TypeKindFlag_DataPtr);

			DataPtrType* ptrType = (DataPtrType*)type;
			DataPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind();

			if (ptrTypeKind == DataPtrTypeKind_Normal) {
				DataPtr ptr = *(DataPtr*)opValue.getConstData();
				ptr.m_p = (char*)ptr.m_p + field->getOffset();
				resultValue->createConst(&ptr, field->getType()->getDataPtrType(TypeKind_DataRef, DataPtrTypeKind_Normal, type->getFlags()));
			} else {
				ASSERT(ptrTypeKind == DataPtrTypeKind_Thin);
				char* p = *(char**) opValue.getConstData();
				p += field->getOffset();
				resultValue->createConst(&p, field->getType()->getDataPtrType_c(TypeKind_DataRef, type->getFlags()));
			}
		}

		return true;
	}

	if (!(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr)) {
		if (!coord->m_unionCoordArray.isEmpty()) {
			err::setFormatStringError("union member operator on registers is not implemented yet");
			return false;
		}

		m_module->m_llvmIrBuilder.createExtractValue(
			opValue,
			coord->m_llvmIndexArray,
			coord->m_llvmIndexArray.getCount(),
			field->getType(),
			resultValue
		);

		return true;
	}

	DataPtrType* opType = (DataPtrType*)opValue.getType();
	coord->m_llvmIndexArray.insert(0, 0);

	DataPtrTypeKind ptrTypeKind = opType->getPtrTypeKind();
	uint_t ptrTypeFlags = opType->getFlags() | field->getPtrTypeFlags();
	if (field->getStorageKind() == StorageKind_Mutable)
		ptrTypeFlags &= ~PtrTypeFlag_Const;

	DataPtrType* resultType;

	if (!m_module->hasCodeGen()) {
		resultType = field->getType()->getDataPtrType(TypeKind_DataRef, ptrTypeKind, ptrTypeFlags);
		resultValue->setType(resultType);
		return true;
	}

	if (ptrTypeKind == DataPtrTypeKind_Thin) {
		resultType = field->getType()->getDataPtrType(
			TypeKind_DataRef,
			DataPtrTypeKind_Thin,
			ptrTypeFlags
		);

		getFieldPtrImpl(opValue, coord, resultType, resultValue);
		return true;
	}

	Value ptrValue;
	if (ptrTypeKind == DataPtrTypeKind_Lean) {
		getFieldPtrImpl(opValue, coord, NULL, &ptrValue);
	} else {
		m_module->m_llvmIrBuilder.createExtractValue(opValue, 0, NULL, &ptrValue);
		m_module->m_llvmIrBuilder.createBitCast(ptrValue, opType->getTargetType()->getDataPtrType_c(), &ptrValue);
		getFieldPtrImpl(ptrValue, coord, NULL, &ptrValue);
	}

	if (opType->getTargetType()->getFlags() & TypeFlag_Pod) {
		resultType = field->getType()->getDataPtrType(
			TypeKind_DataRef,
			DataPtrTypeKind_Lean,
			ptrTypeFlags
		);

		resultValue->setLeanDataPtr(ptrValue.getLlvmValue(), resultType, opValue);
	} else {
		bool result = checkDataPtrRange(opValue);
		if (!result)
			return false;

		ptrTypeFlags |= PtrTypeFlag_Safe;
		resultType = field->getType()->getDataPtrType(
			TypeKind_DataRef,
			DataPtrTypeKind_Lean,
			ptrTypeFlags
		);

		resultValue->setLeanDataPtr(
			ptrValue.getLlvmValue(),
			resultType,
			opValue,
			ptrValue,
			field->getType()->getSize()
		);
	}

	return true;
}

bool
OperatorMgr::getDynamicField(
	const Value& opValue,
	DerivableType* type,
	Field* field,
	Value* resultValue
) {
	bool result;

	Function* getDynamicFieldFunc = m_module->m_functionMgr.getStdFunction(StdFunc_GetDynamicField);
	Value typeValue(&type, m_module->m_typeMgr.getStdType(StdType_BytePtr));
	Value fieldValue(&field, m_module->m_typeMgr.getStdType(StdType_BytePtr));

	Value ptrValue;
	result = m_module->m_operatorMgr.callOperator(
		getDynamicFieldFunc,
		opValue,
		typeValue,
		fieldValue,
		&ptrValue
	);

	if (!result)
		return false;

	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr); // otherwise, getDynamicFieldFunc fails
	DataPtrType* opType = (DataPtrType*)opValue.getType();

	DataPtrTypeKind ptrTypeKind = opType->getPtrTypeKind();
	ASSERT(ptrTypeKind != DataPtrTypeKind_Thin); // otherwise, getDynamicFieldFunc fails

	uint_t ptrTypeFlags = opType->getFlags() | field->getPtrTypeFlags();
	if (field->getStorageKind() == StorageKind_Mutable)
		ptrTypeFlags &= ~PtrTypeFlag_Const;

	ASSERT(opType->getTargetType()->getFlags() & TypeFlag_Pod); // dynamic struct must be POD
	DataPtrType* resultType = field->getType()->getDataPtrType(
		TypeKind_DataRef,
		DataPtrTypeKind_Lean,
		ptrTypeFlags
	);

	if (!m_module->hasCodeGen()) {
		resultValue->setType(resultType);
		return true;
	}

	m_module->m_llvmIrBuilder.createBitCast(ptrValue, resultType, &ptrValue);
	resultValue->setLeanDataPtr(ptrValue.getLlvmValue(), resultType, opValue);
	resultValue->setDynamicFieldInfo(opValue, type, field);
	return true;
}

bool
OperatorMgr::getUnionField(
	const Value& opValue,
	Field* field,
	Value* resultValue
) {
	ValueKind opValueKind = opValue.getValueKind();
	if (opValueKind == ValueKind_Const) {
		resultValue->createConst(opValue.getConstData(), field->getType());
		return true;
	}

	if (opValue.getType()->getTypeKind() != TypeKind_DataRef) {
		err::setFormatStringError("union member operator on registers is not implemented yet");
		return false;
	}

	DataPtrType* opType = (DataPtrType*)opValue.getType();

	uint_t ptrTypeFlags = opType->getFlags() | field->getPtrTypeFlags();
	if (field->getStorageKind() == StorageKind_Mutable)
		ptrTypeFlags &= ~PtrTypeFlag_Const;

	DataPtrTypeKind ptrTypeKind = opType->getPtrTypeKind();

	DataPtrType* ptrType = field->getType()->getDataPtrType(
		TypeKind_DataRef,
		ptrTypeKind == DataPtrTypeKind_Thin ? DataPtrTypeKind_Thin : DataPtrTypeKind_Lean,
		ptrTypeFlags
	);

	if (ptrTypeKind == DataPtrTypeKind_Thin) {
		m_module->m_llvmIrBuilder.createBitCast(opValue, ptrType, resultValue);
	} else if (ptrTypeKind == DataPtrTypeKind_Lean) {
		m_module->m_llvmIrBuilder.createBitCast(opValue, ptrType, resultValue);

		if (opValue.getValueKind() == ValueKind_Variable)
			resultValue->setLeanDataPtrValidator(opValue);
		else
			resultValue->setLeanDataPtrValidator(opValue.getLeanDataPtrValidator());
	} else {
		Value ptrValue;
		m_module->m_llvmIrBuilder.createExtractValue(opValue, 0, NULL, &ptrValue);
		m_module->m_llvmIrBuilder.createBitCast(opValue, field->getType()->getDataPtrType_c(), &ptrValue);

		resultValue->setLeanDataPtr(
			ptrValue.getLlvmValue(),
			ptrType,
			opValue
		);
	}

	return true;
}

bool
OperatorMgr::getClassField(
	const Value& rawOpValue,
	Field* field,
	MemberCoord* coord,
	Value* resultValue
) {
	Value opValue;
	bool result = prepareOperand(rawOpValue, &opValue);
	if (!result)
		return false;

	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_ClassPtr);
	ClassPtrType* opType = (ClassPtrType*)opValue.getType();

	uint_t ptrTypeFlags = opType->getFlags() | field->getPtrTypeFlags() | PtrTypeFlag_Safe;
	if (field->getStorageKind() == StorageKind_Mutable)
		ptrTypeFlags &= ~PtrTypeFlag_Const;

	if (!m_module->hasCodeGen()) {
		Type* resultType = field->getType()->getTypeKind() == TypeKind_Class ?
			(Type*)((ClassType*)field->getType())->getClassPtrType(
				TypeKind_ClassRef,
				ClassPtrTypeKind_Normal,
				ptrTypeFlags
			) :
			field->getType()->getDataPtrType(
				TypeKind_DataRef,
				DataPtrTypeKind_Lean,
				ptrTypeFlags
			);

		resultValue->setType(resultType);
		return true;
	}

	checkNullPtr(opValue);

	ClassType* classType = (ClassType*)field->getParentNamespace();

	MemberCoord dummyCoord;
	if (!coord)
		coord = &dummyCoord;

	coord->m_llvmIndexArray.insert(0, 0);
	coord->m_llvmIndexArray.append(field->getLlvmIndex());

	if (field->getType()->getTypeKind() == TypeKind_Class)
		coord->m_llvmIndexArray.append(1);

	Value ptrValue;
	m_module->m_llvmIrBuilder.createGep(
		opValue,
		coord->m_llvmIndexArray,
		coord->m_llvmIndexArray.getCount(),
		NULL,
		&ptrValue
	);

	if (field->getType()->getTypeKind() == TypeKind_Class) {
		ClassPtrType* ptrType = ((ClassType*)field->getType())->getClassPtrType(
			TypeKind_ClassRef,
			ClassPtrTypeKind_Normal,
			ptrTypeFlags
		);

		resultValue->setLlvmValue(ptrValue.getLlvmValue(), ptrType);
	} else {
		DataPtrType* ptrType = field->getType()->getDataPtrType(
			TypeKind_DataRef,
			DataPtrTypeKind_Lean,
			ptrTypeFlags
		);

		resultValue->setLeanDataPtr(
			ptrValue.getLlvmValue(),
			ptrType,
			opValue,
			ptrValue,
			field->getType()->getSize()
		);
	}

	return true;
}

bool
OperatorMgr::getPropertyField(
	const Value& opValue,
	ModuleItem* member,
	Value* resultValue
) {
	ModuleItemKind itemKind = member->getItemKind();

	switch (itemKind) {
	case ModuleItemKind_Field:
		break;

	case ModuleItemKind_Variable:
		resultValue->setVariable((Variable*)member);
		return true;

	default:
		ASSERT(false);
	}

	ASSERT(opValue.getValueKind() == ValueKind_Property);
	ASSERT(member->getItemKind() == ModuleItemKind_Field);

	Property* prop = opValue.getProperty();

	Closure* closure = opValue.getClosure();
	ASSERT(closure);

	Value parentValue = *closure->getArgValueList()->getHead();
	Type* parentValueType = parentValue.getType();

	DerivableType* parentType = prop->getParentType();
	ASSERT(parentType);

	Type* parentPtrType;
	if (parentType->getTypeKind() == TypeKind_Class) {
		parentPtrType = ((ClassType*)parentType)->getClassPtrType(
			ClassPtrTypeKind_Normal,
			parentValueType->getFlags()
		);
	} else {
		DataPtrTypeKind ptrTypeKind = (parentValueType->getTypeKindFlags() & TypeKindFlag_DataPtr) ?
			((DataPtrType*)parentValueType)->getPtrTypeKind() :
			DataPtrTypeKind_Normal;

		parentPtrType = parentType->getDataPtrType(
			ptrTypeKind,
			parentValueType->getFlags()
		);
	}

	return
		castOperator(&parentValue, parentPtrType) &&
		getField(parentValue, (Field*)member, resultValue);
}

//..............................................................................

} // namespace ct
} // namespace jnc
