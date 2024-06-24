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
#include "jnc_ct_BinOp_Idx.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_ArrayType.h"
#include "jnc_ct_LeanDataPtrValidator.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
BinOp_Idx::op(
	const Value& rawOpValue1,
	const Value& rawOpValue2,
	Value* resultValue
) {
	bool result;

	Value opValue1 = rawOpValue1;
	Value opValue2 = rawOpValue2;

	Type* opType1 = rawOpValue1.getType();
	if (opType1->getTypeKind() == TypeKind_DataRef) {
		Type* targetType = ((DataPtrType*)opType1)->getTargetType();
		TypeKind targetTypeKind = targetType->getTypeKind();
		switch (targetTypeKind) {
		case TypeKind_Array:
			return
				m_module->m_operatorMgr.castOperator(&opValue2, TypeKind_IntPtr) &&
				arrayIndexOperator(rawOpValue1, (ArrayType*)targetType, opValue2, resultValue);

		case TypeKind_Variant:
			return variantIndexOperator(rawOpValue1, opValue2, resultValue);
		}

		result = m_module->m_operatorMgr.loadDataRef(rawOpValue1, &opValue1);
		if (!result)
			return false;

		opType1 = opValue1.getType();
	}

	TypeKind typeKind = opType1->getTypeKind();
	switch (typeKind) {
		Field* field;

	case TypeKind_DataPtr:
		return
			m_module->m_operatorMgr.castOperator(&opValue2, TypeKind_IntPtr) &&
			m_module->m_operatorMgr.binaryOperator(BinOpKind_Add, opValue1, opValue2, &opValue1) &&
			m_module->m_operatorMgr.unaryOperator(UnOpKind_Indir, opValue1, resultValue);

	case TypeKind_Array:
		return
			m_module->m_operatorMgr.castOperator(&opValue2, TypeKind_IntPtr) &&
			arrayIndexOperator(opValue1, (ArrayType*)opType1, opValue2, resultValue);

	case TypeKind_Variant:
		err::setFormatStringError("r-value variant index is not implemented yet");
		return false;

	case TypeKind_String:
		return stringIndexOperator(opValue1, opValue2, resultValue);

	case TypeKind_PropertyRef:
	case TypeKind_PropertyPtr:
		return propertyIndexOperator(((PropertyPtrType*)opType1)->getTargetType(), opValue1, opValue2, resultValue);

	case TypeKind_ClassPtr:
		return derivableTypeIndexOperator(((ClassPtrType*)opType1)->getTargetType(), opValue1, opValue2, resultValue);

	default:
		if (opType1->getTypeKindFlags() & TypeKindFlag_Derivable)
			return derivableTypeIndexOperator((DerivableType*)opType1, opValue1, opValue2, resultValue);

		err::setFormatStringError("cannot index '%s'", opType1->getTypeString().sz());
		return false;
	}
}

bool
BinOp_Idx::arrayIndexOperator(
	const Value& opValue1,
	ArrayType* arrayType,
	const Value& opValue2,
	Value* resultValue
) {
	Type* elementType = arrayType->getElementType();

	if (opValue1.getValueKind() == ValueKind_Const &&
		opValue2.getValueKind() == ValueKind_Const) {
		size_t elementOffset = opValue2.getSizeT() * elementType->getSize();
		Type* type = opValue1.getType();
		if (!(type->getTypeKindFlags() & TypeKindFlag_Ptr)) {
			resultValue->createConst((char*)opValue1.getConstData() + elementOffset, elementType);
		} else {
			ASSERT(type->getTypeKindFlags() & TypeKindFlag_DataPtr);

			DataPtrType* ptrType = (DataPtrType*)type;
			DataPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind();

			if (ptrTypeKind == DataPtrTypeKind_Normal) {
				DataPtr ptr = *(DataPtr*)opValue1.getConstData();
				ptr.m_p = (char*)ptr.m_p + elementOffset;
				resultValue->createConst(
					&ptr,
					elementType->getDataPtrType(
						TypeKind_DataRef,
						DataPtrTypeKind_Normal,
						type->getFlags() & PtrTypeFlag__All
					)
				);
			} else {
				ASSERT(ptrTypeKind == DataPtrTypeKind_Thin);
				char* p = *(char**)opValue1.getConstData();
				p += elementOffset;
				resultValue->createConst(
					&p,
					elementType->getDataPtrType_c(
						TypeKind_DataRef,
						type->getFlags() & PtrTypeFlag__All
					)
				);
			}
		}

		return true;
	}

	TypeKind opTypeKind1 = opValue1.getType()->getTypeKind();

	if (opTypeKind1 != TypeKind_DataRef) {
		ASSERT(opTypeKind1 == TypeKind_Array);
		err::setFormatStringError("indexing register-based arrays is not supported yet");
		return false;
	}

	DataPtrType* opType1 = (DataPtrType*)opValue1.getType();
	uint_t ptrTypeFlags = opType1->getFlags() & PtrTypeFlag__All;
	if (ptrTypeFlags & PtrTypeFlag_Safe) {
		if (opValue2.getValueKind() == ValueKind_Const) {
			Value idxValue;
			bool result = m_module->m_operatorMgr.castOperator(opValue2, TypeKind_IntPtr, &idxValue);
			if (!result)
				return false;

			intptr_t i = idxValue.getSizeT();
			if (i < 0 || i >= (intptr_t)arrayType->getElementCount()) {
				err::setFormatStringError("index '%d' is out of bounds in '%s'", i, arrayType->getTypeString().sz());
				return false;
			}
		} else {
			ptrTypeFlags &= ~PtrTypeFlag_Safe;
		}
	}

	Value ptrValue;
	DataPtrTypeKind ptrTypeKind = opType1->getPtrTypeKind();
	DataPtrType* ptrType;

	if (!m_module->hasCodeGen()) {
		switch (ptrTypeKind) {
		case DataPtrTypeKind_Thin:
			ptrType = elementType->getDataPtrType(TypeKind_DataRef, DataPtrTypeKind_Thin, ptrTypeFlags);
			break;

		case DataPtrTypeKind_Lean:
			ptrType = elementType->getDataPtrType(TypeKind_DataRef, DataPtrTypeKind_Lean, ptrTypeFlags);
			break;

		default: // DataPtrTypeKind_Normal
			ptrType = elementType->getDataPtrType(TypeKind_DataRef, DataPtrTypeKind_Lean, ptrTypeFlags);
		}

		resultValue->setType(ptrType);
	} else {
		switch (ptrTypeKind) {
		case DataPtrTypeKind_Thin:
			ptrType = elementType->getDataPtrType(TypeKind_DataRef, DataPtrTypeKind_Thin, ptrTypeFlags);
			m_module->m_llvmIrBuilder.createGep2(opValue1, arrayType, opValue2, ptrType, resultValue);
			break;

		case DataPtrTypeKind_Lean:
			ptrType = elementType->getDataPtrType(TypeKind_DataRef, DataPtrTypeKind_Lean, ptrTypeFlags);
			m_module->m_llvmIrBuilder.createGep2(opValue1, arrayType, opValue2, ptrType, resultValue);
			resultValue->setLeanDataPtrValidator(opValue1.getLeanDataPtrValidator());
			break;

		default: // DataPtrTypeKind_Normal
			ptrType = elementType->getDataPtrType(TypeKind_DataRef, DataPtrTypeKind_Lean, ptrTypeFlags);
			m_module->m_llvmIrBuilder.createExtractValue(opValue1, 0, NULL, &ptrValue);
			m_module->m_llvmIrBuilder.createBitCast(ptrValue, ptrType, &ptrValue);
			m_module->m_llvmIrBuilder.createGep(ptrValue, elementType, opValue2, ptrType, resultValue);
			resultValue->setLeanDataPtrValidator(opValue1);
		}
	}

	return true;
}

