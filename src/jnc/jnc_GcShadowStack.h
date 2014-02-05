#pragma once

#include "jnc_Value.h"

namespace jnc {

//.............................................................................

class CGcShadowStack : public llvm::GCStrategy
{
protected:
	struct TRoot
	{
		llvm::CallInst* m_pLlvmGcRoot;
		llvm::AllocaInst* m_pLlvmAlloca;
		llvm::Constant* m_pLlvmType;
	};

protected:
	CModule* m_pModule;

public:
	CGcShadowStack();

	virtual
	bool 
	initializeCustomLowering (llvm::Module& LlvmModule);
	
	virtual
	bool 
	performCustomLowering (llvm::Function& LlvmFunction);

private:
	size_t
	CollectRoots (
		CFunction* pFunction,
		rtl::CArrayT <TRoot>* pRootArray
		);

	bool
	GetFrameMap (
		CFunction* pFunction,
		TRoot* pRootArray,
		size_t RootCount,
		CValue* pResultValue
		);
};

//.............................................................................

void
RegisterGcShadowStack (int);

//.............................................................................

// structure backing up shadow stack frame map

struct TGcShadowStackFrameMap
{
	size_t m_Count;

	// followed by array of type pointers
};

// structure backing up shadow stack frame

struct TGcShadowStackFrame
{
	TGcShadowStackFrame* m_pNext;
	TGcShadowStackFrameMap* m_pMap;

	// followed by array of root pointers
};

//.............................................................................

} // namespace jnc 
