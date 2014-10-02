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
			m_module->m_typeMgr.getStdType (StdTypeKind_ObjHdrPtr),
			resultValue
			);
	}
}

void
OperatorMgr::checkDataPtrRange (
	const Value& rawPtrValue,
	size_t size,
	const Value& rangeBeginValue,
	const Value& rangeEndValue
	)
{
	LlvmScopeComment comment (&m_module->m_llvmIrBuilder, "check data pointer range");

	Value sizeValue (size, TypeKind_SizeT);

	Value ptrValue;
	m_module->m_llvmIrBuilder.createBitCast (rawPtrValue, m_module->m_typeMgr.getStdType (StdTypeKind_BytePtr), &ptrValue);

	Value argValueArray [] =
	{
		ptrValue,
		sizeValue,
		rangeBeginValue,
		rangeEndValue,
	};

	Function* checkDataPtrRange = m_module->m_functionMgr.getStdFunction (StdFuncKind_CheckDataPtrRange);

	m_module->m_llvmIrBuilder.createCall (
		checkDataPtrRange,
		checkDataPtrRange->getType (),
		argValueArray,
		countof (argValueArray),
		NULL
		);
}

void
OperatorMgr::checkDataPtrRange (const Value& value)
{
	ASSERT (value.getType ()->getTypeKind () == TypeKind_DataPtr || value.getType ()->getTypeKind () == TypeKind_DataRef);
	DataPtrType* type = (DataPtrType*) value.getType ();
	DataPtrTypeKind ptrTypeKind = type->getPtrTypeKind ();

	if (type->getFlags () & PtrTypeFlagKind_Safe)
		return;

	Value ptrValue;
	Value rangeBeginValue;
	Value rangeEndValue;

	switch (ptrTypeKind)
	{
	case DataPtrTypeKind_Thin:
		return;

	case DataPtrTypeKind_Lean:
		ptrValue = value;
		getLeanDataPtrRange (value, &rangeBeginValue, &rangeEndValue);
		break;

	case DataPtrTypeKind_Normal:
		m_module->m_llvmIrBuilder.createExtractValue (
			value, 0, 
			type->getTargetType ()->getDataPtrType_c (), 
			&ptrValue
			);

		m_module->m_llvmIrBuilder.createExtractValue (value, 1, NULL, &rangeBeginValue);
		m_module->m_llvmIrBuilder.createExtractValue (value, 2, NULL, &rangeEndValue);
		break;

	default:
		ASSERT (false);
	}

	checkDataPtrRange (ptrValue, type->getTargetType ()->getSize (), rangeBeginValue, rangeEndValue);
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
		m_module->m_llvmIrBuilder.createExtractValue (srcValue, 3, m_module->m_typeMgr.getStdType (StdTypeKind_ObjHdrPtr), &srcObjHdrValue);

	Value dstObjHdrValue;
	getDataRefObjHdr (dstValue, &dstObjHdrValue);

	LlvmScopeComment comment (&m_module->m_llvmIrBuilder, "check data pointer scope level");

	Function* checkFunction = m_module->m_functionMgr.getStdFunction (StdFuncKind_CheckScopeLevel);

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
	ASSERT (srcValue.getType ()->getTypeKindFlags () & TypeKindFlagKind_ClassPtr);

	Value dstObjHdrValue;
	getDataRefObjHdr (dstValue, &dstObjHdrValue);

	LlvmScopeComment comment (&m_module->m_llvmIrBuilder, "check class scope level");

	Value ifaceValue;
	m_module->m_llvmIrBuilder.createBitCast (srcValue, m_module->m_typeMgr.getStdType (StdTypeKind_ObjectPtr), &ifaceValue);

	Function* checkFunction = m_module->m_functionMgr.getStdFunction (StdFuncKind_CheckClassPtrScopeLevel);

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
	ASSERT (value.getType ()->getTypeKindFlags () & TypeKindFlagKind_ClassPtr);

	ClassPtrType* ptrType = (ClassPtrType*) value.getType ();
	ClassPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind ();

	if (ptrType->getFlags () & PtrTypeFlagKind_Safe)
		return;

	LlvmScopeComment comment (&m_module->m_llvmIrBuilder, "check null class pointer");

	Value ptrValue;
	m_module->m_llvmIrBuilder.createBitCast (value, m_module->m_typeMgr.getStdType (StdTypeKind_BytePtr), &ptrValue);

	Function* checkFunction = m_module->m_functionMgr.getStdFunction (StdFuncKind_CheckNullPtr);

	m_module->m_llvmIrBuilder.createCall2 (
		checkFunction,
		checkFunction->getType (),
		ptrValue,
		Value (RuntimeErrorKind_NullClassPtr, TypeKind_Int),
		NULL
		);
}

