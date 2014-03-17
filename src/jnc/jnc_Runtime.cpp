#include "pch.h"
#include "jnc_Runtime.h"
#include "jnc_Module.h"
#include "jnc_StdLib.h"
#include "jnc_GcShadowStack.h"

namespace jnc {

using namespace llvm;

//.............................................................................

const char*
GetJitKindString (EJit JitKind)
{
	static const char* StringTable [EJit__Count] =
	{
		"normal-jit", // EJit_Normal = 0,
		"mc-jit",     // EJit_McJit,
	};

	return (size_t) JitKind < EJit__Count ?
		StringTable [JitKind] :
		"undefined-jit";
}

//.............................................................................

class CJitMemoryManager: public llvm::SectionMemoryManager
{
public:
	CRuntime* m_pRuntime;

	CJitMemoryManager (CRuntime* pRuntime)
	{
		m_pRuntime = pRuntime;
	}

	virtual
	void*
	getPointerToNamedFunction (
		const std::string &Name,
		bool AbortOnFailure
		);

	virtual
	uint64_t
	getSymbolAddress (const std::string &Name);
};

void*
CJitMemoryManager::getPointerToNamedFunction (
	const std::string& Name,
	bool AbortOnFailure
	)
{
	void* pf = m_pRuntime->FindFunctionMapping (Name.c_str ());
	if (pf)
		return pf;

	if (AbortOnFailure)
		report_fatal_error ("CJitMemoryManager::getPointerToNamedFunction: unresolved external function '" + Name + "'");

	return NULL;
}

uint64_t
CJitMemoryManager::getSymbolAddress (const std::string &Name)
{
	void* pf = m_pRuntime->FindFunctionMapping (Name.c_str ());
	if (pf)
		return (uint64_t) pf;

	return 0;
}

//.............................................................................

CRuntime::CRuntime ()
{
	m_pModule = NULL;
	m_pLlvmExecutionEngine = NULL;
	m_JitKind = EJit_Normal;

	m_GcState = EGcState_Idle;
	m_GcHeapLimit = -1;
	m_TotalGcAllocSize = 0;
	m_CurrentGcAllocSize = 0;
	m_PeriodGcAllocSize = 0;
	m_PeriodGcAllocLimit = 16 * 1024;
	m_GcUnsafeThreadCount = 1;
	m_CurrentGcRootArrayIdx = 0;

	m_StackLimit = -1;
	m_TlsSlot = -1;
	m_TlsSize = 0;
}

bool
CRuntime::Create (
	CModule* pModule,
	EJit JitKind,
	size_t HeapLimit,
	size_t StackLimit
	)
{
	Destroy ();

	m_pModule = pModule;
	m_JitKind = JitKind;
	m_GcHeapLimit = HeapLimit;
	m_StackLimit = StackLimit;

	// execution engine

	llvm::EngineBuilder EngineBuilder (pModule->GetLlvmModule ());

	std::string errorString;
	EngineBuilder.setErrorStr (&errorString);
	EngineBuilder.setEngineKind(llvm::EngineKind::JIT);

	llvm::TargetOptions TargetOptions;
#if (LLVM_VERSION < 0x0304) // they removed JITExceptionHandling in 3.4
	TargetOptions.JITExceptionHandling = true;
#endif

	if (JitKind == EJit_McJit)
	{
		CJitMemoryManager* pJitMemoryManager = new CJitMemoryManager (this);
		EngineBuilder.setUseMCJIT (true);
#if (LLVM_VERSION < 0x0304) // they distinguish between JIT & MCJIT memory managers in 3.4
		EngineBuilder.setJITMemoryManager (pJitMemoryManager);
#else
		EngineBuilder.setMCJITMemoryManager (pJitMemoryManager);
#endif

		TargetOptions.JITEmitDebugInfo = true;
	}

	EngineBuilder.setTargetOptions (TargetOptions);

#if (_AXL_CPU == AXL_CPU_X86)
	EngineBuilder.setMArch ("x86");
#endif

	m_pLlvmExecutionEngine = EngineBuilder.create ();
	if (!m_pLlvmExecutionEngine)
	{
		err::SetFormatStringError ("cannot create execution engine: %s\n", errorString.c_str());
		return false;
	}

	// static gc roots

	rtl::CArrayT <CVariable*> StaticRootArray = pModule->m_VariableMgr.GetStaticGcRootArray ();
	size_t Count = StaticRootArray.GetCount ();

	m_StaticGcRootArray.SetCount (Count);
	for (size_t i = 0; i < Count; i++)
	{
		CVariable* pVariable = StaticRootArray [i];
		void* p = m_pLlvmExecutionEngine->getPointerToGlobal ((llvm::GlobalVariable*) pVariable->GetLlvmAllocValue ());
		ASSERT (p);

		m_StaticGcRootArray [i].m_p = p;
		m_StaticGcRootArray [i].m_pType = pVariable->GetType ();
	}

	// tls

	m_TlsSize = pModule->m_VariableMgr.GetTlsStructType ()->GetSize ();
	m_TlsSlot = GetTlsMgr ()->CreateSlot ();
	return true;
}

void
CRuntime::Destroy ()
{
	if (!m_pModule)
		return;

	if (m_TlsSlot != -1)
	{
		GetTlsMgr ()->NullifyTls (this);
		GetTlsMgr ()->DestroySlot (m_TlsSlot);
	}

	if (m_pLlvmExecutionEngine)
		delete m_pLlvmExecutionEngine;

	m_pLlvmExecutionEngine = NULL;

	m_TlsSlot = -1;
	m_TlsSize = 0;

	while (!m_TlsList.IsEmpty ())
	{
		TTlsHdr* pTls = m_TlsList.RemoveHead ();
		AXL_MEM_FREE (pTls);
	}

/*	m_GcHeap = NULL;
	m_GcBlockSize = 0;
	m_GcMap.Clear ();
	m_GcObjectList.Clear ();
	m_GcDestructList.Clear ();
*/

	m_StaticGcRootArray.Clear ();
	m_FunctionMap.Clear ();

	m_pModule = NULL;
}

void
CRuntime::MapFunction (
	llvm::Function* pLlvmFunction,
	void* pf
	)
{
	if (m_JitKind == EJit_McJit)
	{
		m_FunctionMap [pLlvmFunction->getName ().data ()] = pf;
	}
	else
	{
		ASSERT (m_pLlvmExecutionEngine);
		m_pLlvmExecutionEngine->addGlobalMapping (pLlvmFunction, pf);
	}
}

bool
CRuntime::Startup ()
{
	// ensure correct state

	m_GcState = EGcState_Idle;
	m_GcUnsafeThreadCount = 1;
	m_GcIdleEvent.Signal ();

	GetTlsMgr ()->NullifyTls (this);
	m_TlsList.Clear ();
	return true;
}

void
CRuntime::Shutdown ()
{
	rtl::CArrayT <TGcRoot> SaveStaticGcRootArray = m_StaticGcRootArray;
	m_StaticGcRootArray.Clear ();

	RunGc ();
	RunGc (); // 2nd gc run is needed to clean up after destruction

	TTlsHdr* pTls = GetTlsMgr ()->NullifyTls (this);
	if (pTls)
	{
		m_TlsList.Remove (pTls);
		AXL_MEM_FREE (pTls);
	}

	ASSERT (m_TlsList.IsEmpty ());

	m_StaticGcRootArray = SaveStaticGcRootArray; // recover
}

void
CRuntime::RuntimeError (
	int Error,
	void* pCodeAddr,
	void* pDataAddr
	)
{
	const char* pErrorString;

	switch (Error)
	{
	case ERuntimeError_OutOfMemory:
		pErrorString = "OUT_OF_MEMORY";
		break;

	case ERuntimeError_StackOverflow:
		pErrorString = "STACK_OVERFLOW";
		break;

	case ERuntimeError_DataPtrOutOfRange:
		pErrorString = "DATA_PTR_OOR";
		break;

	case ERuntimeError_ScopeMismatch:
		pErrorString = "SCOPE_MISMATCH";
		break;

	case ERuntimeError_NullClassPtr:
		pErrorString = "NULL_CLASS_PTR";
		break;

	case ERuntimeError_NullFunctionPtr:
		pErrorString = "NULL_FUNCTION_PTR";
		break;

	case ERuntimeError_NullPropertyPtr:
		pErrorString = "NULL_PROPERTY_PTR";
		break;

	case ERuntimeError_AbstractFunction:
		pErrorString = "ABSTRACT_FUNCTION";
		break;

	default:
		ASSERT (false);
		pErrorString = "<UNDEF>";
	}

	throw err::FormatStringError (
		"RUNTIME ERROR: %s (code %x accessing data %x)",
		pErrorString,
		pCodeAddr,
		pDataAddr
		);
}

void*
CRuntime::GcAllocate (
	CType* pType,
	size_t ElementCount
	)
{
	void* p = GcTryAllocate (pType, ElementCount);
	if (!p)
	{
		RuntimeError (ERuntimeError_OutOfMemory, NULL, NULL);
		ASSERT (false);
	}

	return p;
}

void*
CRuntime::GcTryAllocate (
	CType* pType,
	size_t ElementCount
	)
{
	size_t PrevGcLevel = GcMakeThreadSafe ();
	ASSERT (PrevGcLevel); // otherwise there is risk of losing return value

	size_t Size = sizeof (TObjHdr) + pType->GetSize () * ElementCount;
	if (ElementCount > 1)
		Size += sizeof (size_t);

	WaitGcIdleAndLock ();
	if (m_PeriodGcAllocSize > m_PeriodGcAllocLimit)
	{
	//	RunGc_l ();
	//	WaitGcIdleAndLock ();
	}

	RestoreGcLevel (PrevGcLevel); // restore before unlocking

	void* pBlock = AXL_MEM_ALLOC (Size);
	if (!pBlock)
	{
		m_Lock.Unlock ();
		return NULL;
	}

	TObjHdr* pObject;
	void* p;

	if (pType->GetTypeKind () == EType_Class)
	{
		pObject = (TObjHdr*) pBlock;
		GcAddObject (pObject, (CClassType*) pType);

		// object will be primed by user code

		p = pObject;
	}
	else
	{
		if (ElementCount > 1)
		{
			size_t* pSize = (size_t*) pBlock;
			*pSize = ElementCount;
			pObject = (TObjHdr*) (pSize + 1);
			pObject->m_Flags = EObjHdrFlag_DynamicArray;
		}
		else
		{
			pObject = (TObjHdr*) pBlock;
			pObject->m_Flags = 0;
		}

		pObject->m_ScopeLevel = 0;
		pObject->m_pRoot = pObject;
		pObject->m_pType = pType;

		p = pObject + 1;
	}

	m_PeriodGcAllocSize += Size;
	m_GcMemBlockArray.Append (pObject);
	m_Lock.Unlock ();
	return p;
}

void
CRuntime::GcAddObject (
	TObjHdr* pObject,
	CClassType* pType
	)
{
	char* p = (char*) (pObject + 1);

	rtl::CArrayT <CStructField*> ClassFieldArray = pType->GetClassMemberFieldArray ();
	size_t Count = ClassFieldArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CStructField* pField = ClassFieldArray [i];
		ASSERT (pField->GetType ()->GetTypeKind () == EType_Class);

		TObjHdr* pChildObject = (TObjHdr*) (p + pField->GetOffset ());
		GcAddObject (pChildObject, (CClassType*) pField->GetType ());
	}

