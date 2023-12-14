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

#pragma once

#include "jnc_ct_Value.h"
#include "jnc_ct_BasicBlock.h"
#include "jnc_ct_FunctionType.h"
#include "jnc_ct_LlvmIrInsertPoint.h"

namespace jnc {
namespace ct {

class Module;
class Scope;

#if (LLVM_VERSION < 0x030900)
#	if (_JNC_DEBUG)
#		define LLVM_IR_BUILDER_PRESERVE_NAMES true
#	else
#		define LLVM_IR_BUILDER_PRESERVE_NAMES false
#	endif

typedef llvm::IRBuilder<LLVM_IR_BUILDER_PRESERVE_NAMES> LlvmIrBuilderImpl;
#else
typedef llvm::IRBuilder<> LlvmIrBuilderImpl;
#endif

//..............................................................................

class LlvmIrBuilder {
	friend class Module;

protected:
	Module* m_module;
	LlvmIrBuilderImpl* m_llvmIrBuilder;
	LlvmIrBuilderImpl* m_llvmAllocaIrBuilder;

public:
	LlvmIrBuilder();

	~LlvmIrBuilder() {
		clear();
	}

	operator bool () {
		return m_llvmIrBuilder != NULL;
	}

	Module*
	getModule() {
		return m_module;
	}

	LlvmIrBuilderImpl*
	getLlvmIrBuilder() {
		return m_llvmIrBuilder;
	}

	LlvmIrBuilderImpl*
	getLlvmAllocaIrBuilder() {
		return m_llvmAllocaIrBuilder;
	}

	void
	create();

	void
	clear();

	void
	setAllocaBlock(BasicBlock* block);

	llvm::DebugLoc
	getCurrentDebugLoc() {
		ASSERT(m_llvmIrBuilder);
		return m_llvmIrBuilder->getCurrentDebugLocation();
	}

	void
	setCurrentDebugLoc(const llvm::DebugLoc& llvmDebugLoc) {
		ASSERT(m_llvmIrBuilder);
		m_llvmIrBuilder->SetCurrentDebugLocation(llvmDebugLoc);
	}

	void
	setInstDebugLoc(llvm::Instruction* llvmInst) {
		ASSERT(m_llvmIrBuilder);
		m_llvmIrBuilder->SetInstDebugLocation(llvmInst);
	}

	// branches

	llvm::Instruction*
	getInsertPoint() {
		ASSERT(m_llvmIrBuilder);
		return &*m_llvmIrBuilder->GetInsertPoint();
	}

	void
	setInsertPoint(BasicBlock* block);

	void
	setInsertPoint(llvm::Instruction* llvmInst) {
		ASSERT(m_llvmIrBuilder);
		m_llvmIrBuilder->SetInsertPoint(llvmInst);
	}

	void
	saveInsertPoint(LlvmIrInsertPoint* insertPoint);

	void
	restoreInsertPoint(const LlvmIrInsertPoint& insertPoint);

	bool
	restoreInsertPoint(
		const LlvmIrInsertPoint& insertPoint,
		LlvmIrInsertPoint* prevInsertPoint
	);

	llvm::UnreachableInst*
	createUnreachable() {
		return m_llvmIrBuilder->CreateUnreachable();
	}

	llvm::BranchInst*
	createBr(BasicBlock* block) {
		return m_llvmIrBuilder->CreateBr(block->getLlvmBlock());
	}

	llvm::BranchInst*
	createCondBr(
		const Value& value,
		BasicBlock* trueBlock,
		BasicBlock* falseBlock
	) {
		ASSERT(m_llvmIrBuilder);

		return m_llvmIrBuilder->CreateCondBr(
			value.getLlvmValue(),
			trueBlock->getLlvmBlock(),
			falseBlock->getLlvmBlock()
		);
	}

	llvm::IndirectBrInst*
	createIndirectBr(
		const Value& value,
		BasicBlock** blockArray,
		size_t blockCount
	);

	llvm::SwitchInst*
	createSwitch(
		const Value& value,
		BasicBlock* defaultBlock,
		sl::HashTableIterator<int64_t, BasicBlock*> firstCase,
		size_t caseCount
	);

