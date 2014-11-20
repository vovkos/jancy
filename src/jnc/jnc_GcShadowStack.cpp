#include "pch.h"
#include "jnc_GcShadowStack.h"
#include "jnc_Module.h"

using namespace llvm;

GetElementPtrInst *
createGEP(
	LLVMContext &context,
	IRBuilder<> &B,
	Value *basePtr,
	int idx,
	int idx2,
	const char *name
	)
{
	Value *indices[] =
	{
		ConstantInt::get(Type::getInt32Ty(context), 0),
		ConstantInt::get(Type::getInt32Ty(context), idx),
		ConstantInt::get(Type::getInt32Ty(context), idx2)
	};

	Value* val = B.CreateGEP(basePtr, indices, name);

	assert(isa<GetElementPtrInst>(val) && "Unexpected folded constant");

	return dyn_cast<GetElementPtrInst>(val);
}

GetElementPtrInst *
createGEP (
	LLVMContext &context,
	IRBuilder<> &B,
	Value *basePtr,
	int idx,
	const char *name)
{
	Value *indices[] =
	{
		ConstantInt::get(Type::getInt32Ty(context), 0),
		ConstantInt::get(Type::getInt32Ty(context), idx)
	};

	Value *val = B.CreateGEP(basePtr, indices, name);

	assert(isa<GetElementPtrInst>(val) && "Unexpected folded constant");

	return dyn_cast<GetElementPtrInst>(val);
}

namespace jnc {

//.............................................................................

void
registerGcShadowStack (int)
{
	static llvm::GCRegistry::Add <GcShadowStack> shadowStack (
		"jnc-shadow-stack",
		"Re-engineered shadow stack for Jancy GC"
		);
}

//.............................................................................

/// EscapeEnumerator - This is a little algorithm to find all escape points
/// from a function so that "finally"-style code can be inserted. In addition
/// to finding the existing return and unwind instructions, it also (if
/// necessary) transforms any call instructions into invokes and sends them to
/// a landing pad.
///
/// It's wrapped up in a state machine using the same transform C# uses for
/// 'yield return' enumerators, This transform allows it to be non-allocating.

class EscapeEnumerator
{
	llvm::Function& F;
	const char* cleanupBBName;

	// State.
	int state;
	llvm::Function::iterator stateBB, stateE;
	llvm::IRBuilder<> builder;

public:
	EscapeEnumerator (llvm::Function &F, const char* N = "cleanup");

