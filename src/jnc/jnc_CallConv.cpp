#include "pch.h"
#include "jnc_CallConv.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

llvm::CallingConv::ID
getLlvmCallConv (CallConvKind callConvKind)
{
	llvm::CallingConv::ID llvmCallConvTable [] =
	{
		llvm::CallingConv::C,            // ECallConv_Undefined = 0,
		llvm::CallingConv::C,            // ECallConv_Jnccall_msc32,
		llvm::CallingConv::C,            // ECallConv_Jnccall_msc64,
		llvm::CallingConv::C,            // ECallConv_Jnccall_gcc32,
		llvm::CallingConv::C,            // ECallConv_Jnccall_gcc64,
		llvm::CallingConv::C,            // ECallConv_Cdecl_msc32,
		llvm::CallingConv::C,            // ECallConv_Cdecl_msc64,
		llvm::CallingConv::C,            // ECallConv_Cdecl_gcc32,
		llvm::CallingConv::C,            // ECallConv_Cdecl_gcc64,
		llvm::CallingConv::X86_StdCall,  // ECallConv_Stdcall_msc32,
		llvm::CallingConv::X86_StdCall,  // ECallConv_Stdcall_gcc32,
		llvm::CallingConv::X86_ThisCall, // ECallConv_Thiscall_msc32,
	};

	return (size_t) callConvKind < countof (llvmCallConvTable) ?
		llvmCallConvTable [callConvKind] :
		llvmCallConvTable [CallConvKind_Undefined];
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

uint_t
getCallConvFlags (CallConvKind callConvKind)
{
	uint_t flagTable [] =
	{
		0,                       // ECallConv_Undefined = 0,
		CallConvFlag_Jnccall,   // ECallConv_Jnccall_msc32,
		CallConvFlag_Jnccall,   // ECallConv_Jnccall_msc64,
		CallConvFlag_Jnccall,   // ECallConv_Jnccall_gcc32,
		CallConvFlag_Jnccall,   // ECallConv_Jnccall_gcc64,

		CallConvFlag_Cdecl |    // ECallConv_Cdecl_msc32,
		CallConvFlag_Msc |
		CallConvFlag_UnsafeVarArg,

		CallConvFlag_Cdecl |    // ECallConv_Cdecl_msc64,
		CallConvFlag_Msc |
		CallConvFlag_UnsafeVarArg,

		CallConvFlag_Cdecl |    // ECallConv_Cdecl_gcc32,
		CallConvFlag_Gcc |
		CallConvFlag_UnsafeVarArg,

		CallConvFlag_Cdecl |    // ECallConv_Cdecl_gcc64,
		CallConvFlag_Gcc |
		CallConvFlag_UnsafeVarArg,

		CallConvFlag_Stdcall |  // ECallConv_Stdcall_msc32,
		CallConvFlag_Msc |
		CallConvFlag_NoVarArg,

		CallConvFlag_Stdcall |  // ECallConv_Stdcall_gcc32,
		CallConvFlag_Gcc |
		CallConvFlag_NoVarArg,
	};

	return (size_t) callConvKind < countof (flagTable) ?
		flagTable [callConvKind] :
		flagTable [CallConvKind_Undefined];
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getCallConvString (CallConvKind callConvKind)
{
	static const char* stringTable [] =
	{
		"undefinded-call-conv",  // ECallConv_Undefined = 0,
		"jnccall-msc32",         // ECallConv_Jnccall_msc32,
		"jnccall-msc64",         // ECallConv_Jnccall_msc64,
		"jnccall-gcc32",         // ECallConv_Jnccall_gcc32,
		"jnccall-gcc64",         // ECallConv_Jnccall_gcc64,
		"cdecl-msc32",           // ECallConv_Cdecl_msc32,
		"cdecl-msc64",           // ECallConv_Cdecl_msc64,
		"cdecl-gcc32",           // ECallConv_Cdecl_gcc32,
		"cdecl-gcc64",           // ECallConv_Cdecl_gcc64,
		"stdcall-msc32",         // ECallConv_Stdcall_msc32,
		"stdcall-gcc32",         // ECallConv_Stdcall_gcc32,
	};

	return (size_t) callConvKind < countof (stringTable) ?
		stringTable [callConvKind] :
		stringTable [CallConvKind_Undefined];
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getCallConvDisplayString (CallConvKind callConvKind)
{
	static const char* stringTable [] =
	{
		"undefinded-call-conv",  // ECallConv_Undefined = 0,
		"jnccall",               // ECallConv_Jnccall_msc32,
		"jnccall",               // ECallConv_Jnccall_msc64,
		"jnccall",               // ECallConv_Jnccall_gcc32,
		"jnccall",               // ECallConv_Jnccall_gcc64,
		"cdecl",                 // ECallConv_Cdecl_msc32,
		"cdecl",                 // ECallConv_Cdecl_msc64,
		"cdecl",                 // ECallConv_Cdecl_gcc32,
		"cdecl",                 // ECallConv_Cdecl_gcc64,
		"stdcall",               // ECallConv_Stdcall_msc32,
		"stdcall",               // ECallConv_Stdcall_gcc32,
	};

	return (size_t) callConvKind < countof (stringTable) ?
		stringTable [callConvKind] :
		stringTable [CallConvKind_Undefined];
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getCallConvSignature (CallConvKind callConvKind)
{
	static const char* stringTable [] =
	{
		"UСС",    // ECallConv_Undefined = 0,
		"JM4",    // ECallConv_Jnccall_msc32,
		"JM8",    // ECallConv_Jnccall_msc64,
		"JG4",    // ECallConv_Jnccall_gcc32,
		"JG8",    // ECallConv_Jnccall_gcc64,
		"CM4",    // ECallConv_Cdecl_msc32,
		"CM8",    // ECallConv_Cdecl_msc64,
		"CG4",    // ECallConv_Cdecl_gcc32,
		"CG8",    // ECallConv_Cdecl_gcc64,
		"SM4",    // ECallConv_Stdcall_msc32,
		"SG4",    // ECallConv_Stdcall_gcc32,
		"TM4",    // ECallConv_Thiscall_msc32,
	};

	return (size_t) callConvKind < countof (stringTable) ?
		stringTable [callConvKind] :
		stringTable [CallConvKind_Undefined];
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

CallConvKind
getCallConvKindFromModifiers (uint_t modifiers)
{
#if (_AXL_CPU == AXL_CPU_X86)
	return
		(modifiers & TypeModifier_Thiscall) ? CallConvKind_Thiscall :
		(modifiers & TypeModifier_Stdcall) ? CallConvKind_Stdcall :
		(modifiers & TypeModifier_Cdecl) ? CallConvKind_Cdecl : CallConvKind_Default;
#else
	return (modifiers & (TypeModifier_Cdecl | TypeModifier_Stdcall)) ?
		CallConvKind_Cdecl :
		CallConvKind_Default;
#endif
}

//.............................................................................

CallConv::CallConv ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);

	m_callConvKind = CallConvKind_Undefined;
}

llvm::FunctionType*
CallConv::getLlvmFunctionType (FunctionType* functionType)
{
	rtl::Array <FunctionArg*> argArray = functionType->getArgArray ();
	size_t argCount = argArray.getCount ();

	char buffer [256];
	rtl::Array <llvm::Type*> llvmArgTypeArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	llvmArgTypeArray.setCount (argCount);

	for (size_t i = 0; i < argCount; i++)
		llvmArgTypeArray [i] = argArray [i]->getType ()->getLlvmType ();

	return llvm::FunctionType::get (
		functionType->getReturnType ()->getLlvmType (),
		llvm::ArrayRef <llvm::Type*> (llvmArgTypeArray, argCount),
		(functionType->getFlags () & FunctionTypeFlag_VarArg) != 0
		);
}

llvm::Function*
CallConv::createLlvmFunction (
	FunctionType* functionType,
	const char* tag
	)
{
	llvm::FunctionType* llvmType = (llvm::FunctionType*) functionType->getLlvmType ();
	llvm::Function* llvmFunction = llvm::Function::Create (
		llvmType,
		llvm::Function::ExternalLinkage,
		tag,
		m_module->getLlvmModule ()
		);

	llvm::CallingConv::ID llvmCallConv = getLlvmCallConv ();
	if (llvmCallConv)
		llvmFunction->setCallingConv (llvmCallConv);

	return llvmFunction;
}

void
CallConv::call (
	const Value& calleeValue,
	FunctionType* functionType,
	rtl::BoxList <Value>* argValueList,
	Value* resultValue
	)
{
	m_module->m_llvmIrBuilder.createCall (
		calleeValue,
		functionType,
		*argValueList,
		resultValue
		);
}

void
CallConv::ret (
	Function* function,
	const Value& value
	)
{
	m_module->m_llvmIrBuilder.createRet (value);
}

Value
CallConv::getThisArgValue (Function* function)
{
	ASSERT (function->isMember ());

	llvm::Function::arg_iterator llvmArg = function->getLlvmFunction ()->arg_begin ();
	return Value (llvmArg, function->getThisArgType ());
}

void
CallConv::createArgVariablesImpl (
	Function* function,
	size_t baseLlvmArgIdx
	)
{
	llvm::Function::arg_iterator llvmArg = function->getLlvmFunction ()->arg_begin ();
	for (size_t i = 0; i < baseLlvmArgIdx; i++)
		llvmArg++;

	size_t i = 0;
	if (function->isMember ()) // skip this
	{
		i++;
		llvmArg++;
	}

	rtl::Array <FunctionArg*> argArray = function->getType ()->getArgArray ();
	size_t argCount = argArray.getCount ();
	for (; i < argCount; i++, llvmArg++)
	{
		FunctionArg* arg = argArray [i];
		if (!arg->isNamed ())
			continue;

		llvm::Value* llvmArgValue = llvmArg;

		Variable* argVariable = m_module->m_variableMgr.createArgVariable (arg, llvmArgValue);
		function->getScope ()->addItem (argVariable);

		Value argValue (llvmArgValue, arg->getType ());
		m_module->m_llvmIrBuilder.createStore (argValue, argVariable);
	}
}

//.............................................................................

} // namespace jnc {