	m_GcObjectArray.Append (pObject);
}

void
CRuntime::AddGcRoot (
	void* p,
	CType* pType
	)
{
	ASSERT (m_GcState == EGcState_Mark);
	ASSERT (pType->GetFlags () & ETypeFlag_GcRoot);

	TGcRoot Root = { p, pType };
	m_GcRootArray [m_CurrentGcRootArrayIdx].Append (Root);
}

void
CRuntime::MarkGcLocalHeapRoot (
	void* p,
	CType* pType
	)
{
	if (pType->GetTypeKind () == EType_Class)
		((TObjHdr*) p)->GcMarkObject (this);
	else
		((TObjHdr*) p - 1)->GcMarkData (this);

	if (pType->GetFlags () & ETypeFlag_GcRoot)
		AddGcRoot (p, pType);
}

void
CRuntime::RunGc ()
{
	size_t PrevGcLevel = GcMakeThreadSafe ();
	WaitGcIdleAndLock ();
	RunGc_l ();
	RestoreGcLevel (PrevGcLevel);
}

void
CRuntime::RunGc_l ()
{
	m_GcIdleEvent.Reset ();

	// 1) suspend all mutator threads at safe points

	m_GcState = EGcState_WaitSafePoint;

	ASSERT (m_GcUnsafeThreadCount);
	m_GcSafePointEvent.Reset ();
	intptr_t UnsafeCount = mt::AtomicDec (&m_GcUnsafeThreadCount);
	if (UnsafeCount)
	{
		m_Lock.Unlock ();
		m_GcSafePointEvent.Wait ();
		m_Lock.Lock ();
	}

	// 2) mark

	m_GcState = EGcState_Mark;

	m_GcRootArray [0].Clear ();
	m_CurrentGcRootArrayIdx = 0;

	// 2.1) add static roots

	size_t Count = m_StaticGcRootArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		ASSERT (m_StaticGcRootArray [i].m_pType->GetFlags () & ETypeFlag_GcRoot);
		AddGcRoot (
			m_StaticGcRootArray [i].m_p,
			m_StaticGcRootArray [i].m_pType
			);
	}

	// 2.2) add destructible roots

	// 2.2) add stack roots

	rtl::CIteratorT <TTlsHdr> Tls = m_TlsList.GetHead ();
	for (; Tls; Tls++)
	{
		TTls* pTls = (TTls*) (*Tls + 1);

		TGcShadowStackFrame* pStackFrame = pTls->m_pGcShadowStackTop;
		for (; pStackFrame; pStackFrame = pStackFrame->m_pNext)
		{
			size_t Count = pStackFrame->m_pMap->m_Count;
			void** ppRootArray = (void**) (pStackFrame + 1);
			CType** ppTypeArray = (CType**) (pStackFrame->m_pMap + 1);

			for (size_t i = 0; i < Count; i++)
			{
				void* p = ppRootArray [i];
				CType* pType = ppTypeArray [i];

				if (p) // check needed, stack roots could be nullified
				{
					if (pType->GetTypeKind () == EType_DataPtr &&
						((CDataPtrType*) pType)->GetPtrTypeKind () == EDataPtrType_Thin) // local heap variable
					{
						MarkGcLocalHeapRoot (p, ((CDataPtrType*) pType)->GetTargetType ());
					}
					else
					{
						AddGcRoot (p, pType);
					}
				}
			}
		}
	}

	// 2.3) run mark cycle

	GcMarkCycle ();

	// 2.4) mark objects as dead and schedule destruction

	char Buffer [256];
	rtl::CArrayT <TIfaceHdr*> DestructArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	TGcDestructGuard DestructGuard;

	Count = m_GcObjectArray.GetCount ();
	for (intptr_t i = Count - 1; i >= 0; i--)
	{
		jnc::TObjHdr* pObject = m_GcObjectArray [i];

		if (!(pObject->m_Flags & (EObjHdrFlag_GcMark | EObjHdrFlag_GcWeakMark_c)))
		{
			m_GcObjectArray.Remove (i);
			pObject->m_Flags |= EObjHdrFlag_Dead;

			if (pObject->m_pClassType->GetDestructor ())
				DestructArray.Append ((TIfaceHdr*) (pObject + 1));
		}
	}

	if (!DestructArray.IsEmpty ())
	{
		DestructGuard.m_pDestructArray = &DestructArray;
		m_GcDestructGuardList.InsertTail (&DestructGuard);
	}

	// 2.5) mark all destruct guard gc roots

	rtl::CIteratorT <TGcDestructGuard> It = m_GcDestructGuardList.GetHead ();
	for (; It; It++)
	{
		TGcDestructGuard* pDestructGuard = *It;

		Count = DestructArray.GetCount ();
		for (size_t i = 0; i < Count; i++)
		{
			AddGcRoot (
				&DestructArray [i],
				DestructArray [i]->m_pObject->m_pClassType->GetClassPtrType ()
				);
		}
	}

	GcMarkCycle ();

	// 3) sweep

	m_GcState = EGcState_Sweep;

	// 3.4) free memory blocks

	Count = m_GcMemBlockArray.GetCount ();
	for (intptr_t i = Count - 1; i >= 0; i--)
	{
		TObjHdr* pObject = m_GcMemBlockArray [i];
		if (!(pObject->m_Flags & EObjHdrFlag_GcWeakMark))
		{
			m_GcMemBlockArray.Remove (i);

			void* pBlock = (pObject->m_Flags & EObjHdrFlag_DynamicArray) ?
				(void*) ((size_t*) pObject - 1) :
				pObject;

			AXL_MEM_FREE (pBlock);
		}
		else
		{
			pObject->m_Flags &= ~EObjHdrFlag_GcMask; // unmark
		}
	}

	// 3.5) unmark the remaining objects

	Count = m_GcObjectArray.GetCount ();
	for (intptr_t i = Count - 1; i >= 0; i--)
	{
		jnc::TObjHdr* pObject = m_GcObjectArray [i];
		pObject->m_Flags &= ~EObjHdrFlag_GcMask;
	}

	// 4) gc run is done, resume all suspended threads

	mt::AtomicInc (&m_GcUnsafeThreadCount);
	m_PeriodGcAllocSize = 0;
	m_GcState = EGcState_Idle;
	m_GcIdleEvent.Signal ();
	m_Lock.Unlock ();

	// 5) run destructors

	if (!DestructArray.IsEmpty ())
	{
		Count = DestructArray.GetCount ();
		for (size_t i = 0; i < Count; i++)
		{
			TIfaceHdr* pIface = DestructArray [i];
			CFunction* pDestructor = pIface->m_pObject->m_pClassType->GetDestructor ();
			ASSERT (pDestructor);

			FObject_Destruct* pf = (FObject_Destruct*) pDestructor->GetMachineCode ();
			pf (pIface);
		}

		m_Lock.Lock ();
		m_GcDestructGuardList.Remove (&DestructGuard);
		m_Lock.Unlock ();
	}
}

