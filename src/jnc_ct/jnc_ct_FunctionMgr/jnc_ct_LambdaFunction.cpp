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
#include "jnc_ct_LambdaFunction.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

// lambdas are compiled in-place (nested into the compilation of the outer function)
// which allows discovering captures while compiling the lambda body:
//
// - by-value captures ([a, b]) are snapshots taken at the creation site and stored
//   directly in the closure (writes persist across calls);
// - every other outer local referenced from the body is captured by-ref -- a safe
//   pointer to the outer variable (taking the address of an outer stack variable
//   lifts it to the heap, so the closure can outlive the frame);
// - the outer 'this' is captured by-value on the first use (for struct methods, as
//   a safe fat pointer);
// - the body accesses each capture via a variable in the lambda's scope; until the
//   closure is laid out, its address is a placeholder alloca (created on the first use)

//..............................................................................

void
LambdaCapture::finalize(const Value& closurePtrValue) {
	ASSERT(closurePtrValue.getType()->getTypeKindFlags() & TypeKindFlag_ClassPtr);

	Module* module = m_variable->getModule();
	ClassType* closureType = ((ClassPtrType*)closurePtrValue.getType())->getTargetType();

	Value fieldPtrValue;
	module->m_operatorMgr.getClassField(
		closurePtrValue,
		closureType,
		m_field,
		NULL,
		&fieldPtrValue
	);

	llvm::Instruction* llvmPlaceholder = (llvm::Instruction*)m_variable->getLlvmValue();
	llvmPlaceholder->replaceAllUsesWith(fieldPtrValue.getLlvmValue());
	llvmPlaceholder->eraseFromParent();
}

//..............................................................................

Variable*
LambdaFunction::createByRefCaptureVariable(Variable* variable) {
	Type* type = variable->getType();
	Type* ptrType;
	if (variable->getFlags() & VariableFlag_LambdaByRefCapture)
		ptrType = type; // already is a pointer
	else {
		uint_t ptrTypeFlags = PtrTypeFlag_Safe | (variable->getPtrTypeFlags() & PtrTypeFlag_ConstKindMask);
		ptrType = type->getTypeKind() == TypeKind_Class ? // a class variable is the object itself
			(Type*)((ClassType*)type)->getClassPtrType(ptrTypeFlags) :
			type->getDataPtrType(ptrTypeFlags);
	}

	LambdaByRefCapture capture;
	capture.m_outerVariable = variable;
	capture.m_variable = m_module->m_variableMgr.createLambdaCaptureVariable<VariableFlag_LambdaByRefCapture>(
		variable->getName(),
		ptrType
	);

	capture.m_variable->copySrcPos(variable);

	// add to the function scope, so subsequent lookups find the capture (the body
	// opens its own nested scope, so the function scope never clashes with body locals)

	bool result = m_scope->addItem(capture.m_variable);
	ASSERT(result);

	m_byRefCaptureArray.append(capture);
	return capture.m_variable;
}

Variable*
LambdaFunction::createOuterThisVariable() {
	ASSERT(!m_outerThisCapture.m_variable);

	m_outerThisCapture.m_variable = m_module->m_variableMgr.createLambdaCaptureVariable<VariableFlag_LambdaByValCapture>(
		sl::StringRef(),
		m_outerThisType,
		ConstKind_Const // can't change 'this'
	);

	return m_outerThisCapture.m_variable;
}

void
LambdaFunction::addByValCapturesToScope() {
	ASSERT(m_module->hasCodeGen()); // see Parser::createLambda

	// [] variables are created in the outer scope -- re-parent them (otherwise, they look like
	// foreign locals and get captured by-ref); name clashes are checked in Parser::createLambda

	sl::StringHashTableIterator<LambdaByValCapture> it = m_byValCaptureMap.getHead();
	for (; it; it++) {
		Variable* variable = it->m_value.m_variable;
		variable->m_scope = m_scope;
		bool result = m_scope->addItem(variable);
		ASSERT(result);
	}
}