	llvm::IRBuilder<>*
	next ();
};

//.............................................................................

GcShadowStack::GcShadowStack ()
{
	m_module = NULL;
	InitRoots = true;
	CustomRoots = true;
}

bool
GcShadowStack::initializeCustomLowering (llvm::Module& llvmModule)
{
	m_module = getCurrentThreadModule ();
	ASSERT (m_module && m_module->getLlvmModule () == &llvmModule);

	return true;
}

bool
GcShadowStack::performCustomLowering (llvm::Function& llvmFunction)
{
	bool result;

	LLVMContext& context = llvmFunction.getContext();

	Function* function = m_module->m_functionMgr.findFunctionByLlvmFunction (&llvmFunction);
	ASSERT (function && function->getLlvmFunction () == &llvmFunction);

	rtl::Array <Root> rootArray;
	collectRoots (function, &rootArray);

	if (rootArray.isEmpty ())
		return false;

	size_t rootCount = rootArray.getCount ();

	Value frameMapValue;
	result = getFrameMap (function, rootArray, rootCount, &frameMapValue);
	if (!result)
		return false;

	Type* frameType = m_module->m_typeMgr.getGcShadowStackFrameType (rootCount);

	// Build the shadow stack entry after tls-related injected code

	llvm::BasicBlock::iterator llvmInst = llvmFunction.getEntryBlock().begin();
	ASSERT (llvm::isa <llvm::CallInst> (llvmInst)); // get-tls

	llvm::Instruction* llvmGetTls = llvmInst;

	llvm::Instruction* llvmAnchor = function->getLlvmPostTlsPrologueInst ();
	llvm::BasicBlock::iterator IP = llvmAnchor ? llvmAnchor : llvmGetTls;

	IRBuilder<> atEntry(IP->getParent(), IP);

	Instruction *stackEntry = atEntry.CreateAlloca(frameType->getLlvmType (), 0, "gc_frame");

	while (isa<AllocaInst>(IP))
		++IP;

	atEntry.SetInsertPoint(IP->getParent(), IP);

	// gc shadow stack top

	Variable* gcShadowStackTopVariable = m_module->m_variableMgr.getStdVariable (StdVariable_GcShadowStackTop);
	StructField* field = gcShadowStackTopVariable->getTlsField ();
	llvm::Value* head = createGEP (context, atEntry, llvmGetTls, field->getLlvmIndex (), "gc_stack_top");

	// Initialize the map pointer and load the current head of the shadow stack.
	Instruction *currentHead = atEntry.CreateLoad(head, "gc_currhead");
	Instruction *entryMapPtr = createGEP (context, atEntry, stackEntry, 1, "gc_frame.map");
	atEntry.CreateStore (frameMapValue.getLlvmValue (), entryMapPtr);

	// After all the allocas...
	for (size_t i = 0; i < rootCount; i++)
	{
		// For each root, find the corresponding slot in the aggregate...
		llvm::Value *slotPtr = createGEP(context, atEntry, stackEntry, 2, i, "gc_root");

		// And use it in lieu of the alloca.
		AllocaInst *originalAlloca = rootArray [i].m_llvmAlloca;
		slotPtr->takeName(originalAlloca);
		originalAlloca->replaceAllUsesWith(slotPtr);
	}

	// Move past the original stores inserted by GCStrategy::InitRoots. This isn't
	// really necessary (the collector would never see the intermediate state at
	// runtime), but it's nicer not to push the half-initialized entry onto the
	// shadow stack.
	while (isa<StoreInst>(IP))
		++IP;

	atEntry.SetInsertPoint(IP->getParent(), IP);

	Type* framePtrType = m_module->m_typeMgr.getStdType (StdType_BytePtr);

	// Push the entry onto the shadow stack.

	Function* gcEnter = m_module->m_functionMgr.getStdFunction (StdFunction_GcEnter);
	Function* gcLeave = m_module->m_functionMgr.getStdFunction (StdFunction_GcLeave);
	atEntry.CreateCall (gcEnter->getLlvmFunction ());

	llvm::Value* entryNextPtr = createGEP(context, atEntry, stackEntry, 0, "gc_frame.next");
	llvm::Value* newHeadVal = atEntry.CreateBitCast (stackEntry, framePtrType->getLlvmType (), "gc_newhead");
	atEntry.CreateStore(currentHead, entryNextPtr);
	atEntry.CreateStore(newHeadVal, head);

	// For each instruction that escapes...
	EscapeEnumerator EE (llvmFunction, "gc_cleanup");
	while (IRBuilder<> *atExit = EE.next())
	{
		// Pop the entry from the shadow stack. Don't reuse CurrentHead from
		// AtEntry, since that would make the value live for the entire function.
		llvm::Value* entryNextPtr2 = createGEP(context, *atExit, stackEntry, 0, "gc_frame.next");
		llvm::Value* savedHead = atExit->CreateLoad(entryNextPtr2, "gc_savedhead");
		atExit->CreateStore(savedHead, head);
		atExit->CreateCall (gcLeave->getLlvmFunction ());
	}

	// Erase the original allocas (which are no longer used) and the intrinsic
	// calls (which are no longer valid). Doing this last avoids invalidating
	// iterators.

	for (size_t i = 0; i < rootCount; i++)
	{
		rootArray [i].m_llvmGcRoot->eraseFromParent();
		rootArray [i].m_llvmAlloca->eraseFromParent();
	}

	return true;
}

size_t
GcShadowStack::collectRoots (
	Function* function,
	rtl::Array <Root>* rootArray
	)
{
	Type* bytePtrType = m_module->getSimpleType (StdType_BytePtr);

	llvm::Function::iterator llvmBlock = function->getLlvmFunction ()->begin();
	llvm::Function::iterator llvmBlockEnd = function->getLlvmFunction ()->end ();

	if (llvmBlock == llvmBlockEnd)
		return 0;

	llvm::BasicBlock::iterator llvmInst = llvmBlock->begin ();
	llvm::BasicBlock::iterator llvmInstEnd = llvmBlock->end ();

	for (; llvmInst != llvmInstEnd; llvmInst++)
	{
		llvm::IntrinsicInst* llvmInstinsicInst = llvm::dyn_cast <llvm::IntrinsicInst> (llvmInst);

		if (!llvmInstinsicInst ||
			llvmInstinsicInst->getCalledFunction ()->getIntrinsicID() != llvm::Intrinsic::gcroot)
			continue;

		Root root;
		root.m_llvmGcRoot = llvmInstinsicInst;
		root.m_llvmAlloca = llvm::cast <llvm::AllocaInst> (llvmInstinsicInst->getArgOperand(0)->stripPointerCasts ());
		root.m_llvmType   = llvm::cast <llvm::Constant> (llvmInstinsicInst->getArgOperand(1));

		rootArray->append (root);
	}

	return rootArray->getCount ();
}

bool
GcShadowStack::getFrameMap (
	Function* function,
	Root* rootArray,
	size_t rootCount,
	Value* resultValue
	)
{
	StructType* frameMapType = m_module->m_typeMgr.getGcShadowStackFrameMapType (rootCount);

	Type* typeArrayType = frameMapType->getFieldByIndex (1)->getType ();
	ASSERT (typeArrayType->getTypeKind () == TypeKind_Array);

	char buffer [256];
	rtl::Array <llvm::Constant*> llvmTypeArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	llvmTypeArray.setCount (rootCount);

	for (size_t i = 0; i < rootCount; i++)
		llvmTypeArray [i] = rootArray [i].m_llvmType;

	llvm::Constant*
	llvmFrameMap [2] =
	{
		(llvm::Constant*) Value (rootCount, m_module->getSimpleType (TypeKind_SizeT)).getLlvmValue (),

		llvm::ConstantArray::get (
			(llvm::ArrayType*) typeArrayType->getLlvmType (),
			llvm::ArrayRef <llvm::Constant*> (llvmTypeArray, rootCount)
			)
	};

	llvm::Constant* llvmFrameMapConst = llvm::ConstantStruct::get (
		(llvm::StructType*) frameMapType->getLlvmType (),
		llvm::ArrayRef <llvm::Constant*> (llvmFrameMap, 2)
		);

	llvm::GlobalVariable* llvmFrameMapVariable = new llvm::GlobalVariable (
		*m_module->getLlvmModule (),
		frameMapType->getLlvmType (),
		false,
		llvm::GlobalVariable::InternalLinkage,
		llvmFrameMapConst,
		(const char*) (function->m_tag + ".StackFrameMap")
		);

	resultValue->setLlvmValue (
		llvmFrameMapVariable,
		frameMapType->getDataPtrType_c (),
		ValueKind_Const
		);

	return true;
}

//.............................................................................

using namespace llvm;

EscapeEnumerator::EscapeEnumerator (
	llvm::Function &F,
	const char* N
	):
	F (F),
	cleanupBBName (N),
	state (0),
	builder (F.getContext())
{
}

llvm::IRBuilder<>*
EscapeEnumerator::next ()
{
	switch (state)
	{
	default:
		return 0;

	case 0:
		stateBB = F.begin();
		stateE = F.end();
		state = 1;

	case 1:
		// Find all 'return', 'resume', and 'unwind' instructions.
		while (stateBB != stateE)
		{
			llvm::BasicBlock *curBB = stateBB++;

			std::string s = curBB->getName ();

			// Branches and invokes do not escape, only unwind, resume, and return
			// do.
			TerminatorInst* TI = curBB->getTerminator();
			ASSERT (TI);

			if (!isa<ReturnInst>(TI) && !isa<ResumeInst>(TI))
				continue;

			builder.SetInsertPoint(TI->getParent(), TI);
			return &builder;
		}

		state = 2;
		return 0;

#ifdef JNC_EH // will only be relevant after exception handling is implemented
		// Find all 'call' instructions.

		smallVector<instruction*,16> calls;

		for (function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
			for (llvm::BasicBlock::iterator II = BB->begin(), EE = BB->end(); II != EE; ++II)
				if (callInst *CI = dyn_cast<callInst>(II))
						if (!CI->getCalledFunction() ||
							!CI->getCalledFunction()->getIntrinsicID())
						{
							calls.push_back(CI);
						}

		if (calls.empty())
			return 0;

		// Create a cleanup block.
		LLVMContext &C = F.getContext();
		llvm::BasicBlock *cleanupBB = llvm::BasicBlock::create(C, cleanupBBName, &F);
		type *exnTy = structType::get(
			type::getInt8PtrTy(C),
			type::getInt32Ty(C),
			NULL
			);

		constant *persFn = F.getParent()->getOrInsertFunction(
			"__gcc_personality_v0",
			functionType::get(type::getInt32Ty(C), true)
			);

		landingPadInst *LPad = landingPadInst::create(
			exnTy,
			persFn,
			1,
			"cleanup.lpad",
			cleanupBB
			);

		LPad->setCleanup(true);
		resumeInst *RI = resumeInst::create(LPad, cleanupBB);

		// Transform the 'call' instructions into 'invoke's branching to the
		// cleanup block. Go in reverse order to make prettier BB names.
		smallVector<value*,16> args;

		for (unsigned I = calls.size(); I != 0; )
		{
			callInst *CI = cast<callInst>(calls[--I]);

			// Split the basic block containing the function call.
			llvm::BasicBlock *callBB = CI->getParent();
			llvm::BasicBlock *newBB =
			callBB->splitBasicBlock(CI, callBB->getName() + ".cont");

			// Remove the unconditional branch inserted at the end of CallBB.
			callBB->getInstList().pop_back();
			newBB->getInstList().remove(CI);

			// Create a new invoke instruction.
			args.clear();
			callSite CS(CI);
			args.append(CS.arg_begin(), CS.arg_end());

			invokeInst *II = invokeInst::create(
				CI->getCalledValue(),
				newBB,
				cleanupBB,
				args,
				CI->getName(),
				callBB
				);

			II->setCallingConv(CI->getCallingConv());
			II->setAttributes(CI->getAttributes());
			CI->replaceAllUsesWith(II);
			delete CI;
		}

		builder.setInsertPoint(RI->getParent(), RI);
		return &builder;
#endif
	}
}

//.............................................................................

} // namespace jnc
