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
#include "jnc_ct_BasicBlock.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

BasicBlock::BasicBlock(
	Module* module,
	const sl::StringRef& name,
	uint_t flags
) {
	m_module = module;
	m_name = name;
	m_flags = flags;
	m_function = NULL;
	m_landingPadScope = NULL;
	m_llvmBlock = NULL;
}

Value
BasicBlock::getBlockAddressValue() {
	ASSERT(m_llvmBlock);
	llvm::BlockAddress* llvmAddress = llvm::BlockAddress::get(m_function->getLlvmFunction(), m_llvmBlock);

	Value value;
	value.setLlvmValue(llvmAddress, m_module->m_typeMgr.getStdType(StdType_ByteThinPtr));
	return value;
}

//..............................................................................

} // namespace ct
} // namespace jnc
