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

#pragma once

#include "jnc_ct_Type.h"
#include "jnc_ct_Scope.h"

namespace jnc {
namespace ct {

class Module;

//..............................................................................

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

//..............................................................................

} // namespace ct
} // namespace jnc