bool
BinOp_Idx::variantIndexOperator(
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
) {
	Property* prop = m_module->m_functionMgr.getStdProperty(StdProp_VariantIndex);
	resultValue->setProperty(prop);

	Value variantValue;
	bool result = m_module->m_operatorMgr.unaryOperator(UnOpKind_Addr, opValue1, &variantValue);
	if (!result)
		return false;

	Closure* closure = resultValue->createClosure();
	closure->append(variantValue);
	closure->append(opValue2);
	return true;
}

bool
BinOp_Idx::stringIndexOperator(
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
) {
	StructType* stringType = (StructType*)m_module->m_typeMgr.getStdType(StdType_StringStruct);
	Field* ptrField = stringType->getFieldArray()[0];
	Value ptrValue;

	return
		m_module->m_operatorMgr.getField(opValue1, stringType, ptrField, &ptrValue) &&
		m_module->m_operatorMgr.binaryOperator(BinOpKind_Add, ptrValue, opValue2, &ptrValue) &&
		m_module->m_operatorMgr.unaryOperator(UnOpKind_Indir, ptrValue, resultValue);
}

bool
BinOp_Idx::propertyIndexOperator(
	PropertyType* propertyType,
	const Value& rawOpValue1,
	const Value& rawOpValue2,
	Value* resultValue
) {
	Closure* closure = rawOpValue1.getClosure();
	size_t closureArgCount = closure ? closure->getArgValueList()->getCount() : 0;
	size_t propArgCount = propertyType->getGetterType()->getArgArray().getCount();
	if (closureArgCount >= propArgCount) { // closure is full; use normal index operator
		Value opValue1;
		return
			m_module->m_operatorMgr.getProperty(rawOpValue1, &opValue1) &&
			m_module->m_operatorMgr.binaryOperator(BinOpKind_Idx, opValue1, rawOpValue2, resultValue);
	}

	*resultValue = rawOpValue1;

	if (!closure)
		closure = resultValue->createClosure();

	closure->getArgValueList()->insertTail(rawOpValue2);
	return true;
}

bool
BinOp_Idx::derivableTypeIndexOperator(
	DerivableType* derivableType,
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
) {
	Property* prop = getDerivableTypeIndexerProperty(derivableType, opValue2);
	if (!prop)
		return false;

	resultValue->setProperty(prop);
	Closure* closure = resultValue->createClosure();
	closure->getArgValueList()->insertTail(opValue1);
	closure->getArgValueList()->insertTail(opValue2);

	return true;
}

Property*
BinOp_Idx::getDerivableTypeIndexerProperty(
	DerivableType* derivableType,
	const Value& opValue2
) {
	if (derivableType->hasIndexerProperties())
		return derivableType->chooseIndexerProperty(opValue2);

	sl::Array<BaseTypeSlot*> baseTypeArray = derivableType->getBaseTypeArray();
	size_t count = baseTypeArray.getCount();
	for (size_t i = 0; i < count; i ++) {
		DerivableType* baseType = baseTypeArray[i]->getType();
		if (baseType->hasIndexerProperties())
			return baseType->chooseIndexerProperty(opValue2);
	}

	err::setFormatStringError("'%s' has no indexer properties", derivableType->getTypeString().sz());
	return NULL;
}

//..............................................................................

} // namespace ct
} // namespace jnc
