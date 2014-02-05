#include "pch.h"
#include "jnc_BasicBlock.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CBasicBlock::CBasicBlock ()
{
	m_pModule = NULL;
	m_pLlvmBlock = NULL;
	m_pFunction = NULL;
	m_Flags = 0;
}

CValue
CBasicBlock::GetBlockAddressValue ()
{
	llvm::BlockAddress* pLlvmAddress = llvm::BlockAddress::get (m_pFunction->GetLlvmFunction (), m_pLlvmBlock);
	
	CValue Value;
	Value.SetLlvmValue (pLlvmAddress, m_pModule->GetSimpleType (EStdType_BytePtr));
	return Value;	
}

//.............................................................................

} // namespace jnc {
 