void
CRuntime::GcMarkCycle ()
{
	// mark breadth first

	while (!m_GcRootArray [m_CurrentGcRootArrayIdx].IsEmpty ())
	{
		size_t PrevGcRootArrayIdx =  m_CurrentGcRootArrayIdx;
		m_CurrentGcRootArrayIdx = !m_CurrentGcRootArrayIdx;
		m_GcRootArray [m_CurrentGcRootArrayIdx].Clear ();

		size_t Count = m_GcRootArray [PrevGcRootArrayIdx].GetCount ();
		for (size_t i = 0; i < Count; i++)
		{
			const TGcRoot* pRoot = &m_GcRootArray [PrevGcRootArrayIdx] [i];
			pRoot->m_pType->GcMark (this, pRoot->m_p);
		}
	}
}

void
CRuntime::GcEnter ()
{
	TTlsHdr* pTls = GetTlsMgr ()->GetTls (this);
	ASSERT (pTls);

	pTls->m_GcLevel++;
	if (pTls->m_GcLevel > 1) // was already unsafe
		GcPulse (); // pulse on enter only, no pulse on leave: might lose retval gcroot
	else
		GcIncrementUnsafeThreadCount ();
}

void
CRuntime::GcLeave ()
{
	ASSERT (m_GcState == EGcState_Idle || m_GcState == EGcState_WaitSafePoint);

	TTlsHdr* pTls = GetTlsMgr ()->GetTls (this);
	ASSERT (pTls && pTls->m_GcLevel);

	pTls->m_GcLevel--;
	if (!pTls->m_GcLevel) // not unsafe anymore
		GcDecrementUnsafeThreadCount ();
}

