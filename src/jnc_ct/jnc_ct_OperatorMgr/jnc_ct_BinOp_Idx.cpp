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

namespace jnc {
namespace ct {

//..............................................................................

Type*
BinOp_Idx::getResultType (
	const Value& opValue1,
	const Value& opValue2
	)
{
	Type* opType1 = opValue1.getType ();
	if (opType1->getTypeKind () == TypeKind_DataRef)
	{
		DataPtrType* ptrType = (DataPtrType*) opType1;
		Type* targetType = ptrType->getTargetType ();
		TypeKind targetTypeKind = targetType->getTypeKind ();
		switch (targetTypeKind)
		{
		case TypeKind_Array:
			return ((ArrayType*) targetType)->getElementType ()->getDataPtrType (
				TypeKind_DataRef,
				ptrType->getPtrTypeKind (),
				ptrType->getFlags ()
				);

		case TypeKind_Variant:
			return m_module->m_typeMgr.getSimplePropertyType (targetType); // variant property
		}

		opType1 = targetType;
	}

	DataPtrType* ptrType;

	TypeKind typeKind = opType1->getTypeKind ();
	switch (typeKind)
	{
	case TypeKind_DataPtr:
		ptrType = (DataPtrType*) opType1;
		return ptrType->getTargetType ()->getDataPtrType (
			TypeKind_DataRef,
			ptrType->getPtrTypeKind (),
			ptrType->getFlags ()
			);

	case TypeKind_Array:
		return ((ArrayType*) opType1)->getElementType ();

	case TypeKind_Variant:
		return opType1; // variant property

	case TypeKind_PropertyRef:
	case TypeKind_PropertyPtr:
		return getPropertyIndexResultType (opValue1, opValue2);

	case TypeKind_ClassPtr:
		return getDerivableTypeIndexResultType (((ClassPtrType*) opType1)->getTargetType (), opValue1, opValue2);

	default:
		if (opType1->getTypeKindFlags () & TypeKindFlag_Derivable)
			return getDerivableTypeIndexResultType ((DerivableType*) opType1, opValue1, opValue2);

		err::setFormatStringError ("cannot index '%s'", opType1->getTypeString ().sz ());
		return NULL;
	}
}

bool
BinOp_Idx::op (
	const Value& rawOpValue1,
	const Value& rawOpValue2,
	Value* resultValue
	)
{
	bool result;

	Value opValue1 = rawOpValue1;
	Value opValue2 = rawOpValue2;

	Type* opType1 = rawOpValue1.getType ();
	if (opType1->getTypeKind () == TypeKind_DataRef)
	{
		Type* targetType = ((DataPtrType*) opType1)->getTargetType ();
		TypeKind targetTypeKind = targetType->getTypeKind ();
		switch (targetTypeKind)
		{
		case TypeKind_Array:
			return
				m_module->m_operatorMgr.castOperator (&opValue2, TypeKind_IntPtr) &&
				arrayIndexOperator (rawOpValue1, (ArrayType*) targetType, opValue2, resultValue);

		case TypeKind_Variant:
			return variantIndexOperator (rawOpValue1, opValue2, resultValue);
		}

		result = m_module->m_operatorMgr.loadDataRef (rawOpValue1, &opValue1);
		if (!result)
			return false;

		opType1 = opValue1.getType ();
	}

	TypeKind typeKind = opType1->getTypeKind ();
	switch (typeKind)
	{
	case TypeKind_DataPtr:
		return
			m_module->m_operatorMgr.castOperator (&opValue2, TypeKind_IntPtr) &&
			m_module->m_operatorMgr.binaryOperator (BinOpKind_Add, opValue1, opValue2, &opValue1) &&
			m_module->m_operatorMgr.unaryOperator (UnOpKind_Indir, opValue1, resultValue);

	case TypeKind_Array:
		return
			m_module->m_operatorMgr.castOperator (&opValue2, TypeKind_IntPtr) &&
			arrayIndexOperator (opValue1, (ArrayType*) opType1, opValue2, resultValue);

	case TypeKind_Variant:
		err::setFormatStringError ("r-value variant index is not implemented yet");
		return false;

	case TypeKind_PropertyRef:
	case TypeKind_PropertyPtr:
		return propertyIndexOperator (opValue1, opValue2, resultValue);

	case TypeKind_ClassPtr:
		return derivableTypeIndexOperator (((ClassPtrType*) opType1)->getTargetType (), opValue1, opValue2, resultValue);

	default:
		if (opType1->getTypeKindFlags () & TypeKindFlag_Derivable)
			return derivableTypeIndexOperator ((DerivableType*) opType1, opValue1, opValue2, resultValue);

		err::setFormatStringError ("cannot index '%s'", opType1->getTypeString ().sz ());
		return false;
	}
}

bool
BinOp_Idx::arrayIndexOperator (
	const Value& opValue1,
	ArrayType* arrayType,
	const Value& opValue2,
	Value* resultValue
	)
{
	Type* elementType = arrayType->getElementType ();

	if (opValue1.getValueKind () == ValueKind_Const &&
		opValue2.getValueKind () == ValueKind_Const)
	{
		size_t elementOffset = opValue2.getSizeT () * elementType->getSize ();
		Type* type = opValue1.getType ();
		if (!(type->getTypeKindFlags () & TypeKindFlag_Ptr))
		{
			resultValue->createConst ((char*) opValue1.getConstData () + elementOffset, elementType);
		}
		else
		{
			ASSERT (type->getTypeKindFlags () & TypeKindFlag_DataPtr);

			DataPtrType* ptrType = (DataPtrType*) type;
			DataPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind ();

			if (ptrTypeKind == DataPtrTypeKind_Normal)
			{
				DataPtr ptr = *(DataPtr*) opValue1.getConstData ();
				ptr.m_p = (char*) ptr.m_p + elementOffset;
				resultValue->createConst (&ptr, elementType->getDataPtrType (TypeKind_DataRef, DataPtrTypeKind_Normal, type->getFlags ()));
			}
			else
			{
				ASSERT (ptrTypeKind == DataPtrTypeKind_Thin);
				char* p = *(char**) opValue1.getConstData ();
				p += elementOffset;
				resultValue->createConst (&p, elementType->getDataPtrType_c (TypeKind_DataRef, type->getFlags ()));
			}
		}

		return true;
	}

	TypeKind opTypeKind1 = opValue1.getType ()->getTypeKind ();

	if (opTypeKind1 != TypeKind_DataRef)
	{
		ASSERT (opTypeKind1 == TypeKind_Array);
		err::setFormatStringError ("indexing register-based arrays is not supported yet");
		return false;
	}

	DataPtrType* opType1 = (DataPtrType*) opValue1.getType ();

	DataPtrType* ptrType;

	uint_t ptrTypeFlags = opType1->getFlags ();

	if (ptrTypeFlags & PtrTypeFlag_Safe)
	{
		if (opValue2.getValueKind () == ValueKind_Const)
		{
			Value idxValue;
			bool result = m_module->m_operatorMgr.castOperator (opValue2, TypeKind_IntPtr, &idxValue);
			if (!result)
				return false;

			intptr_t i = idxValue.getSizeT ();
			if (i < 0 || i >= (intptr_t) arrayType->getElementCount ())
			{
				err::setFormatStringError ("index '%d' is out of bounds in '%s'", i, arrayType->getTypeString ().sz ());
				return false;
			}
		}
		else
		{
			ptrTypeFlags &= ~PtrTypeFlag_Safe;
		}
	}

	Value ptrValue;

	DataPtrTypeKind ptrTypeKind = opType1->getPtrTypeKind ();
	if (ptrTypeKind == DataPtrTypeKind_Thin)
	{
		ptrType = elementType->getDataPtrType (TypeKind_DataRef, DataPtrTypeKind_Thin, ptrTypeFlags);
		m_module->m_llvmIrBuilder.createGep2 (opValue1, opValue2, ptrType, resultValue);
	}
	else if (ptrTypeKind == DataPtrTypeKind_Lean)
	{
		ptrType = elementType->getDataPtrType (TypeKind_DataRef, DataPtrTypeKind_Lean, ptrTypeFlags);
		m_module->m_llvmIrBuilder.createGep2 (opValue1, opValue2, ptrType, resultValue);
		resultValue->setLeanDataPtrValidator (opValue1.getLeanDataPtrValidator ());
	}
	else // EDataPtrType_Normal
	{
		m_module->m_llvmIrBuilder.createExtractValue (opValue1, 0, NULL, &ptrValue);

		ptrType = elementType->getDataPtrType_c ();

		m_module->m_llvmIrBuilder.createGep2 (ptrValue, opValue2, NULL, &ptrValue);

		ptrType = elementType->getDataPtrType (TypeKind_DataRef, DataPtrTypeKind_Lean, ptrTypeFlags);

		resultValue->setLeanDataPtr (
			ptrValue.getLlvmValue (),
			ptrType,
			opValue1
			);
	}

	return true;
}

bool
BinOp_Idx::variantIndexOperator (
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
	)
{
	Property* prop = m_module->m_functionMgr.getStdProperty (StdProp_VariantIndex);
	resultValue->setProperty (prop);

	Value variantValue;
	bool result = m_module->m_operatorMgr.unaryOperator (UnOpKind_Addr, opValue1, &variantValue);
	if (!result)
		return false;

	Closure* closure = resultValue->createClosure ();
	closure->append (variantValue);
	closure->append (opValue2);
	return true;
}

bool
BinOp_Idx::propertyIndexOperator (
	const Value& rawOpValue1,
	const Value& rawOpValue2,
	Value* resultValue
	)
{
	*resultValue = rawOpValue1;

	Closure* closure = resultValue->getClosure ();
	if (!closure)
		closure = resultValue->createClosure ();

	closure->getArgValueList ()->insertTail (rawOpValue2);
	return true;
}

Type*
BinOp_Idx::getPropertyIndexResultType (
	const Value& rawOpValue1,
	const Value& rawOpValue2
	)
{
	Value resultValue;
	propertyIndexOperator (rawOpValue1, rawOpValue2, &resultValue);
	return resultValue.getClosure ()->getClosureType (rawOpValue1.getType ());
}

Type*
BinOp_Idx::getDerivableTypeIndexResultType (
	DerivableType* derivableType,
	const Value& opValue1,
	const Value& opValue2
	)
{
	Property* prop = getDerivableTypeIndexerProperty (derivableType, opValue2);
	if (!prop)
		return NULL;

	Value resultValue = prop;
	Closure* closure = resultValue.createClosure ();
	closure->getArgValueList ()->insertTail (opValue1);
	closure->getArgValueList ()->insertTail (opValue2);

	return closure->getClosureType (prop->getType ());
}

bool
BinOp_Idx::derivableTypeIndexOperator (
	DerivableType* derivableType,
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
	)
{
	Property* prop = getDerivableTypeIndexerProperty (derivableType, opValue2);
	if (!prop)
		return false;

	resultValue->setProperty (prop);
	Closure* closure = resultValue->createClosure ();
	closure->getArgValueList ()->insertTail (opValue1);
	closure->getArgValueList ()->insertTail (opValue2);

	return true;
}

Property*
BinOp_Idx::getDerivableTypeIndexerProperty (
	DerivableType* derivableType,
	const Value& opValue2
	)
{
	if (derivableType->hasIndexerProperties ())
		return derivableType->chooseIndexerProperty (opValue2);

	sl::Array <BaseTypeSlot*> baseTypeArray = derivableType->getBaseTypeArray ();
	size_t count = baseTypeArray.getCount ();
	for (size_t i = 0; i < count; i ++)
	{
		DerivableType* baseType = baseTypeArray [i]->getType ();
		if (baseType->hasIndexerProperties ())
			return baseType->chooseIndexerProperty (opValue2);
	}

	err::setFormatStringError ("'%s' has no indexer properties", derivableType->getTypeString ().sz ());
	return NULL;
}

//..............................................................................

} // namespace ct
} // namespace jnc