	llvm::SwitchInst*
	createSwitch(
		const Value& value,
		BasicBlock* defaultBlock,
		int64_t* constArray,
		BasicBlock** blockArray,
		size_t caseCount
	);

	llvm::ReturnInst*
	createRet() {
		ASSERT(m_llvmIrBuilder);
		return m_llvmIrBuilder->CreateRetVoid();
	}

	llvm::ReturnInst*
	createRet(const Value& value) {
		ASSERT(m_llvmIrBuilder);
		return m_llvmIrBuilder->CreateRet(value.getLlvmValue());
	}

	llvm::PHINode*
	createPhi(
		const Value* valueArray,
		BasicBlock** blockArray,
		size_t count,
		Value* resultValue
	);

	llvm::PHINode*
	createPhi(
		const Value& value1,
		BasicBlock* block1,
		const Value& value2,
		BasicBlock* block2,
		Value* resultValue
	);

	// memory access

	llvm::AllocaInst*
	createAlloca(
		Type* type,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmAllocaIrBuilder);

		llvm::AllocaInst* inst = m_llvmAllocaIrBuilder->CreateAlloca(type->getLlvmType(), (llvm::Value*)NULL);
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::LoadInst*
	createLoadImpl(
		llvm::Type* llvmType,
		llvm::Value* llvmValue,
		bool isVolatile = false
	) {
		ASSERT(m_llvmIrBuilder);
#if (LLVM_VERSION_MAJOR < 8)
		return m_llvmIrBuilder->CreateLoad(llvmValue, isVolatile);
#else
#	if (LLVM_VERSION_MAJOR < 17)
		ASSERT(llvmType == llvmValue->getType()->getPointerElementType());
#	endif
		return m_llvmIrBuilder->CreateLoad(llvmType, llvmValue, isVolatile);
#endif
	}

