#include "pch.h"
#include "jnc_ScopeLevelStack.h"
#include "jnc_Module.h"

#if (_AXL_ENV == AXL_ENV_POSIX)
#	pragma AXL_TODO ("MCJIT is atrocious with line info; disabling cache helps a bit - remove it when MCJIT improves")
#	// define _JNC_NO_SCOPE_LEVEL_CACHE // <-- doesn't help much really (just a few cases get better)
#endif

namespace jnc {

//.............................................................................

void
CScopeLevelStack::TakeOver (CScopeLevelStack* pSrcStack)
{
	m_pModule = pSrcStack->m_pModule;
	m_List.TakeOver (&pSrcStack->m_List);
	m_Stack = pSrcStack->m_Stack;
	pSrcStack->m_Stack.Clear ();
}

CValue
CScopeLevelStack::GetScopeLevel (size_t Level)
{
#ifndef _JNC_NO_SCOPE_LEVEL_CACHE
	TEntry* pEntry = GetEntry (Level);
	if (pEntry->m_ScopeLevelValue)
		return pEntry->m_ScopeLevelValue;

	CFunction* pFunction = m_pModule->m_FunctionMgr.GetCurrentFunction ();
	CBasicBlock* pPrevBlock = m_pModule->m_ControlFlowMgr.SetCurrentBlock (pFunction->GetEntryBlock ());
#endif

	CType* pType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_SizeT);

	CValue ScopeBaseLevelValue = m_pModule->m_FunctionMgr.GetScopeLevel ();
	CValue ScopeIncValue (Level, pType);
	CValue ScopeLevelValue;

	m_pModule->m_LlvmIrBuilder.CreateAdd_i (
		ScopeBaseLevelValue,
		ScopeIncValue,
		pType,
		&ScopeLevelValue
		);

#ifndef _JNC_NO_SCOPE_LEVEL_CACHE
	m_pModule->m_ControlFlowMgr.SetCurrentBlock (pPrevBlock);
	pEntry->m_ScopeLevelValue = ScopeLevelValue;
#endif
	return ScopeLevelValue;
}

CValue
CScopeLevelStack::GetObjHdr (size_t Level)
{
#ifndef _JNC_NO_SCOPE_LEVEL_CACHE
	TEntry* pEntry = GetEntry (Level);
	if (pEntry->m_ObjHdrValue)
		return pEntry->m_ObjHdrValue;

	CFunction* pFunction = m_pModule->m_FunctionMgr.GetCurrentFunction ();
	CBasicBlock* pPrevBlock = m_pModule->m_ControlFlowMgr.SetCurrentBlock (pFunction->GetEntryBlock ());
#endif

	CValue ObjHdrValue;

	CType* pType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_SizeT);

	m_pModule->m_LlvmIrBuilder.CreateAlloca (pType, "scopeLevel", pType, &ObjHdrValue);

	CValue ScopeLevelValue = GetScopeLevel (Level);
	m_pModule->m_LlvmIrBuilder.CreateStore (ScopeLevelValue, ObjHdrValue);

	pType = m_pModule->m_TypeMgr.GetStdType (EStdType_ObjHdrPtr);
	m_pModule->m_LlvmIrBuilder.CreateBitCast (ObjHdrValue, pType, &ObjHdrValue);

#ifndef _JNC_NO_SCOPE_LEVEL_CACHE
	m_pModule->m_ControlFlowMgr.SetCurrentBlock (pPrevBlock);
	pEntry->m_ObjHdrValue = ObjHdrValue;
#endif
	return ObjHdrValue;
}

CScopeLevelStack::TEntry*
CScopeLevelStack::GetEntry (size_t Level)
{
	size_t Count = m_Stack.GetCount ();
	if (Level >= Count)
		m_Stack.SetCount (Level + 1);

	if (m_Stack [Level])
		return m_Stack [Level];

	TEntry* pEntry = AXL_MEM_NEW (TEntry);
	m_List.InsertTail (pEntry);
	m_Stack [Level] = pEntry;
	return pEntry;
}

//.............................................................................

} // namespace jnc {
