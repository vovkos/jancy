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
ScopeLevelStack::takeOver (ScopeLevelStack* srcStack)
{
	m_module = srcStack->m_module;
	m_list.takeOver (&srcStack->m_list);
	m_stack = srcStack->m_stack;
	srcStack->m_stack.clear ();
}

Value
ScopeLevelStack::getScopeLevel (size_t level)
{
#ifndef _JNC_NO_SCOPE_LEVEL_CACHE
	Entry* entry = getEntry (level);
	if (entry->m_scopeLevelValue)
		return entry->m_scopeLevelValue;

	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	BasicBlock* prevBlock = m_module->m_controlFlowMgr.setCurrentBlock (function->getEntryBlock ());
#endif

	Type* type = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);

	Value scopeBaseLevelValue = m_module->m_functionMgr.getScopeLevel ();
	Value scopeIncValue (level, type);
	Value scopeLevelValue;

	m_module->m_llvmIrBuilder.createAdd_i (
		scopeBaseLevelValue,
		scopeIncValue,
		type,
		&scopeLevelValue
		);

#ifndef _JNC_NO_SCOPE_LEVEL_CACHE
	m_module->m_controlFlowMgr.setCurrentBlock (prevBlock);
	entry->m_scopeLevelValue = scopeLevelValue;
#endif
	return scopeLevelValue;
}

Value
ScopeLevelStack::getObjHdr (size_t level)
{
#ifndef _JNC_NO_SCOPE_LEVEL_CACHE
	Entry* entry = getEntry (level);
	if (entry->m_objHdrValue)
		return entry->m_objHdrValue;

	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	BasicBlock* prevBlock = m_module->m_controlFlowMgr.setCurrentBlock (function->getEntryBlock ());
#endif

	Value objHdrValue;

	Type* type = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);

	m_module->m_llvmIrBuilder.createAlloca (type, "scopeLevel", type, &objHdrValue);

	Value scopeLevelValue = getScopeLevel (level);
	m_module->m_llvmIrBuilder.createStore (scopeLevelValue, objHdrValue);

	type = m_module->m_typeMgr.getStdType (StdTypeKind_ObjHdrPtr);
	m_module->m_llvmIrBuilder.createBitCast (objHdrValue, type, &objHdrValue);

#ifndef _JNC_NO_SCOPE_LEVEL_CACHE
	m_module->m_controlFlowMgr.setCurrentBlock (prevBlock);
	entry->m_objHdrValue = objHdrValue;
#endif
	return objHdrValue;
}

ScopeLevelStack::Entry*
ScopeLevelStack::getEntry (size_t level)
{
	size_t count = m_stack.getCount ();
	if (level >= count)
		m_stack.setCount (level + 1);

	if (m_stack [level])
		return m_stack [level];

	Entry* entry = AXL_MEM_NEW (Entry);
	m_list.insertTail (entry);
	m_stack [level] = entry;
	return entry;
}

//.............................................................................

} // namespace jnc {
