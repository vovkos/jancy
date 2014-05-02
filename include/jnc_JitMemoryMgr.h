// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Type.h"
#include "jnc_Scope.h"

namespace jnc {

class CModule;

//.............................................................................

class CJitMemoryMgr: public llvm::SectionMemoryManager
{
protected:
	CModule* m_pModule;

public:
	CJitMemoryMgr (CModule* pModule)
	{
		m_pModule = pModule;
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

//.............................................................................

} // namespace jnc {
