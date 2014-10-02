#pragma once

#include "jnc_Value.h"

namespace jnc {

//.............................................................................

class GcShadowStack : public llvm::GCStrategy
{
protected:
	struct Root
	{
		llvm::CallInst* m_llvmGcRoot;
		llvm::AllocaInst* m_llvmAlloca;
		llvm::Constant* m_llvmType;
	};

protected:
	Module* m_module;

public:
	GcShadowStack();

	virtual
	bool 
	initializeCustomLowering (llvm::Module& llvmModule);
	
	virtual
	bool 
	performCustomLowering (llvm::Function& llvmFunction);

private:
	size_t
	collectRoots (
		Function* function,
		rtl::Array <Root>* rootArray
		);

	bool
	getFrameMap (
		Function* function,
		Root* rootArray,
		size_t rootCount,
		Value* resultValue
		);
};

//.............................................................................

void
registerGcShadowStack (int);

//.............................................................................

// structure backing up shadow stack frame map

struct GcShadowStackFrameMap
{
	size_t m_count;

	// followed by array of type pointers
};

// structure backing up shadow stack frame

struct GcShadowStackFrame
{
	GcShadowStackFrame* m_next;
	GcShadowStackFrameMap* m_map;

	// followed by array of root pointers
};

//.............................................................................

} // namespace jnc 