bool
LambdaFunction::compile() {
	// lay out the type while still in the outer function's context
	// (e.g., user array sizes are parsed as const expressions)

	bool result = m_type->ensureLayout();
	if (!result)
		return false;

	m_flags |= FunctionFlag_CompilePending; // compiled right here, never via the module's compile array

	size_t namespaceStackDepth = m_module->m_namespaceMgr.getNamespaceStackDepth();

	FunctionCompileContext context;
	m_module->m_functionMgr.captureFunctionCompileContext(&context);

	result = Function::compile();

	// a failed (or error-recovered) compilation may leave namespaces open

	m_module->m_namespaceMgr.closeNamespaces(namespaceStackDepth);
	m_module->m_functionMgr.restoreFunctionCompileContext(&context);

	if (!result || !m_module->hasCodeGen()) // codegen is dropped after the first error
		return result;

	// variables get their llvm allocas on the first use

	sl::StringHashTableIterator<LambdaByValCapture> byValIt = m_byValCaptureMap.getHead();
	for (; byValIt; byValIt++)
		if (!byValIt->m_value.m_variable->hasLlvmValue()) {
			err::setError("'%s' is captured but never used", byValIt->getKey().sz());
			lex::pushSrcPosError(m_parentUnit->getFilePath(), byValIt->m_value.m_variable->getPos());
			return false;
		}

	Variable* outerThisVariable = m_outerThisCapture.m_variable;
	size_t byRefCaptureCount = m_byRefCaptureArray.getCount();
	size_t captureCount = m_byValCaptureMap.getCount() + byRefCaptureCount + (outerThisVariable ? 1 : 0);
	if (!captureCount)
		return true;

	// a closure class is only needed for multiple captures (or a struct 'this')

	LambdaByRefCapture* byRefCaptureArray = m_byRefCaptureArray.p();
	bool isThisOnly = captureCount == 1 && outerThisVariable && m_outerThisType->getTypeKind() == TypeKind_ClassPtr;
	if (!isThisOnly) {
		m_closureType = m_module->m_typeMgr.createInternalClassType(m_module->createUniqueName("Lambda"));

		if (outerThisVariable)
			m_outerThisCapture.m_field = m_closureType->createField("this", outerThisVariable->getType());

		for (byValIt = m_byValCaptureMap.getHead(); byValIt; byValIt++)
			byValIt->m_value.m_field = m_closureType->createField(byValIt->getKey(), byValIt->m_value.m_variable->getType());

		for (size_t i = 0; i < byRefCaptureCount; i++) {
			Variable* variable = byRefCaptureArray[i].m_variable;
			byRefCaptureArray[i].m_field = m_closureType->createField(variable->getName(), variable->getType());
		}

		result = m_closureType->require();
		if (!result)
			return false;
	}

	// replace field placeholders on top of the lambda's prologue
	// (don't leak the outer function's debug location into the lambda)

	LlvmIrInsertPoint prevInsertPoint;
	llvm::DebugLoc prevDebugLoc = m_module->m_llvmIrBuilder.getCurrentDebugLoc();
	m_module->m_llvmIrBuilder.saveInsertPoint(&prevInsertPoint);
	m_module->m_llvmIrBuilder.setInsertPoint(&*m_prologueBlock->getLlvmBlock()->begin());
	m_module->m_llvmIrBuilder.setCurrentDebugLoc(llvm::DebugLoc());

	Value thisArgValue = m_type->getCallConv()->getThisArgValue(this);

	if (isThisOnly) {
		// the outer class 'this' is passed as the closure;
		// the placeholder becomes a plain local holding the outer 'this'

		Value outerThisValue;
		m_module->m_llvmIrBuilder.createBitCast(thisArgValue, m_outerThisType, &outerThisValue);
		m_module->m_llvmIrBuilder.createStore(outerThisValue, Value(outerThisVariable->getLlvmValue(), m_outerThisType->getDataPtrType(DataPtrKind_Thin)));
	} else {
		Value closurePtrValue;
		m_module->m_llvmIrBuilder.createBitCast(thisArgValue, m_closureType->getClassPtrType(PtrTypeFlag_Safe), &closurePtrValue); // never null

		if (outerThisVariable)
			m_outerThisCapture.finalize(closurePtrValue);

		for (byValIt = m_byValCaptureMap.getHead(); byValIt; byValIt++)
			byValIt->m_value.finalize(closurePtrValue);

		for (size_t i = 0; i < byRefCaptureCount; i++)
			byRefCaptureArray[i].finalize(closurePtrValue);
	}

	m_module->m_llvmIrBuilder.restoreInsertPoint(prevInsertPoint);
	m_module->m_llvmIrBuilder.setCurrentDebugLoc(prevDebugLoc);
	return true;
}

bool
LambdaFunction::createFunctionPtrValue(Value* resultValue) {
	bool result;

	FunctionPtrType* ptrType = m_type->getShortType()->getFunctionPtrType();

	if (!m_module->hasCodeGen()) {
		resultValue->setType(ptrType);
		return true;
	}

	Value closureValue;
	if (m_closureType) {
		result = m_module->m_operatorMgr.newOperator(m_closureType, &closureValue);
		if (!result)
			return false;

		if (m_outerThisCapture.m_variable) {
			Value thisValue;
			Value fieldValue;
			result =
				m_module->m_operatorMgr.getThisValue(&thisValue) &&
				m_module->m_operatorMgr.getClassField(closureValue, m_closureType, m_outerThisCapture.m_field, NULL, &fieldValue) &&
				m_module->m_operatorMgr.binaryOperator(BinOpKind_Assign, fieldValue, thisValue);

			if (!result)
				return false;
		}

		sl::StringHashTableIterator<LambdaByValCapture> byValIt = m_byValCaptureMap.getHead();
		for (; byValIt; byValIt++) {
			Value fieldValue;
			result =
				m_module->m_operatorMgr.getClassField(closureValue, m_closureType, byValIt->m_value.m_field, NULL, &fieldValue) &&
				m_module->m_operatorMgr.binaryOperator(BinOpKind_Assign, fieldValue, byValIt->m_value.m_value);

			if (!result)
				return false;
		}

		size_t byRefCaptureCount = m_byRefCaptureArray.getCount();
		for (size_t i = 0; i < byRefCaptureCount; i++) {
			const LambdaByRefCapture* capture = &m_byRefCaptureArray[i];

			Value ptrValue;
			Value fieldValue;
			result =
				ptrValue.trySetVariable(capture->m_outerVariable) && // a nested lambda gets captured here
				m_module->m_operatorMgr.unaryOperator(UnOpKind_Addr, &ptrValue) &&
				m_module->m_operatorMgr.getClassField(closureValue, m_closureType, capture->m_field, NULL, &fieldValue) &&
				m_module->m_operatorMgr.binaryOperator(BinOpKind_Assign, fieldValue, ptrValue);

			if (!result)
				return false;
		}
	} else if (m_outerThisCapture.m_variable) { // the outer class 'this' is the closure
		result =
			m_module->m_operatorMgr.getThisValue(&closureValue) &&
			m_module->m_operatorMgr.prepareOperand(&closureValue); // in a nested lambda, it's a capture variable

		if (!result)
			return false;
	} else
		closureValue = m_module->m_typeMgr.getStdType(StdType_AbstractClassPtr)->getZeroValue();

	m_module->m_llvmIrBuilder.createClosureFunctionPtr(m_llvmFunction, closureValue, ptrType, resultValue);
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
