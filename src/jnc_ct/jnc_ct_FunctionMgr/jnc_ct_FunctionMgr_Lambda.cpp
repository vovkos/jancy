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
#include "jnc_ct_FunctionMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

// lambdas are compiled in-place (nested into the compilation of the outer function)
// which allows discovering captures while compiling the lambda body:
//
// - every outer local referenced from the body becomes a closure field; until
//   the closure is laid out, fields are accessed via placeholder allocas;
// - by-value captures ([a, b]) are snapshots stored directly in the closure (and
//   accessed just like class fields, i.e. writes persist across calls);
// - by-ref captures store a safe pointer to the outer variable (taking the address
//   of an outer stack variable lifts it to the heap, so the closure can outlive the frame);
// - the outer 'this' is captured (by value) on the first use of 'this' in the body
//   (for struct methods, as a safe fat pointer)

bool
FunctionMgr::compileLambda(
	LambdaFunction* function,
	Value* resultValue
) {
	// lay out the type while still in the outer function's context
	// (e.g., user array sizes are parsed as const expressions)

	bool result = function->getType()->ensureLayout();
	if (!result)
		return false;

	// don't use the outer 'this' value itself -- in a nested lambda, it's captured on demand

	bool isNested = m_currentFunction->m_functionKind == FunctionKind_Lambda;
	Type* outerThisType =
		isNested ? ((LambdaFunction*)m_currentFunction)->m_outerThisType :
		m_thisValue ? m_thisValue.getType() :
		NULL;

	if (outerThisType && outerThisType->getTypeKind() == TypeKind_DataPtr) { // struct method: store a safe fat pointer
		DataPtrType* ptrType = (DataPtrType*)outerThisType;
		outerThisType = ptrType->getTargetType()->getDataPtrType(
			TypeKind_DataPtr,
			(ptrType->getFlags() & PtrTypeFlag_ConstKindMask) | PtrTypeFlag_Safe
		);
	}

	function->m_outerThisType = outerThisType;
	function->m_flags |= FunctionFlag_CompilePending; // compiled right here, never via the module's compile array

	size_t namespaceStackDepth = m_module->m_namespaceMgr.getNamespaceStackDepth();

	FunctionCompileContext context;
	captureFunctionCompileContext(&context);
	result = function->compile();

	// a failed (or error-recovered) compilation may leave namespaces open

	m_module->m_namespaceMgr.closeNamespaces(namespaceStackDepth);
	m_currentFunction = NULL;
	restoreFunctionCompileContext(&context);

	if (!result)
		return false;

	if (m_module->hasCodeGen()) { // otherwise, captures are not tracked
		sl::HashTableIterator<Variable*, LambdaCapture> it = function->m_captureMap.getHead();
		for (; it; it++)
			if (!(it->m_value.m_flags & LambdaCaptureFlag_Used)) {
				err::setError("'%s' is captured but never used", it->getKey()->getName().sz());
				lex::pushSrcPosError(function->getParentUnit()->getFilePath(), it->m_value.m_pos);
				return false;
			}
	}

	FunctionPtrType* ptrType = function->getType()->getShortType()->getFunctionPtrType();

	if (!m_module->hasCodeGen()) { // codegen is dropped after the first error
		resultValue->setType(ptrType);
		return true;
	}

	size_t captureCount = function->m_captureMap.getCount();
	sl::HashTableIterator<Variable*, LambdaCapture> captureIt = function->m_captureMap.getHead();
	bool isThisOnly =
		captureCount == 1 &&
		!captureIt->getKey() &&
		outerThisType->getTypeKind() == TypeKind_ClassPtr;

	// a closure class is only needed for multiple captures (or a struct 'this')

	ClassType* closureType = NULL;
	if (captureCount && !isThisOnly) {
		closureType = m_module->m_typeMgr.createInternalClassType(m_module->createUniqueName("Lambda"));

		sl::HashTableIterator<Variable*, LambdaCapture> it = function->m_captureMap.getHead();
		for (; it; it++)
			it->m_value.m_field = closureType->createField(
				it->getKey() ? it->getKey()->getName() : sl::StringRef("this"),
				it->m_value.m_fieldType
			);

		result = closureType->require();
		if (!result)
			return false;
	}

	// replace field placeholders on top of the lambda's prologue
	// (don't leak the outer function's debug location into the lambda)

	LlvmIrInsertPoint prevInsertPoint;
	llvm::DebugLoc prevDebugLoc = m_module->m_llvmIrBuilder.getCurrentDebugLoc();
	m_module->m_llvmIrBuilder.saveInsertPoint(&prevInsertPoint);
	m_module->m_llvmIrBuilder.setInsertPoint(&*function->getPrologueBlock()->getLlvmBlock()->begin());
	m_module->m_llvmIrBuilder.setCurrentDebugLoc(llvm::DebugLoc());

	Value thisArgValue = function->getType()->getCallConv()->getThisArgValue(function);

	if (isThisOnly) {
		// the outer class 'this' is passed as the closure;
		// the placeholder becomes a plain local holding the outer 'this'

		Value outerThisValue;
		m_module->m_llvmIrBuilder.createBitCast(thisArgValue, outerThisType, &outerThisValue);
		m_module->m_llvmIrBuilder.createStore(outerThisValue, Value(captureIt->m_value.m_llvmFieldAlloca, outerThisType->getDataPtrType(DataPtrKind_Thin)));
	} else if (closureType) {
		Value closurePtrValue;
		m_module->m_llvmIrBuilder.createBitCast(thisArgValue, closureType->getClassPtrType(), &closurePtrValue);
		StructType* ifaceStructType = closureType->getIfaceStructType();

		sl::HashTableIterator<Variable*, LambdaCapture> it = function->m_captureMap.getHead();
		for (; it; it++) {
			LambdaCapture* capture = &it->m_value;

			Value fieldPtrValue;
			m_module->m_llvmIrBuilder.createGep2(
				closurePtrValue,
				ifaceStructType,
				capture->m_field->getLlvmIndex(),
				NULL,
				&fieldPtrValue
			);

			capture->m_llvmFieldAlloca->replaceAllUsesWith(fieldPtrValue.getLlvmValue());
			capture->m_llvmFieldAlloca->eraseFromParent();
			capture->m_llvmFieldAlloca = NULL;
		}
	}

	m_module->m_llvmIrBuilder.restoreInsertPoint(prevInsertPoint);
	m_module->m_llvmIrBuilder.setCurrentDebugLoc(prevDebugLoc);

	// back in the outer function

	Value closureValue;
	if (!captureCount)
		closureValue = m_module->m_typeMgr.getStdType(StdType_AbstractClassPtr)->getZeroValue();
	else if (isThisOnly)
		closureValue = isNested ? getLambdaThisValue((LambdaFunction*)m_currentFunction) : m_thisValue;
	else {
		result = m_module->m_operatorMgr.newOperator(closureType, &closureValue);
		if (!result)
			return false;

		sl::HashTableIterator<Variable*, LambdaCapture> it = function->m_captureMap.getHead();
		for (; it; it++) {
			Value srcValue;
			if (!it->getKey()) {
				srcValue = isNested ? getLambdaThisValue((LambdaFunction*)m_currentFunction) : m_thisValue;
				ASSERT(srcValue);
			} else {
				result = m_module->m_variableMgr.getVariableValue(it->getKey(), &srcValue); // nested lambdas
				if (!result)
					return false;

				if (it->m_value.m_flags & LambdaCaptureFlag_Ptr) {
					result = m_module->m_operatorMgr.unaryOperator(UnOpKind_Addr, &srcValue);
					if (!result)
						return false;
				}
			}

			Value fieldValue;
			result =
				m_module->m_operatorMgr.getClassField(closureValue, closureType, it->m_value.m_field, NULL, &fieldValue) &&
				m_module->m_operatorMgr.binaryOperator(BinOpKind_Assign, fieldValue, srcValue);

			if (!result)
				return false;
		}
	}

	m_module->m_llvmIrBuilder.createClosureFunctionPtr(function->getLlvmFunction(), closureValue, ptrType, resultValue);
	return true;
}

