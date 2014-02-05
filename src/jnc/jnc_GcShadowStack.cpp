#include "pch.h"
#include "jnc_GcShadowStack.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

void
RegisterGcShadowStack (int)
{
	static llvm::GCRegistry::Add <CGcShadowStack> ShadowStack (
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
	const char* CleanupBBName;

	// State.
	int State;
	llvm::Function::iterator StateBB, StateE;
	llvm::IRBuilder<> Builder;

public:
	EscapeEnumerator (llvm::Function &F, const char* N = "cleanup");

	llvm::IRBuilder<>*
	Next ();
};

//.............................................................................

CGcShadowStack::CGcShadowStack ()
{
	m_pModule = NULL;
	InitRoots = true;
	CustomRoots = true;
}

bool
CGcShadowStack::initializeCustomLowering (llvm::Module& LlvmModule)
{
	m_pModule = GetCurrentThreadModule ();
	ASSERT (m_pModule && m_pModule->GetLlvmModule () == &LlvmModule);

	return true;
}

using namespace llvm;

GetElementPtrInst *
CreateGEP(
	LLVMContext &Context,
	IRBuilder<> &B,
	Value *BasePtr,
	int Idx,
	int Idx2,
	const char *Name
	)
{
	Value *Indices[] =
	{
		ConstantInt::get(Type::getInt32Ty(Context), 0),
		ConstantInt::get(Type::getInt32Ty(Context), Idx),
		ConstantInt::get(Type::getInt32Ty(Context), Idx2)
	};

	Value* Val = B.CreateGEP(BasePtr, Indices, Name);

	assert(isa<GetElementPtrInst>(Val) && "Unexpected folded constant");

	return dyn_cast<GetElementPtrInst>(Val);
}

GetElementPtrInst *
CreateGEP (
	LLVMContext &Context,
	IRBuilder<> &B,
	Value *BasePtr,
	int Idx,
	const char *Name)
{
	Value *Indices[] =
	{
		ConstantInt::get(Type::getInt32Ty(Context), 0),
		ConstantInt::get(Type::getInt32Ty(Context), Idx)
	};

	Value *Val = B.CreateGEP(BasePtr, Indices, Name);

	assert(isa<GetElementPtrInst>(Val) && "Unexpected folded constant");

	return dyn_cast<GetElementPtrInst>(Val);
}

bool
CGcShadowStack::performCustomLowering (llvm::Function& LlvmFunction)
{
	bool Result;

	LLVMContext& Context = LlvmFunction.getContext();

	CFunction* pFunction = m_pModule->m_FunctionMgr.FindFunctionByLlvmFunction (&LlvmFunction);
	ASSERT (pFunction && pFunction->GetLlvmFunction () == &LlvmFunction);

	rtl::CArrayT <TRoot> RootArray;
	CollectRoots (pFunction, &RootArray);

	if (RootArray.IsEmpty ())
		return false;

	size_t RootCount = RootArray.GetCount ();

	CValue FrameMapValue;
	Result = GetFrameMap (pFunction, RootArray, RootCount, &FrameMapValue);
	if (!Result)
		return false;

	CType* pFrameType = m_pModule->m_TypeMgr.GetGcShadowStackFrameType (RootCount);

	// Build the shadow stack entry after tls-related injected code

	BasicBlock::iterator LlvmInst = LlvmFunction.getEntryBlock().begin();
	ASSERT (llvm::isa <llvm::CallInst> (LlvmInst)); // get-tls

	llvm::Instruction* pLlvmGetTls = LlvmInst;

	llvm::Instruction* pLlvmAnchor = pFunction->GetLlvmPostTlsPrologueInst ();
	BasicBlock::iterator IP = pLlvmAnchor ? pLlvmAnchor : pLlvmGetTls;

	IRBuilder<> AtEntry(IP->getParent(), IP);

	Instruction *StackEntry = AtEntry.CreateAlloca(pFrameType->GetLlvmType (), 0, "gc_frame");

	while (isa<AllocaInst>(IP))
		++IP;

	AtEntry.SetInsertPoint(IP->getParent(), IP);

	// gc shadow stack top

	CVariable* pGcShadowStackTopVariable = m_pModule->m_VariableMgr.GetStdVariable (EStdVariable_GcShadowStackTop);
	CStructField* pField = pGcShadowStackTopVariable->GetTlsField ();
	llvm::Value* Head = CreateGEP (Context, AtEntry, pLlvmGetTls, pField->GetLlvmIndex (), "gc_stack_top");

	// Initialize the map pointer and load the current head of the shadow stack.
	Instruction *CurrentHead = AtEntry.CreateLoad(Head, "gc_currhead");
	Instruction *EntryMapPtr = CreateGEP (Context, AtEntry, StackEntry, 1, "gc_frame.map");
	AtEntry.CreateStore (FrameMapValue.GetLlvmValue (), EntryMapPtr);

	// After all the allocas...
	for (size_t i = 0; i < RootCount; i++)
	{
		// For each root, find the corresponding slot in the aggregate...
		Value *SlotPtr = CreateGEP(Context, AtEntry, StackEntry, 2, i, "gc_root");

		// And use it in lieu of the alloca.
		AllocaInst *OriginalAlloca = RootArray [i].m_pLlvmAlloca;
		SlotPtr->takeName(OriginalAlloca);
		OriginalAlloca->replaceAllUsesWith(SlotPtr);
	}

	// Move past the original stores inserted by GCStrategy::InitRoots. This isn't
	// really necessary (the collector would never see the intermediate state at
	// runtime), but it's nicer not to push the half-initialized entry onto the
	// shadow stack.
	while (isa<StoreInst>(IP))
		++IP;

	AtEntry.SetInsertPoint(IP->getParent(), IP);

	CType* pFramePtrType = m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr);

	// Push the entry onto the shadow stack.

	CFunction* pGcEnter = m_pModule->m_FunctionMgr.GetStdFunction (EStdFunc_GcEnter);
	CFunction* pGcLeave = m_pModule->m_FunctionMgr.GetStdFunction (EStdFunc_GcLeave);
	AtEntry.CreateCall (pGcEnter->GetLlvmFunction ());

	Value* EntryNextPtr = CreateGEP(Context, AtEntry, StackEntry, 0, "gc_frame.next");
	Value* NewHeadVal = AtEntry.CreateBitCast (StackEntry, pFramePtrType->GetLlvmType (), "gc_newhead");
	AtEntry.CreateStore(CurrentHead, EntryNextPtr);
	AtEntry.CreateStore(NewHeadVal, Head);

	// For each instruction that escapes...
	EscapeEnumerator EE (LlvmFunction, "gc_cleanup");
	while (IRBuilder<> *AtExit = EE.Next())
	{
		// Pop the entry from the shadow stack. Don't reuse CurrentHead from
		// AtEntry, since that would make the value live for the entire function.
		Value* EntryNextPtr2 = CreateGEP(Context, *AtExit, StackEntry, 0, "gc_frame.next");
		Value* SavedHead = AtExit->CreateLoad(EntryNextPtr2, "gc_savedhead");
		AtExit->CreateStore(SavedHead, Head);
		AtExit->CreateCall (pGcLeave->GetLlvmFunction ());
	}

	// Delete the original allocas (which are no longer used) and the intrinsic
	// calls (which are no longer valid). Doing this last avoids invalidating
	// iterators.

	for (size_t i = 0; i < RootCount; i++)
	{
		RootArray [i].m_pLlvmGcRoot->eraseFromParent();
		RootArray [i].m_pLlvmAlloca->eraseFromParent();
	}

	return true;
}