	llvm::LoadInst*
	createLoad(
		const Value& value,
		Type* resultType,
		Value* resultValue,
		bool isVolatile = false
	) {
		llvm::LoadInst* inst = createLoadImpl(resultType->getLlvmType(), value.getLlvmValue(), isVolatile);
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::StoreInst*
	createStore(
		const Value& srcValue,
		const Value& dstValue,
		bool isVolatile = false
	) {
		ASSERT(m_llvmIrBuilder);
		return m_llvmIrBuilder->CreateStore(srcValue.getLlvmValue(), dstValue.getLlvmValue(), isVolatile);
	}

	// member access

	llvm::Value*
	createGepImpl(
		llvm::Type* llvmType,
		llvm::Value* llvmValue,
		const llvm::ArrayRef<llvm::Value*>& llvmIndexArray
	) {
		ASSERT(m_llvmIrBuilder);
#if (LLVM_VERSION < 0x030700)
		return m_llvmIrBuilder->CreateGEP(llvmValue, llvmIndexArray);
#else
#	if (LLVM_VERSION_MAJOR < 17)
		ASSERT(llvmType == llvmValue->getType()->getPointerElementType());
#	endif
		return m_llvmIrBuilder->CreateGEP(llvmType, llvmValue, llvmIndexArray);
#endif
	}

	llvm::Value*
	createGep(
		const Value& value,
		Type* elementType,
		const Value* indexArray,
		size_t indexCount,
		Type* resultType,
		Value* resultValue
	);

	llvm::Value*
	createGep(
		const Value& value,
		Type* elementType,
		const int32_t* indexArray,
		size_t indexCount,
		Type* resultType,
		Value* resultValue
	);

	llvm::Value*
	createGep(
		const Value& value,
		Type* elementType,
		const Value& indexValue,
		Type* resultType,
		Value* resultValue
	) {
		llvm::Value* inst = createGepImpl(elementType->getLlvmType(), value.getLlvmValue(), indexValue.getLlvmValue());
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::Value*
	createGep(
		const Value& value,
		Type* elementType,
		int32_t index,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		Value indexValue;
		indexValue.setConstInt32(index, getSimpleType(TypeKind_Int32, m_module));
		return createGep(value, elementType, indexValue, resultType, resultValue);
	}

	llvm::Value*
	createGep2(
		const Value& value,
		Type* elementType,
		const Value& indexValue1,
		const Value& indexValue2,
		Type* resultType,
		Value* resultValue
	) {
		Value indexArray[] = {
			indexValue1,
			indexValue2,
		};

		return createGep(value, elementType, indexArray, countof(indexArray), resultType, resultValue);
	}

	llvm::Value*
	createGep2(
		const Value& value,
		Type* elementType,
		int32_t index1,
		int32_t index2,
		Type* resultType,
		Value* resultValue
	) {
		Value indexValue1;
		Value indexValue2;
		indexValue1.setConstInt32(index1, getSimpleType(TypeKind_Int32, m_module));
		indexValue2.setConstInt32(index2, getSimpleType(TypeKind_Int32, m_module));
		return createGep2(value, elementType, indexValue1, indexValue2, resultType, resultValue);
	}

	llvm::Value*
	createGep2(
		const Value& value,
		Type* elementType,
		const Value& indexValue2,
		Type* resultType,
		Value* resultValue
	) {
		Value indexValue1;
		indexValue1.setConstInt32(0, getSimpleType(TypeKind_Int32, m_module));
		return createGep2(value, elementType, indexValue1, indexValue2, resultType, resultValue);
	}

	llvm::Value*
	createGep2(
		const Value& value,
		Type* elementType,
		int32_t index2,
		Type* resultType,
		Value* resultValue
	) {
		Value indexValue1;
		Value indexValue2;
		indexValue1.setConstInt32(0, getSimpleType(TypeKind_Int32, m_module));
		indexValue2.setConstInt32(index2, getSimpleType(TypeKind_Int32, m_module));
		return createGep2(value, elementType, indexValue1, indexValue2, resultType, resultValue);
	}

	llvm::Value*
	createExtractValue(
		const Value& value,
		int32_t index,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateExtractValue(value.getLlvmValue(), index);
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::Value*
	createExtractValue(
		const Value& value,
		const int32_t* indexArray,
		size_t indexCount,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateExtractValue(
			value.getLlvmValue(),
			llvm::ArrayRef<uint_t> ((uint_t*)indexArray, indexCount)
		);

		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::Value*
	createInsertValue(
		const Value& aggregateValue,
		const Value& memberValue,
		int32_t index,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateInsertValue(
			aggregateValue.getLlvmValue(),
			memberValue.getLlvmValue(),
			index
		);

		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::Value*
	createInsertValue(
		const Value& aggregateValue,
		const Value& memberValue,
		const int32_t* indexArray,
		size_t indexCount,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateInsertValue(
			aggregateValue.getLlvmValue(),
			memberValue.getLlvmValue(),
			llvm::ArrayRef<uint_t> ((uint_t*)indexArray, indexCount)
		);

		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	// unary

	llvm::Value*
	createNeg_i(
		const Value& opValue,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateNeg(opValue.getLlvmValue());
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::Value*
	createNeg_f(
		const Value& opValue,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateFNeg(opValue.getLlvmValue());
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::Value*
	createNot(
		const Value& opValue,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateNot(opValue.getLlvmValue());
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	// binary

	llvm::Value*
	createAdd_i(
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateAdd(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::Value*
	createAdd_f(
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateFAdd(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::Value*
	createSub_i(
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateSub(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::Value*
	createSub_f(
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateFSub(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::Value*
	createMul_i(
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateMul(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::Value*
	createMul_f(
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateFMul(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::Value*
	createDiv_i(
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateSDiv(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::Value*
	createDiv_u(
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateUDiv(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::Value*
	createDiv_f(
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateFDiv(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::Value*
	createMod_i(
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateSRem(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::Value*
	createMod_u(
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateURem(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::Value*
	createShl(
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateShl(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::Value*
	createShr(
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateLShr(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::Value*
	createAnd(
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue
	) {
		llvm::Value* inst = m_llvmIrBuilder->CreateAnd(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::Value*
	createOr(
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateOr(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	llvm::Value*
	createXor(
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateXor(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, resultType);
		return inst;
	}

	// relational

	llvm::Value*
	createEq_i(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateICmpEQ(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, getSimpleType(TypeKind_Bool, m_module));
		return inst;
	}

	llvm::Value*
	createEq_f(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateFCmpOEQ(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, getSimpleType(TypeKind_Bool, m_module));
		return inst;
	}

	llvm::Value*
	createNe_i(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateICmpNE(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, getSimpleType(TypeKind_Bool, m_module));
		return inst;
	}

	llvm::Value*
	createNe_f(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateFCmpONE(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, getSimpleType(TypeKind_Bool, m_module));
		return inst;
	}

	llvm::Value*
	createLt_i(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateICmpSLT(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, getSimpleType(TypeKind_Bool, m_module));
		return inst;
	}

	llvm::Value*
	createLt_u(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateICmpULT(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, getSimpleType(TypeKind_Bool, m_module));
		return inst;
	}

	llvm::Value*
	createLt_f(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateFCmpOLT(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, getSimpleType(TypeKind_Bool, m_module));
		return inst;
	}

	llvm::Value*
	createLe_i(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateICmpSLE(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, getSimpleType(TypeKind_Bool, m_module));
		return inst;
	}

	llvm::Value*
	createLe_u(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateICmpULE(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, getSimpleType(TypeKind_Bool, m_module));
		return inst;
	}

	llvm::Value*
	createLe_f(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateFCmpOLE(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, getSimpleType(TypeKind_Bool, m_module));
		return inst;
	}

	llvm::Value*
	createGt_i(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateICmpSGT(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, getSimpleType(TypeKind_Bool, m_module));
		return inst;
	}

	llvm::Value*
	createGt_u(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateICmpUGT(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, getSimpleType(TypeKind_Bool, m_module));
		return inst;
	}

	llvm::Value*
	createGt_f(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateFCmpOGT(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, getSimpleType(TypeKind_Bool, m_module));
		return inst;
	}

	llvm::Value*
	createGe_i(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateICmpSGE(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, getSimpleType(TypeKind_Bool, m_module));
		return inst;
	}

	llvm::Value*
	createGe_u(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateICmpUGE(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, getSimpleType(TypeKind_Bool, m_module));
		return inst;
	}

	llvm::Value*
	createGe_f(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateFCmpOGE(opValue1.getLlvmValue(), opValue2.getLlvmValue());
		resultValue->setLlvmValue(inst, getSimpleType(TypeKind_Bool, m_module));
		return inst;
	}

	llvm::AtomicCmpXchgInst*
	createCmpXchg(
		const Value& ptrValue,
		const Value& cmpValue,
		const Value& newValue,
		llvm::AtomicOrdering orderingKind,
		llvm::SynchronizationScope_vn syncKind,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::AtomicCmpXchgInst* inst = m_llvmIrBuilder->CreateAtomicCmpXchg(
			ptrValue.getLlvmValue(),
			cmpValue.getLlvmValue(),
			newValue.getLlvmValue(),
#if (LLVM_VERSION_MAJOR >= 13)
            llvm::MaybeAlign(),
#endif
			orderingKind,
#if (LLVM_VERSION >= 0x030500)
			orderingKind,
#endif
			syncKind
		);

		resultValue->setLlvmValue(inst, newValue.getType());
		return inst;
	}

#if (LLVM_VERSION >= 0x030500)
	llvm::AtomicCmpXchgInst*
	createCmpXchg(
		const Value& ptrValue,
		const Value& cmpValue,
		const Value& newValue,
		llvm::AtomicOrdering successOrderingKind,
		llvm::AtomicOrdering failureOrderingKind,
		llvm::SynchronizationScope_vn syncKind,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::AtomicCmpXchgInst* inst = m_llvmIrBuilder->CreateAtomicCmpXchg(
			ptrValue.getLlvmValue(),
			cmpValue.getLlvmValue(),
			newValue.getLlvmValue(),
#if (LLVM_VERSION_MAJOR >= 13)
            llvm::MaybeAlign(),
#endif
			successOrderingKind,
			failureOrderingKind,
			syncKind
		);

		resultValue->setLlvmValue(inst, newValue.getType());
		return inst;
	}
#endif

	llvm::AtomicRMWInst*
	createRmw(
		llvm::AtomicRMWInst::BinOp opKind,
		const Value& ptrValue,
		const Value& newValue,
		llvm::AtomicOrdering orderingKind,
		llvm::SynchronizationScope_vn syncKind,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::AtomicRMWInst* inst = m_llvmIrBuilder->CreateAtomicRMW(
			opKind,
			ptrValue.getLlvmValue(),
			newValue.getLlvmValue(),
#if (LLVM_VERSION_MAJOR >= 13)
            llvm::MaybeAlign(),
#endif
			orderingKind,
			syncKind
		);

		resultValue->setLlvmValue(inst, newValue.getType());
		return inst;
	}

	// casts

	llvm::Value*
	createBitCast(
		const Value& opValue,
		llvm::Type* type,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);
		llvm::Value* inst = m_llvmIrBuilder->CreateBitCast(opValue.getLlvmValue(), type);
		resultValue->setLlvmValue(inst, NULL);
		return inst;
	}

	llvm::Value*
	createBitCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateBitCast(opValue.getLlvmValue(), type->getLlvmType());
		resultValue->setLlvmValue(inst, type);
		return inst;
	}

	llvm::Value*
	createIntToPtr(
		const Value& opValue,
		Type* type,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateIntToPtr(opValue.getLlvmValue(), type->getLlvmType());
		resultValue->setLlvmValue(inst, type);
		return inst;
	}

	llvm::Value*
	createPtrToInt(
		const Value& opValue,
		Type* type,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreatePtrToInt(opValue.getLlvmValue(), type->getLlvmType());
		resultValue->setLlvmValue(inst, type);
		return inst;
	}

	llvm::Value*
	createTrunc_i(
		const Value& opValue,
		Type* type,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateTrunc(opValue.getLlvmValue(), type->getLlvmType());
		resultValue->setLlvmValue(inst, type);
		return inst;
	}

	llvm::Value*
	createTrunc_f(
		const Value& opValue,
		Type* type,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateFPTrunc(opValue.getLlvmValue(), type->getLlvmType());
		resultValue->setLlvmValue(inst, type);
		return inst;
	}

	llvm::Value*
	createExt_i(
		const Value& opValue,
		Type* type,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateSExt(opValue.getLlvmValue(), type->getLlvmType());
		resultValue->setLlvmValue(inst, type);
		return inst;
	}

	llvm::Value*
	createExt_u(
		const Value& opValue,
		Type* type,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateZExt(opValue.getLlvmValue(), type->getLlvmType());
		resultValue->setLlvmValue(inst, type);
		return inst;
	}

	llvm::Value*
	createExt_f(
		const Value& opValue,
		Type* type,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateFPExt(opValue.getLlvmValue(), type->getLlvmType());
		resultValue->setLlvmValue(inst, type);
		return inst;
	}

	llvm::Value*
	createIntToFp(
		const Value& opValue,
		Type* type,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateSIToFP(opValue.getLlvmValue(), type->getLlvmType());
		resultValue->setLlvmValue(inst, type);
		return inst;
	}

	llvm::Value*
	createIntToFp_u(
		const Value& opValue,
		Type* type,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateUIToFP(opValue.getLlvmValue(), type->getLlvmType());
		resultValue->setLlvmValue(inst, type);
		return inst;
	}

	llvm::Value*
	createFpToInt(
		const Value& opValue,
		Type* type,
		Value* resultValue
	) {
		ASSERT(m_llvmIrBuilder);

		llvm::Value* inst = m_llvmIrBuilder->CreateFPToSI(opValue.getLlvmValue(), type->getLlvmType());
		resultValue->setLlvmValue(inst, type);
		return inst;
	}

	// calls

	llvm::CallInst*
	createCall(
		const Value& calleeValue,
		FunctionType* functionType,
		llvm::Value* const* llvmArgValueArray,
		size_t argCount,
		Type* resultType, // not necessarily functionType->getReturnType()
		Value* resultValue
	);

	llvm::CallInst*
	createCall(
		const Value& calleeValue,
		FunctionType* functionType,
		const Value* argArray,
		size_t argCount,
		Type* resultType, // not necessarily functionType->getReturnType()
		Value* resultValue
	);

	llvm::CallInst*
	createCall(
		const Value& calleeValue,
		FunctionType* functionType,
		const Value* argArray,
		size_t argCount,
		Value* resultValue
	) {
		return createCall(calleeValue, functionType, argArray, argCount, functionType->getReturnType(), resultValue);
	}

	llvm::CallInst*
	createCall(
		const Value& calleeValue,
		FunctionType* functionType,
		const sl::BoxList<Value>& argValueList,
		Type* resultType, // not necessarily functionType->getReturnType()
		Value* resultValue
	);

	llvm::CallInst*
	createCall(
		const Value& calleeValue,
		FunctionType* functionType,
		const sl::BoxList<Value>& argValueList,
		Value* resultValue
	) {
		return createCall(calleeValue, functionType, argValueList, functionType->getReturnType(), resultValue);
	}

	// the following functions are convenient but be sure they don't need
	// special handing by CallConv (e.g. struct-ret, arg coercion etc)

	llvm::CallInst*
	createCall(
		const Value& calleeValue,
		FunctionType* functionType,
		Value* resultValue
	) {
		return createCall(calleeValue, functionType, (llvm::Value* const*)NULL, 0, functionType->getReturnType(), resultValue);
	}

	llvm::CallInst*
	createCall(
		const Value& calleeValue,
		FunctionType* functionType,
		const Value& argValue,
		Value* resultValue
	) {
		return createCall(calleeValue, functionType, &argValue, 1, resultValue);
	}

	llvm::CallInst*
	createCall2(
		const Value& calleeValue,
		FunctionType* functionType,
		const Value& argValue1,
		const Value& argValue2,
		Value* resultValue
	) {
		Value argArray[] = {
			argValue1,
			argValue2,
		};

		return createCall(calleeValue, functionType, argArray, countof(argArray), resultValue);
	}

	llvm::CallInst*
	createCall3(
		const Value& calleeValue,
		FunctionType* functionType,
		const Value& argValue1,
		const Value& argValue2,
		const Value& argValue3,
		Value* resultValue
	) {
		Value argArray[] = {
			argValue1,
			argValue2,
			argValue3,
		};

		return createCall(calleeValue, functionType, argArray, countof(argArray), resultValue);
	}

	llvm::CallInst*
	createCall4(
		const Value& calleeValue,
		FunctionType* functionType,
		const Value& argValue1,
		const Value& argValue2,
		const Value& argValue3,
		const Value& argValue4,
		Value* resultValue
	) {
		Value argArray[] = {
			argValue1,
			argValue2,
			argValue3,
			argValue4,
		};

		return createCall(calleeValue, functionType, argArray, countof(argArray), resultValue);
	}

	// function & property pointer operations

	void
	createClosureFunctionPtr(
		const Value& ptrValue,
		const Value& closureValue,
		FunctionPtrType* resultType,
		Value* resultValue
	);

	void
	createClosurePropertyPtr(
		const Value& ptrValue,
		const Value& closureValue,
		PropertyPtrType* resultType,
		Value* resultValue
	);

	// typed attributes (sret, byval)

	void
	addTypedAttribute(
		llvm::Function* llvmFunction,
		unsigned i,
		llvm::Attribute::AttrKind attrKind,
		Type* type
	);

	void
	addTypedAttribute(
		llvm::CallInst* llvmCallInst,
		unsigned i,
		llvm::Attribute::AttrKind attrKind,
		Type* type
	);
};

//..............................................................................

sl::String
getLlvmInstructionString(llvm::Instruction* llvmInst);

//..............................................................................

} // namespace ct
} // namespace jnc
