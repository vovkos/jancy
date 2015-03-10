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
OperatorMgr::checkDataPtrRange_lean (
	const Value& value,
	size_t size
	)
{
	Value ptrValue;
	Value sizeValue (size, m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT));
	Value rangeBeginValue;
	Value rangeEndValue;

	getLeanDataPtrRange (value, &rangeBeginValue, &rangeEndValue);
	m_module->m_llvmIrBuilder.createBitCast (value, m_module->m_typeMgr.getStdType (StdType_BytePtr), &ptrValue);

	Value argValueArray [] =
	{
		ptrValue,
		sizeValue,
		rangeBeginValue,
		rangeEndValue,
	};

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	if (!(scope->getFlags () & ScopeFlag_CanThrow))
	{
		Function* checkFunction = m_module->m_functionMgr.getStdFunction (StdFunction_CheckDataPtrRange_thin);

		m_module->m_llvmIrBuilder.createCall (
			checkFunction,
			checkFunction->getType (),
			argValueArray,
			countof (argValueArray),
			NULL
			);
	}
	else
	{
		Function* checkFunction = m_module->m_functionMgr.getStdFunction (StdFunction_TryCheckDataPtrRange_thin);
		FunctionType* checkFunctionType = checkFunction->getType ();

		Value returnValue;
		m_module->m_llvmIrBuilder.createCall (
			checkFunction,
			checkFunctionType,
			argValueArray,
			countof (argValueArray),
			&returnValue
			);

		bool result = m_module->m_controlFlowMgr.throwIf (returnValue, checkFunctionType);
		ASSERT (result);
	}
}