size_t
CGcShadowStack::CollectRoots (
	CFunction* pFunction,
	rtl::CArrayT <TRoot>* pRootArray
	)
{
	CType* pBytePtrType = m_pModule->GetSimpleType (EStdType_BytePtr);

	llvm::Function::iterator LlvmBlock = pFunction->GetLlvmFunction ()->begin();
	llvm::Function::iterator LlvmBlockEnd = pFunction->GetLlvmFunction ()->end ();

	if (LlvmBlock == LlvmBlockEnd)
		return 0;

	llvm::BasicBlock::iterator LlvmInst = LlvmBlock->begin ();
	llvm::BasicBlock::iterator LlvmInstEnd = LlvmBlock->end ();

	for (; LlvmInst != LlvmInstEnd; LlvmInst++)
	{
		llvm::IntrinsicInst* pLlvmInstinsicInst = llvm::dyn_cast <llvm::IntrinsicInst> (LlvmInst);

		if (!pLlvmInstinsicInst ||
			pLlvmInstinsicInst->getCalledFunction ()->getIntrinsicID() != llvm::Intrinsic::gcroot)
			continue;

		TRoot Root;
		Root.m_pLlvmGcRoot = pLlvmInstinsicInst;
		Root.m_pLlvmAlloca = llvm::cast <llvm::AllocaInst> (pLlvmInstinsicInst->getArgOperand(0)->stripPointerCasts ());
		Root.m_pLlvmType   = llvm::cast <llvm::Constant> (pLlvmInstinsicInst->getArgOperand(1));

		pRootArray->Append (Root);
	}

	return pRootArray->GetCount ();
}

