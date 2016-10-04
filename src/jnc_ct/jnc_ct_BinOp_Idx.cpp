#include "pch.h"
#include "jnc_ct_BinOp_Idx.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

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
		Type* baseType = ptrType->getTargetType ();

		if (baseType->getTypeKind () == TypeKind_Array)
			return ((ArrayType*) baseType)->getElementType ()->getDataPtrType (
				TypeKind_DataRef, 
				ptrType->getPtrTypeKind (), 
				ptrType->getFlags ()
				);

		opType1 = baseType;
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

	case TypeKind_PropertyRef:
	case TypeKind_PropertyPtr:
		return getPropertyIndexResultType (opValue1, opValue2);

	default:
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
		Type* baseType = ((DataPtrType*) opType1)->getTargetType ();

		if (baseType->getTypeKind () == TypeKind_Array)
			return 
				m_module->m_operatorMgr.castOperator (&opValue2, TypeKind_IntPtr) &&
				arrayIndexOperator (rawOpValue1, (ArrayType*) baseType, opValue2, resultValue);

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

	case TypeKind_PropertyRef:
	case TypeKind_PropertyPtr:
		return propertyIndexOperator (opValue1, opValue2, resultValue);

	default:
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

	if (opValue1.getValueKind () == ValueKind_Const && opValue2.getValueKind ())
	{
		void* p = (char*) opValue1.getConstData () + opValue2.getSizeT () * elementType->getSize ();
		resultValue->createConst (p, elementType);
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

//.............................................................................

} // namespace ct
} // namespace jnc