void
OperatorMgr::checkDataPtrRange_fat (
	const Value& value,
	size_t size
	)
{
	Value sizeValue (size, m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT));

	Value argValueArray [] =
	{
		value,
		sizeValue,
	};

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	if (!(scope->getFlags () & ScopeFlag_CanThrow))
	{
		Function* checkFunction = m_module->m_functionMgr.getStdFunction (StdFunction_CheckDataPtrRange_fat);
		m_module->m_llvmIrBuilder.createCall (
			checkFunction,
			checkFunction->getType (),
			argValueArray,
			countof (argValueArray), 
			NULL
			);
	}
	else
	{
		Function* checkFunction = m_module->m_functionMgr.getStdFunction (StdFunction_TryCheckDataPtrRange_fat);
		FunctionType* checkFunctionType = checkFunction->getType ();

		Value returnValue;
		m_module->m_llvmIrBuilder.createCall (
			checkFunction,
			checkFunctionType,
			argValueArray,
			countof (argValueArray),
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

	if (type->getFlags () & PtrTypeFlag_Safe)
		return;

	DataPtrTypeKind ptrTypeKind = type->getPtrTypeKind ();
	switch (ptrTypeKind)
	{
	case DataPtrTypeKind_Thin:
		return;

	case DataPtrTypeKind_Lean:
		checkDataPtrRange_lean (value, type->getTargetType ()->getSize ());
		break;

	case DataPtrTypeKind_Normal:
		checkDataPtrRange_fat (value, type->getTargetType ()->getSize ());
		break;

	default:
		ASSERT (false);
	}
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
		
	if (ptrTypeKind == DataPtrTypeKind_Thin) // in general case we can't deduce scope-level
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

	Value dstObjHdrValue;
	getDataRefObjHdr (dstValue, &dstObjHdrValue);

	LlvmScopeComment comment (&m_module->m_llvmIrBuilder, "check class scope level");

	Value ifaceValue;
	m_module->m_llvmIrBuilder.createBitCast (srcValue, m_module->m_typeMgr.getStdType (StdType_ObjectPtr), &ifaceValue);

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
OperatorMgr::checkClassPtrNull (const Value& value)
{
	ASSERT (value.getType ()->getTypeKindFlags () & TypeKindFlag_ClassPtr);

	ClassPtrType* ptrType = (ClassPtrType*) value.getType ();
	ClassPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind ();

	if (ptrType->getFlags () & PtrTypeFlag_Safe)
		return;

	LlvmScopeComment comment (&m_module->m_llvmIrBuilder, "check null class pointer");

	Value ptrValue;
	m_module->m_llvmIrBuilder.createBitCast (value, m_module->m_typeMgr.getStdType (StdType_BytePtr), &ptrValue);

	Function* checkFunction = m_module->m_functionMgr.getStdFunction (StdFunction_CheckNullPtr);

	m_module->m_llvmIrBuilder.createCall2 (
		checkFunction,
		checkFunction->getType (),
		ptrValue,
		Value (RuntimeErrorKind_NullClassPtr, m_module->m_typeMgr.getPrimitiveType (TypeKind_Int)),
		NULL
		);
}

void
OperatorMgr::checkFunctionPtrNull (const Value& value)
{
	ASSERT (value.getType ()->getTypeKindFlags () & TypeKindFlag_FunctionPtr);

	FunctionPtrType* ptrType = (FunctionPtrType*) value.getType ();
	FunctionPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind ();

	if (ptrType->getFlags () & PtrTypeFlag_Safe)
		return;

	LlvmScopeComment comment (&m_module->m_llvmIrBuilder, "check null function pointer");

	Value ptrValue;

	if (ptrTypeKind == FunctionPtrTypeKind_Thin)
		ptrValue = value;
	else
		m_module->m_llvmIrBuilder.createExtractValue (value, 0, NULL, &ptrValue);

	m_module->m_llvmIrBuilder.createBitCast (ptrValue, m_module->m_typeMgr.getStdType (StdType_BytePtr), &ptrValue);

	Function* checkFunction = m_module->m_functionMgr.getStdFunction (StdFunction_CheckNullPtr);

	m_module->m_llvmIrBuilder.createCall2 (
		checkFunction,
		checkFunction->getType (),
		ptrValue,
		Value (RuntimeErrorKind_NullFunctionPtr, m_module->m_typeMgr.getPrimitiveType (TypeKind_Int)),
		NULL
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

	if (!ptrType->hasClosure ())
		return;

	Value closureValue;
	m_module->m_llvmIrBuilder.createExtractValue (srcValue, 1, m_module->m_typeMgr.getStdType (StdType_ObjectPtr), &closureValue);
	checkClassPtrScopeLevel (closureValue, dstValue);
}

void
OperatorMgr::checkPropertyPtrNull (const Value& value)
{
	ASSERT (value.getType ()->getTypeKindFlags () & TypeKindFlag_PropertyPtr);

	PropertyPtrType* ptrType = (PropertyPtrType*) value.getType ();
	PropertyPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind ();

	if (ptrType->getFlags () & PtrTypeFlag_Safe)
		return;

	LlvmScopeComment comment (&m_module->m_llvmIrBuilder, "check null property pointer");

	Value ptrValue;

	if (ptrTypeKind == PropertyPtrTypeKind_Thin)
		ptrValue = value;
	else
		m_module->m_llvmIrBuilder.createExtractValue (value, 0, NULL, &ptrValue);

	m_module->m_llvmIrBuilder.createBitCast (ptrValue, m_module->m_typeMgr.getStdType (StdType_BytePtr), &ptrValue);

	Function* checkFunction = m_module->m_functionMgr.getStdFunction (StdFunction_CheckNullPtr);
	
	m_module->m_llvmIrBuilder.createCall2 (
		checkFunction,
		checkFunction->getType (),
		ptrValue,
		Value (RuntimeErrorKind_NullPropertyPtr, m_module->m_typeMgr.getPrimitiveType (TypeKind_Int)),
		NULL
		);
}

void
OperatorMgr::checkPropertyPtrScopeLevel (
	const Value& srcValue,
	const Value& dstValue
	)
{
	ASSERT (srcValue.getType ()->getTypeKind () == TypeKind_PropertyPtr);
	PropertyPtrType* ptrType = (PropertyPtrType*) srcValue.getType ();

	if (!ptrType->hasClosure ())
		return;

	Value closureValue;
	m_module->m_llvmIrBuilder.createExtractValue (srcValue, 1, m_module->m_typeMgr.getStdType (StdType_ObjectPtr), &closureValue);
	checkClassPtrScopeLevel (closureValue, dstValue);
}

//.............................................................................

} // namespace jnc {