void
CRuntime::GcPulse ()
{
	ASSERT (m_GcState == EGcState_Idle || m_GcState == EGcState_WaitSafePoint);

	if (m_GcState != EGcState_WaitSafePoint)
		return;

	TTlsHdr* pTls = GetTlsMgr ()->GetTls (this);
	ASSERT (pTls);

	if (pTls->m_GcLevel)
	{
		GcDecrementUnsafeThreadCount ();
		GcIncrementUnsafeThreadCount ();
	}
}

void
CRuntime::GcIncrementUnsafeThreadCount ()
{
	// what we try to prevent here is entering an unsafe region when collector thread
	// thinks all the mutators are parked at safe regions and therefore moves on to mark/sweep

	for (;;)
	{
		if (m_GcState != EGcState_Idle)
			m_GcIdleEvent.Wait ();

		intptr_t OldCount = m_GcUnsafeThreadCount;
		if (OldCount == 0) // oh-oh -- we started gc run in between these two 'if's
			continue;

		intptr_t NewCount = OldCount + 1;
		intptr_t PrevCount = mt::AtomicCmpXchg (&m_GcUnsafeThreadCount, OldCount, NewCount);
		if (PrevCount == OldCount)
			break;
	}
}

void
CRuntime::GcDecrementUnsafeThreadCount ()
{
	intptr_t Count = mt::AtomicDec (&m_GcUnsafeThreadCount);
	if (m_GcState == EGcState_WaitSafePoint)
	{
		if (!Count)
			m_GcSafePointEvent.Signal ();

		m_GcIdleEvent.Wait ();
	}
}

