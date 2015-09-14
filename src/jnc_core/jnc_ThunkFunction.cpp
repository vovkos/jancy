#include "pch.h"
#include "jnc_ThunkFunction.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

ThunkFunction::ThunkFunction ()
{
	m_functionKind = FunctionKind_Thunk;
	m_targetFunction = NULL;
}

bool 
ThunkFunction::compile ()
{
	ASSERT (m_targetFunction);

	bool result;

	rtl::Array <FunctionArg*> targetArgArray = m_targetFunction->getType ()->getArgArray ();
	rtl::Array <FunctionArg*> thunkArgArray = m_type->getArgArray ();

	size_t targetArgCount = targetArgArray.getCount ();
	size_t thunkArgCount = thunkArgArray.getCount ();

	char buffer [256];
	rtl::Array <Value> argArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	argArray.setCount (targetArgCount);

	m_module->m_functionMgr.internalPrologue (this);

	llvm::Function::arg_iterator llvmArg = getLlvmFunction ()->arg_begin();

	// skip the first thunk argument (if needed)
	
	if (thunkArgCount != targetArgCount)
	{
		ASSERT (thunkArgCount == targetArgCount + 1);
		thunkArgArray.remove (0); 
		llvmArg++;
	}
		
	for (size_t i = 0; i < targetArgCount; i++, llvmArg++)
	{
		Value argValue (llvmArg, thunkArgArray [i]->getType ());
		result = m_module->m_operatorMgr.castOperator (&argValue, targetArgArray [i]->getType ());
		if (!result)
			return false;

		argArray [i] = argValue;
	}	

	Value returnValue;
	m_module->m_llvmIrBuilder.createCall (
		m_targetFunction, 
		m_targetFunction->getType (),
		argArray,
		argArray.getCount (),
		&returnValue
		);

	if (m_type->getReturnType ()->getTypeKind () != TypeKind_Void)
	{
		result = m_module->m_controlFlowMgr.ret (returnValue);
		if (!result)
			return false;
	}

	m_module->m_functionMgr.internalEpilogue ();
	return true;
}

//.............................................................................

} // namespace jnc {