void
OperatorMgr::checkFunctionPtrNull (const Value& value)
{
	ASSERT (value.getType ()->getTypeKindFlags () & TypeKindFlagKind_FunctionPtr);

	FunctionPtrType* ptrType = (FunctionPtrType*) value.getType ();
	FunctionPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind ();

	if (ptrType->getFlags () & PtrTypeFlagKind_Safe)
		return;

	LlvmScopeComment comment (&m_module->m_llvmIrBuilder, "check null function pointer");

	Value ptrValue;

	if (ptrTypeKind == FunctionPtrTypeKind_Thin)
		ptrValue = value;
	else
		m_module->m_llvmIrBuilder.createExtractValue (value, 0, NULL, &ptrValue);

	m_module->m_llvmIrBuilder.createBitCast (ptrValue, m_module->m_typeMgr.getStdType (StdTypeKind_BytePtr), &ptrValue);

	Function* checkFunction = m_module->m_functionMgr.getStdFunction (StdFuncKind_CheckNullPtr);

	m_module->m_llvmIrBuilder.createCall2 (
		checkFunction,
		checkFunction->getType (),
		ptrValue,
		Value (RuntimeErrorKind_NullFunctionPtr, TypeKind_Int),
		NULL
		);
}

void
OperatorMgr::checkFunctionPtrScopeLevel (
	const Value& srcValue,
	const Value& dstValue
	)
{
	ASSERT (srcValue.getType ()->getTypeKindFlags () & TypeKindFlagKind_FunctionPtr);
	FunctionPtrType* ptrType = (FunctionPtrType*) srcValue.getType ();

	if (!ptrType->hasClosure ())
		return;

	Value closureValue;
	m_module->m_llvmIrBuilder.createExtractValue (srcValue, 1, m_module->getSimpleType (StdTypeKind_ObjectPtr), &closureValue);
	checkClassPtrScopeLevel (closureValue, dstValue);
}

void
OperatorMgr::checkPropertyPtrNull (const Value& value)
{
	ASSERT (value.getType ()->getTypeKindFlags () & TypeKindFlagKind_PropertyPtr);

	PropertyPtrType* ptrType = (PropertyPtrType*) value.getType ();
	PropertyPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind ();

	if (ptrType->getFlags () & PtrTypeFlagKind_Safe)
		return;

	LlvmScopeComment comment (&m_module->m_llvmIrBuilder, "check null property pointer");

	Value ptrValue;

	if (ptrTypeKind == PropertyPtrTypeKind_Thin)
		ptrValue = value;
	else
		m_module->m_llvmIrBuilder.createExtractValue (value, 0, NULL, &ptrValue);

	m_module->m_llvmIrBuilder.createBitCast (ptrValue, m_module->m_typeMgr.getStdType (StdTypeKind_BytePtr), &ptrValue);

	Function* checkFunction = m_module->m_functionMgr.getStdFunction (StdFuncKind_CheckNullPtr);
	
	m_module->m_llvmIrBuilder.createCall2 (
		checkFunction,
		checkFunction->getType (),
		ptrValue,
		Value (RuntimeErrorKind_NullPropertyPtr, TypeKind_Int),
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
	m_module->m_llvmIrBuilder.createExtractValue (srcValue, 1, m_module->getSimpleType (StdTypeKind_ObjectPtr), &closureValue);
	checkClassPtrScopeLevel (closureValue, dstValue);
}

//.............................................................................

} // namespace jnc {