size_t
CRuntime::GcMakeThreadSafe ()
{
	TTlsHdr* pTls = GetTlsMgr ()->GetTls (this);
	ASSERT (pTls);

	if (!pTls->m_GcLevel)
		return 0;

	size_t PrevGcLevel = pTls->m_GcLevel;
	pTls->m_GcLevel = 0;
	GcDecrementUnsafeThreadCount ();
	return PrevGcLevel;
}

void
CRuntime::RestoreGcLevel (size_t PrevGcLevel)
{
	if (!PrevGcLevel)
		return;

	TTlsHdr* pTls = GetTlsMgr ()->GetTls (this);
	ASSERT (pTls);

	pTls->m_GcLevel = PrevGcLevel;
	GcIncrementUnsafeThreadCount ();
}

void
CRuntime::WaitGcIdleAndLock ()
{
	ASSERT (!GetTls ()->m_GcLevel); // otherwise we have a potential deadlock

	for (;;)
	{
		m_Lock.Lock ();

		if (m_GcState == EGcState_Idle)
			break;

		m_Lock.Unlock ();
		m_GcIdleEvent.Wait ();
	}
}

TTlsHdr*
CRuntime::GetTls ()
{
	TTlsHdr* pTls = GetTlsMgr ()->GetTls (this);
	ASSERT (pTls);

	// check for stack overflow

	char* p = (char*) _alloca (1);

	if (!pTls->m_pStackEpoch) // first time call
	{
		pTls->m_pStackEpoch = p;
	}
	else
	{
		char* p0 = (char*) pTls->m_pStackEpoch;
		if (p0 >= p) // the opposite could happen, but it's stack-overflow-safe
		{
			size_t StackSize = p0 - p;
			if (StackSize > m_StackLimit)
			{
				CStdLib::RuntimeError (ERuntimeError_StackOverflow, NULL, NULL);
				ASSERT (false);
			}
		}
	}

	return pTls;
}