Value
FunctionMgr::getLambdaThisValue(LambdaFunction* function) {
	ASSERT(function == m_currentFunction);

	if (function->m_outerThisValue || !function->m_outerThisType)
		return function->m_outerThisValue;

	if (!m_module->hasCodeGen()) { // codegen is dropped after the first error
		function->m_outerThisValue = function->m_outerThisType;
		return function->m_outerThisValue;
	}

	Value fieldPtrValue;
	m_module->m_llvmIrBuilder.createAlloca(function->m_outerThisType, NULL, &fieldPtrValue);

	LlvmIrInsertPoint prevInsertPoint;
	bool isInsertPointChanged = m_module->m_llvmIrBuilder.restoreInsertPoint(
		function->m_captureInsertPoint,
		&prevInsertPoint
	);

	m_module->m_llvmIrBuilder.createLoad(fieldPtrValue, function->m_outerThisType, &function->m_outerThisValue);

	if (isInsertPointChanged)
		m_module->m_llvmIrBuilder.restoreInsertPoint(prevInsertPoint);

	LambdaCapture* capture = &function->m_captureMap.visit(NULL)->m_value; // NULL key -- the outer 'this'
	capture->m_fieldType = function->m_outerThisType;
	capture->m_llvmFieldAlloca = (llvm::AllocaInst*)fieldPtrValue.getLlvmValue();
	capture->m_flags = LambdaCaptureFlag_Used;
	return function->m_outerThisValue;
}

//..............................................................................

} // namespace ct
} // namespace jnc
