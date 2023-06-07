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
#include "jnc_ct_JitMemoryMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

void*
JitMemoryMgr::getPointerToNamedFunction(
	const std::string& name,
	bool abortOnFailure
) {
	void* p = m_module->findFunctionMapping(name.c_str());
	if (p)
		return p;

	if (abortOnFailure) {
		std::string errorString = "JitMemoryManager::getPointerToNamedFunction: unresolved external function '" + name + "'";
		llvm::report_fatal_error(errorString.c_str());
	}

	return NULL;
}

uint64_t
JitMemoryMgr::getSymbolAddress(const std::string &name) {
	void* p = m_module->findFunctionMapping(name.c_str());
	if (p)
		return (uint64_t)p;

	return 0;
}

//..............................................................................

} // namespace ct
} // namespace jnc