TTlsHdr*
CRuntime::CreateTls ()
{
	size_t Size = sizeof (TTlsHdr) + m_TlsSize;

	TTlsHdr* pTls = (TTlsHdr*) AXL_MEM_ALLOC (Size);
	memset (pTls, 0, Size);
	pTls->m_pRuntime = this;
	pTls->m_pStackEpoch = NULL;

	m_Lock.Lock ();
	m_TlsList.InsertTail (pTls);
	m_Lock.Unlock ();

	return pTls;
}

void
CRuntime::DestroyTls (TTlsHdr* pTls)
{
	m_Lock.Lock ();
	m_TlsList.Remove (pTls);
	m_Lock.Unlock ();

	AXL_MEM_FREE (pTls);
}

TIfaceHdr*
CRuntime::CreateObject (
	CClassType* pType,
	uint_t Flags
	)
{
	CFunction* pPrimer = pType->GetPrimer ();
	if (!pPrimer) // abstract
	{
		err::SetFormatStringError ("cannot create abstract '%s'", pType->m_Tag.cc ());
		return NULL;
	}

	GcEnter ();

	TObjHdr* pObject = (TObjHdr*) GcAllocate (pType);
	if (!pObject)
		return NULL;

	CScopeThreadRuntime ScopeRuntime (this);

	if (Flags & ECreateObjectFlag_Prime)
	{
		FObject_Prime* pfPrime = (FObject_Prime*) pPrimer->GetMachineCode ();
		pfPrime (pObject, 0, pObject, 0);
	}

	TIfaceHdr* pInterface = (TIfaceHdr*) (pObject + 1);

	if ((Flags & ECreateObjectFlag_Construct) && pType->GetConstructor ())
	{
		CFunction* pConstructor = pType->GetDefaultConstructor ();
		if (!pConstructor)
			return NULL;

		FObject_Construct* pfConstruct = (FObject_Construct*) pConstructor->GetMachineCode ();
		pfConstruct (pInterface);
	}

	if (Flags & ECreateObjectFlag_Pin)
		PinObject (pInterface);

	GcLeave ();

	return pInterface;
}

void
CRuntime::PinObject (TIfaceHdr* pObject)
{
	m_Lock.Lock ();
	m_GcPinTable.Goto (pObject);
	m_Lock.Unlock ();
}

void
CRuntime::UnpinObject (TIfaceHdr* pObject)
{
	m_Lock.Lock ();
	m_GcPinTable.DeleteByKey (pObject);
	m_Lock.Unlock ();
}

//.............................................................................

} // namespace jnc
