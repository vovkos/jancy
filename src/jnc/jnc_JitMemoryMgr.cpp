#include "pch.h"
#include "jnc_JitMemoryMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

void*
JitMemoryMgr::getPointerToNamedFunction (
	const std::string& name,
	bool abortOnFailure
	)
{
	void* pf = m_module->findFunctionMapping (name.c_str ());
	if (pf)
		return pf;

	if (abortOnFailure)
	{
		std::string errorString = "CJitMemoryManager::getPointerToNamedFunction: unresolved external function '" + name + "'";
		llvm::report_fatal_error (errorString);
	}

	return NULL;
}

uint64_t
JitMemoryMgr::getSymbolAddress (const std::string &name)
{
	void* pf = m_module->findFunctionMapping (name.c_str ());
	if (pf)
		return (uint64_t) pf;

	return 0;
}

//.............................................................................

} // namespace jnc {
