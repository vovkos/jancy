#include "pch.h"
#include "jnc_JitMemoryMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

void*
CJitMemoryMgr::getPointerToNamedFunction (
	const std::string& Name,
	bool AbortOnFailure
	)
{
	void* pf = m_pModule->FindFunctionMapping (Name.c_str ());
	if (pf)
		return pf;

	if (AbortOnFailure)
	{
		std::string ErrorString = "CJitMemoryManager::getPointerToNamedFunction: unresolved external function '" + Name + "'";
		llvm::report_fatal_error (ErrorString);
	}

	return NULL;
}

uint64_t
CJitMemoryMgr::getSymbolAddress (const std::string &Name)
{
	void* pf = m_pModule->FindFunctionMapping (Name.c_str ());
	if (pf)
		return (uint64_t) pf;

	return 0;
}

//.............................................................................

} // namespace jnc {
