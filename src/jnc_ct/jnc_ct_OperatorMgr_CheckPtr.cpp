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

namespace jnc {
namespace ct {

//..............................................................................

void
OperatorMgr::checkPtr (
	StdFunc stdCheckFunction,
	StdFunc stdTryCheckFunction,
	const Value* argValueArray,
	size_t argCount
	)
{
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	ASSERT (scope);

	if (!scope->canStaticThrow ())
	{
		Function* checkFunction = m_module->m_functionMgr.getStdFunction (stdCheckFunction);

		m_module->m_llvmIrBuilder.createCall (
			checkFunction,
			checkFunction->getType (),
			argValueArray,
			argCount,
			NULL
			);
	}
	else
	{
		Function* checkFunction = m_module->m_functionMgr.getStdFunction (stdTryCheckFunction);
		FunctionType* checkFunctionType = checkFunction->getType ();

		Value returnValue;
		m_module->m_llvmIrBuilder.createCall (
			checkFunction,
			checkFunctionType,
			argValueArray,
			argCount,
			&returnValue
			);

		bool result = m_module->m_controlFlowMgr.throwExceptionIf (returnValue, checkFunctionType);
		ASSERT (result);
	}
}

bool
OperatorMgr::checkDataPtrRange (const Value& value)
{
	ASSERT (value.getType ()->getTypeKindFlags () & TypeKindFlag_DataPtr);

	DataPtrType* type = (DataPtrType*) value.getType ();
	DataPtrTypeKind ptrTypeKind = type->getPtrTypeKind ();

	if (m_module->m_operatorMgr.isUnsafeRgn () ||
		(type->getFlags () & PtrTypeFlag_Safe) ||
		ptrTypeKind == DataPtrTypeKind_Thin)
		return true;

	size_t targetSize = type->getTargetType ()->getSize ();

	Value ptrValue;
	Value validatorValue;

	if (ptrTypeKind == DataPtrTypeKind_Normal)
	{
		m_module->m_llvmIrBuilder.createExtractValue (value, 0, NULL, &ptrValue);
		m_module->m_llvmIrBuilder.createExtractValue (value, 1, NULL, &validatorValue);
	}
	else
	{
		ASSERT (ptrTypeKind == DataPtrTypeKind_Lean);

		m_module->m_llvmIrBuilder.createBitCast (value, m_module->m_typeMgr.getStdType (StdType_BytePtr), &ptrValue);

		LeanDataPtrValidator* validator = value.getLeanDataPtrValidator ();
		if (validator->isDynamicRange () || validator->hasValidatorValue ())
		{
			validatorValue = validator->getValidatorValue ();
		}
		else
		{
			size_t rangeLength = validator->getRangeLength ();
			if (rangeLength < targetSize)
			{
				err::setFormatStringError ("'%s' fails range check", type->getTypeString ().sz ());
				return false;
			}

			rangeLength -= targetSize;

			Value rangeBeginValue = validator->getRangeBeginValue ();
			m_module->m_llvmIrBuilder.createBitCast (rangeBeginValue, m_module->m_typeMgr.getStdType (StdType_BytePtr), &rangeBeginValue);

			Value argValueArray [] =
			{
				ptrValue,
				rangeBeginValue,
				Value (rangeLength, m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT)),
			};

			checkPtr (
				StdFunc_CheckDataPtrRangeDirect,
				StdFunc_TryCheckDataPtrRangeDirect,
				argValueArray,
				countof (argValueArray)
				);
			return true;
		}
	}

	Value argValueArray [] =
	{
		ptrValue,
		Value (targetSize, m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT)),
		validatorValue,
	};

	checkPtr (
		StdFunc_CheckDataPtrRangeIndirect,
		StdFunc_TryCheckDataPtrRangeIndirect,
		argValueArray,
		countof (argValueArray)
		);

	return true;
}

void
OperatorMgr::checkNullPtr (const Value& value)
{
	Type* type = value.getType ();

	if (m_module->m_operatorMgr.isUnsafeRgn () || (type->getFlags () & PtrTypeFlag_Safe))
		return;

	TypeKind typeKind = type->getTypeKind ();

	bool isThin;

	switch (typeKind)
	{
	case TypeKind_ClassPtr:
	case TypeKind_ClassRef:
		isThin = true;
		break;

	case TypeKind_FunctionPtr:
	case TypeKind_FunctionRef:
		isThin = ((FunctionPtrType*) type)->getPtrTypeKind () == FunctionPtrTypeKind_Thin;
		break;

	case TypeKind_PropertyPtr:
	case TypeKind_PropertyRef:
		isThin = ((PropertyPtrType*) type)->getPtrTypeKind () == PropertyPtrTypeKind_Thin;
		break;

	default:
		ASSERT (false);
		return;
	}

	Value ptrValue;
	Value typeKindValue (typeKind, m_module->m_typeMgr.getPrimitiveType (TypeKind_Int));

	if (isThin)
		m_module->m_llvmIrBuilder.createBitCast (value, m_module->m_typeMgr.getStdType (StdType_BytePtr), &ptrValue);
	else
		m_module->m_llvmIrBuilder.createExtractValue (value, 0, NULL, &ptrValue);

	Value argValueArray [] =
	{
		ptrValue,
		typeKindValue,
	};

	checkPtr (
		StdFunc_CheckNullPtr,
		StdFunc_TryCheckNullPtr,
		argValueArray,
		countof (argValueArray)
		);
}

//..............................................................................

} // namespace ct
} // namespace jnc
