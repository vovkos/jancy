// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Type.h"
#include "jnc_Scope.h"

namespace jnc {

class Module;

//.............................................................................

class JitMemoryMgr: public llvm::SectionMemoryManager
{
protected:
	Module* m_module;

public:
	JitMemoryMgr (Module* module)
	{
		m_module = module;
	}

	virtual
	void*
	getPointerToNamedFunction (
		const std::string &name,
		bool abortOnFailure
		);

	virtual
	uint64_t
	getSymbolAddress (const std::string &name);
};

//.............................................................................

} // namespace jnc {
