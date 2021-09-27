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

BasicBlock::BasicBlock() {
	m_module = NULL;
	m_flags = 0;
	m_llvmBlock = NULL;
	m_function = NULL;
	m_landingPadScope = NULL;
}

Value
BasicBlock::getBlockAddressValue() {
	ASSERT(m_llvmBlock);
	llvm::BlockAddress* llvmAddress = llvm::BlockAddress::get(m_function->getLlvmFunction(), m_llvmBlock);

	Value value;
	value.setLlvmValue(llvmAddress, m_module->m_typeMgr.getStdType(StdType_BytePtr));
	return value;
}

//..............................................................................

} // namespace ct
} // namespace jnc
