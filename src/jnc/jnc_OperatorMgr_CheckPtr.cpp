#include "pch.h"
#include "jnc_OperatorMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

void
OperatorMgr::getDataRefObjHdr (
	const Value& value,
	Value* resultValue
	)
{
	ASSERT (value.getType ()->getTypeKind () == TypeKind_DataRef);
	DataPtrType* ptrType = (DataPtrType*) value.getType ();
	DataPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind ();

	if (ptrTypeKind == DataPtrTypeKind_Lean)
	{
		getLeanDataPtrObjHdr (value, resultValue);
	}
	else
	{
		m_module->m_llvmIrBuilder.createExtractValue (
			value,
			3,
			m_module->m_typeMgr.getStdType (StdType_ObjHdrPtr),
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

	bool canThrow = (scope->getFlags () & ScopeFlag_CanThrow) != 0;
	bool isThrowLocked = m_module->m_controlFlowMgr.isThrowLocked ();

	if (!canThrow && !isThrowLocked)
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

		if (!isThrowLocked)
		{
			bool result = m_module->m_controlFlowMgr.throwIf (returnValue, checkFunctionType);
			ASSERT (result);
		}
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

bool
OperatorMgr::checkDataPtrScopeLevel (
	const Value& srcValue,
	const Value& dstValue
	)
{
	ASSERT (srcValue.getType ()->getTypeKind () == TypeKind_DataPtr);

	DataPtrType* ptrType = (DataPtrType*) srcValue.getType ();
	DataPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind ();
		
	if (m_module->m_operatorMgr.isUnsafeRgn () || ptrTypeKind == DataPtrTypeKind_Thin)
		return true;

	if (srcValue.getValueKind () == ValueKind_Variable && dstValue.getValueKind () == ValueKind_Variable)
	{
		if (srcValue.getVariable ()->getScopeLevel () > dstValue.getVariable ()->getScopeLevel ())
		{
			err::setFormatStringError ("data pointer scope level mismatch");
			return false;
		}

		return true;
	}

	Value srcObjHdrValue;

	if (ptrTypeKind == DataPtrTypeKind_Lean)
		getLeanDataPtrObjHdr (srcValue, &srcObjHdrValue);
	else
		m_module->m_llvmIrBuilder.createExtractValue (srcValue, 3, m_module->m_typeMgr.getStdType (StdType_ObjHdrPtr), &srcObjHdrValue);

	Function* checkFunction;
	Value dstObjHdrValue;

	if (dstValue.getValueKind () == ValueKind_Variable)
	{
		checkFunction = m_module->m_functionMgr.getStdFunction (StdFunction_CheckScopeLevelDirect);
		dstObjHdrValue = m_module->m_namespaceMgr.getScopeLevel (dstValue.getVariable ()->getScopeLevel ());
	}
	else
	{
		checkFunction = m_module->m_functionMgr.getStdFunction (StdFunction_CheckScopeLevel);
		getDataRefObjHdr (dstValue, &dstObjHdrValue);
	}

	LlvmScopeComment comment (&m_module->m_llvmIrBuilder, "check data pointer scope level");

	m_module->m_llvmIrBuilder.createCall2 (
		checkFunction,
		checkFunction->getType (),
		srcObjHdrValue,
		dstObjHdrValue,
		NULL
		);

	return true;
}

void
OperatorMgr::checkClassPtrScopeLevel (
	const Value& srcValue,
	const Value& dstValue
	)
{
	ASSERT (srcValue.getType ()->getTypeKindFlags () & TypeKindFlag_ClassPtr);

	if (m_module->m_operatorMgr.isUnsafeRgn ())
		return;

	Value dstObjHdrValue;
	getDataRefObjHdr (dstValue, &dstObjHdrValue);

	LlvmScopeComment comment (&m_module->m_llvmIrBuilder, "check class scope level");

	Value ifaceValue;
	m_module->m_llvmIrBuilder.createBitCast (srcValue, m_module->m_typeMgr.getStdType (StdType_AbstractClassPtr), &ifaceValue);

	Function* checkFunction = m_module->m_functionMgr.getStdFunction (StdFunction_CheckClassPtrScopeLevel);

	m_module->m_llvmIrBuilder.createCall2 (
		checkFunction,
		checkFunction->getType (),
		ifaceValue,
		dstObjHdrValue,
		NULL
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

void
OperatorMgr::checkFunctionPtrScopeLevel (
	const Value& srcValue,
	const Value& dstValue
	)
{
	ASSERT (srcValue.getType ()->getTypeKindFlags () & TypeKindFlag_FunctionPtr);
	FunctionPtrType* ptrType = (FunctionPtrType*) srcValue.getType ();

	if (m_module->m_operatorMgr.isUnsafeRgn () || !ptrType->hasClosure ())
		return;

	Value closureValue;
	m_module->m_llvmIrBuilder.createExtractValue (srcValue, 1, m_module->m_typeMgr.getStdType (StdType_AbstractClassPtr), &closureValue);
	checkClassPtrScopeLevel (closureValue, dstValue);
}

void
OperatorMgr::checkPropertyPtrScopeLevel (
	const Value& srcValue,
	const Value& dstValue
	)
{
	ASSERT (srcValue.getType ()->getTypeKind () == TypeKind_PropertyPtr);
	PropertyPtrType* ptrType = (PropertyPtrType*) srcValue.getType ();

	if (m_module->m_operatorMgr.isUnsafeRgn () || !ptrType->hasClosure ())
		return;

	Value closureValue;
	m_module->m_llvmIrBuilder.createExtractValue (srcValue, 1, m_module->m_typeMgr.getStdType (StdType_AbstractClassPtr), &closureValue);
	checkClassPtrScopeLevel (closureValue, dstValue);
}

//.............................................................................

} // namespace jnc {
