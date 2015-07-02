#include "pch.h"
#include "jnc_OperatorMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

void
OperatorMgr::getDataRefBox (
	const Value& value,
	Value* resultValue
	)
{
	ASSERT (value.getType ()->getTypeKind () == TypeKind_DataRef);
	DataPtrType* ptrType = (DataPtrType*) value.getType ();
	DataPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind ();

	if (ptrTypeKind == DataPtrTypeKind_Lean)
	{
		getLeanDataPtrBox (value, resultValue);
	}
	else
	{
		m_module->m_llvmIrBuilder.createExtractValue (
			value,
			3,
			m_module->m_typeMgr.getStdType (StdType_BoxPtr),
			resultValue
			);
	}
}

void
OperatorMgr::checkPtr (
	StdFunction stdTryCheckFunction,
	StdFunction stdCheckFunction,
	const Value* argValueArray,
	size_t argCount
	)
{
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	ASSERT (scope);

	if (!(scope->getFlags () & ScopeFlag_CanThrow))
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

		bool result = m_module->m_controlFlowMgr.throwIf (returnValue, checkFunctionType);
		ASSERT (result);
	}
}

void
OperatorMgr::checkDataPtrRange (const Value& value)
{
	ASSERT (value.getType ()->getTypeKind () == TypeKind_DataPtr || value.getType ()->getTypeKind () == TypeKind_DataRef);
	DataPtrType* type = (DataPtrType*) value.getType ();

	if (m_module->m_operatorMgr.isUnsafeRgn () || (type->getFlags () & PtrTypeFlag_Safe))
		return;

	Value ptrValue;
	Value rangeBeginValue;
	Value rangeEndValue;

	DataPtrTypeKind ptrTypeKind = type->getPtrTypeKind ();
	switch (ptrTypeKind)
	{
	case DataPtrTypeKind_Thin:
		return;

	case DataPtrTypeKind_Lean:
		getLeanDataPtrRange (value, &rangeBeginValue, &rangeEndValue);
		m_module->m_llvmIrBuilder.createBitCast (value, m_module->m_typeMgr.getStdType (StdType_BytePtr), &ptrValue);
		break;

	case DataPtrTypeKind_Normal:
		m_module->m_llvmIrBuilder.createExtractValue (value, 0, NULL, &ptrValue);
		m_module->m_llvmIrBuilder.createExtractValue (value, 1, NULL, &rangeBeginValue);
		m_module->m_llvmIrBuilder.createExtractValue (value, 2, NULL, &rangeEndValue);
		break;

	default:
		ASSERT (false);
		return;
	}

	Value sizeValue (
		type->getTargetType ()->getSize (), 
		m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT)
		);

	Value argValueArray [] =
	{
		ptrValue,
		sizeValue,
		rangeBeginValue,
		rangeEndValue,
	};

	checkPtr (
		StdFunction_TryCheckDataPtrRange,
		StdFunction_CheckDataPtrRange,
		argValueArray,
		countof (argValueArray)
		);
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
		StdFunction_TryCheckNullPtr,
		StdFunction_CheckNullPtr,
		argValueArray,
		countof (argValueArray)
		);

}

//.............................................................................

} // namespace jnc {
