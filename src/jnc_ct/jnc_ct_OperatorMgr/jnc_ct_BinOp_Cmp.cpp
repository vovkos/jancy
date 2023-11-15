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
#include "jnc_ct_BinOp_Cmp.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

Type*
getPtrCmpOperatorOperandType(
	const Value& opValue1,
	const Value& opValue2
) {
	// TODO: check that we don't compare pointers of different typekinds

	return opValue1.getType()->getModule()->m_typeMgr.getPrimitiveType(TypeKind_IntPtr);
}

bool
cmpStringOperator(
	BinOpKind opKind,
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
) {
	Module* module = opValue1.getType()->getModule();
	Function* func = module->m_functionMgr.getStdFunction(StdFunc_StringCmp);
	Value cmpValue;
	Value zeroValue = func->getType()->getReturnType()->getZeroValue();

	return
		module->m_operatorMgr.callOperator(func, opValue1, opValue2, &cmpValue) &&
		module->m_operatorMgr.binaryOperator(opKind, cmpValue, zeroValue, resultValue);
}

//..............................................................................

bool
BinOp_Eq::llvmOpString(
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
) {
	Function* func = m_module->m_functionMgr.getStdFunction(StdFunc_StringEq);
	return m_module->m_operatorMgr.callOperator(func, opValue1, opValue2, resultValue);
}

llvm::Value*
BinOp_Eq::llvmOpInt(
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue,
	bool isUnsigned
) {
	return m_module->m_llvmIrBuilder.createEq_i(opValue1, opValue2, resultValue);
}

llvm::Value*
BinOp_Eq::llvmOpFp(
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
) {
	return m_module->m_llvmIrBuilder.createEq_f(opValue1, opValue2, resultValue);
}

//..............................................................................

bool
BinOp_Ne::llvmOpString(
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
) {
	Function* func = m_module->m_functionMgr.getStdFunction(StdFunc_StringEq);
	Value eqValue;
	return
		m_module->m_operatorMgr.callOperator(func, opValue1, opValue2, &eqValue) &&
		m_module->m_operatorMgr.unaryOperator(UnOpKind_LogNot, eqValue, resultValue);
}

llvm::Value*
BinOp_Ne::llvmOpInt(
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue,
	bool isUnsigned
) {
	return m_module->m_llvmIrBuilder.createNe_i(opValue1, opValue2, resultValue);
}

llvm::Value*
BinOp_Ne::llvmOpFp(
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
) {
	return m_module->m_llvmIrBuilder.createNe_f(opValue1, opValue2, resultValue);
}

//..............................................................................

llvm::Value*
BinOp_Lt::llvmOpInt(
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue,
	bool isUnsigned
) {
	return isUnsigned ?
		m_module->m_llvmIrBuilder.createLt_u(opValue1, opValue2, resultValue) :
		m_module->m_llvmIrBuilder.createLt_i(opValue1, opValue2, resultValue);
}

llvm::Value*
BinOp_Lt::llvmOpFp(
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
) {
	return m_module->m_llvmIrBuilder.createLt_f(opValue1, opValue2, resultValue);
}

//..............................................................................

llvm::Value*
BinOp_Le::llvmOpInt(
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue,
	bool isUnsigned
) {
	return isUnsigned ?
		m_module->m_llvmIrBuilder.createLe_u(opValue1, opValue2, resultValue) :
		m_module->m_llvmIrBuilder.createLe_i(opValue1, opValue2, resultValue);
}

llvm::Value*
BinOp_Le::llvmOpFp(
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
) {
	return m_module->m_llvmIrBuilder.createLe_f(opValue1, opValue2, resultValue);
}

//..............................................................................

llvm::Value*
BinOp_Gt::llvmOpInt(
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue,
	bool isUnsigned
) {
	return isUnsigned ?
		m_module->m_llvmIrBuilder.createGt_u(opValue1, opValue2, resultValue) :
		m_module->m_llvmIrBuilder.createGt_i(opValue1, opValue2, resultValue);
}

llvm::Value*
BinOp_Gt::llvmOpFp(
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
) {
	return m_module->m_llvmIrBuilder.createGt_f(opValue1, opValue2, resultValue);
}

//..............................................................................

llvm::Value*
BinOp_Ge::llvmOpInt(
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue,
	bool isUnsigned
) {
	return isUnsigned ?
		m_module->m_llvmIrBuilder.createGe_u(opValue1, opValue2, resultValue) :
		m_module->m_llvmIrBuilder.createGe_i(opValue1, opValue2, resultValue);
}

llvm::Value*
BinOp_Ge::llvmOpFp(
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
) {
	return m_module->m_llvmIrBuilder.createGe_f(opValue1, opValue2, resultValue);
}

//..............................................................................

} // namespace ct
} // namespace jnc