bool
CGcShadowStack::GetFrameMap (
	CFunction* pFunction,
	TRoot* pRootArray,
	size_t RootCount,
	CValue* pResultValue
	)
{
	CStructType* pFrameMapType = m_pModule->m_TypeMgr.GetGcShadowStackFrameMapType (RootCount);

	CType* pTypeArrayType = pFrameMapType->GetFieldByIndex (1)->GetType ();
	ASSERT (pTypeArrayType->GetTypeKind () == EType_Array);

	char Buffer [256];
	rtl::CArrayT <llvm::Constant*> LlvmTypeArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	LlvmTypeArray.SetCount (RootCount);

	for (size_t i = 0; i < RootCount; i++)
		LlvmTypeArray [i] = pRootArray [i].m_pLlvmType;

	llvm::Constant*
	LlvmFrameMap [2] =
	{
		(llvm::Constant*) CValue (RootCount, m_pModule->GetSimpleType (EType_SizeT)).GetLlvmValue (),

		llvm::ConstantArray::get (
			(llvm::ArrayType*) pTypeArrayType->GetLlvmType (),
			llvm::ArrayRef <llvm::Constant*> (LlvmTypeArray, RootCount)
			)
	};

	llvm::Constant* pLlvmFrameMapConst = llvm::ConstantStruct::get (
		(llvm::StructType*) pFrameMapType->GetLlvmType (),
		llvm::ArrayRef <llvm::Constant*> (LlvmFrameMap, 2)
		);

	llvm::GlobalVariable* pLlvmFrameMapVariable = new llvm::GlobalVariable (
		*m_pModule->GetLlvmModule (),
		pFrameMapType->GetLlvmType (),
		false,
		llvm::GlobalVariable::InternalLinkage,
		pLlvmFrameMapConst,
		(const char*) (pFunction->m_Tag + ".StackFrameMap")
		);

	pResultValue->SetLlvmValue (
		pLlvmFrameMapVariable,
		pFrameMapType->GetDataPtrType_c (),
		EValue_Const
		);

	return true;
}

//.............................................................................

using namespace llvm;

EscapeEnumerator::EscapeEnumerator (
	Function &F,
	const char* N
	):
	F (F),
	CleanupBBName (N),
	State (0),
	Builder (F.getContext())
{
}

llvm::IRBuilder<>*
EscapeEnumerator::Next ()
{
	switch (State)
	{
	default:
		return 0;

	case 0:
		StateBB = F.begin();
		StateE = F.end();
		State = 1;

	case 1:
		// Find all 'return', 'resume', and 'unwind' instructions.
		while (StateBB != StateE)
		{
			BasicBlock *CurBB = StateBB++;

			std::string s = CurBB->getName ();

			// Branches and invokes do not escape, only unwind, resume, and return
			// do.
			TerminatorInst* TI = CurBB->getTerminator();
			ASSERT (TI);

			if (!isa<ReturnInst>(TI) && !isa<ResumeInst>(TI))
				continue;

			Builder.SetInsertPoint(TI->getParent(), TI);
			return &Builder;
		}

		State = 2;
		return 0;

#ifdef JNC_EH // will only be relevant after exception handling is implemented
		// Find all 'call' instructions.

		SmallVector<Instruction*,16> Calls;

		for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
			for (BasicBlock::iterator II = BB->begin(), EE = BB->end(); II != EE; ++II)
				if (CallInst *CI = dyn_cast<CallInst>(II))
						if (!CI->getCalledFunction() ||
							!CI->getCalledFunction()->getIntrinsicID())
						{
							Calls.push_back(CI);
						}

		if (Calls.empty())
			return 0;

		// Create a cleanup block.
		LLVMContext &C = F.getContext();
		BasicBlock *CleanupBB = BasicBlock::Create(C, CleanupBBName, &F);
		Type *ExnTy = StructType::get(
			Type::getInt8PtrTy(C),
			Type::getInt32Ty(C),
			NULL
			);

		Constant *PersFn = F.getParent()->getOrInsertFunction(
			"__gcc_personality_v0",
			FunctionType::get(Type::getInt32Ty(C), true)
			);

		LandingPadInst *LPad = LandingPadInst::Create(
			ExnTy,
			PersFn,
			1,
			"cleanup.lpad",
			CleanupBB
			);

		LPad->setCleanup(true);
		ResumeInst *RI = ResumeInst::Create(LPad, CleanupBB);

		// Transform the 'call' instructions into 'invoke's branching to the
		// cleanup block. Go in reverse order to make prettier BB names.
		SmallVector<Value*,16> Args;

		for (unsigned I = Calls.size(); I != 0; )
		{
			CallInst *CI = cast<CallInst>(Calls[--I]);

			// Split the basic block containing the function call.
			BasicBlock *CallBB = CI->getParent();
			BasicBlock *NewBB =
			CallBB->splitBasicBlock(CI, CallBB->getName() + ".cont");

			// Remove the unconditional branch inserted at the end of CallBB.
			CallBB->getInstList().pop_back();
			NewBB->getInstList().remove(CI);

			// Create a new invoke instruction.
			Args.clear();
			CallSite CS(CI);
			Args.append(CS.arg_begin(), CS.arg_end());

			InvokeInst *II = InvokeInst::Create(
				CI->getCalledValue(),
				NewBB,
				CleanupBB,
				Args,
				CI->getName(),
				CallBB
				);

			II->setCallingConv(CI->getCallingConv());
			II->setAttributes(CI->getAttributes());
			CI->replaceAllUsesWith(II);
			delete CI;
		}

		Builder.SetInsertPoint(RI->getParent(), RI);
		return &Builder;
#endif
	}
}

//.............................................................................

} // namespace jnc
