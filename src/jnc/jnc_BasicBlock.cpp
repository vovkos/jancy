#include "pch.h"
#include "jnc_BasicBlock.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

BasicBlock::BasicBlock ()
{
	m_module = NULL;
	m_llvmBlock = NULL;
	m_function = NULL;
	m_flags = 0;
}

Value
BasicBlock::getBlockAddressValue ()
{
	llvm::BlockAddress* llvmAddress = llvm::BlockAddress::get (m_function->getLlvmFunction (), m_llvmBlock);
	
	Value value;
	value.setLlvmValue (llvmAddress, m_module->getSimpleType (StdTypeKind_BytePtr));
	return value;	
}

//.............................................................................

} // namespace jnc {
 