#include "pch.h"
#include "jnc_CallConv.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

llvm::CallingConv::ID
GetLlvmCallConv (ECallConv CallConvKind)
{
	llvm::CallingConv::ID LlvmCallConvTable [] =
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

	return (size_t) CallConvKind < countof (LlvmCallConvTable) ?
		LlvmCallConvTable [CallConvKind] :
		LlvmCallConvTable [ECallConv_Undefined];
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

uint_t
GetCallConvFlags (ECallConv CallConvKind)
{
	uint_t FlagTable [] =
	{
		0,                       // ECallConv_Undefined = 0,
		ECallConvFlag_Jnccall,   // ECallConv_Jnccall_msc32,
		ECallConvFlag_Jnccall,   // ECallConv_Jnccall_msc64,
		ECallConvFlag_Jnccall,   // ECallConv_Jnccall_gcc32,
		ECallConvFlag_Jnccall,   // ECallConv_Jnccall_gcc64,

		ECallConvFlag_Cdecl |    // ECallConv_Cdecl_msc32,
		ECallConvFlag_Msc |
		ECallConvFlag_UnsafeVarArg,

		ECallConvFlag_Cdecl |    // ECallConv_Cdecl_msc64,
		ECallConvFlag_Msc |
		ECallConvFlag_UnsafeVarArg,

		ECallConvFlag_Cdecl |    // ECallConv_Cdecl_gcc32,
		ECallConvFlag_Gcc |
		ECallConvFlag_UnsafeVarArg,

		ECallConvFlag_Cdecl |    // ECallConv_Cdecl_gcc64,
		ECallConvFlag_Gcc |
		ECallConvFlag_UnsafeVarArg,

		ECallConvFlag_Stdcall |  // ECallConv_Stdcall_msc32,
		ECallConvFlag_Msc |
		ECallConvFlag_NoVarArg,

		ECallConvFlag_Stdcall |  // ECallConv_Stdcall_gcc32,
		ECallConvFlag_Gcc |
		ECallConvFlag_NoVarArg,
	};

	return (size_t) CallConvKind < countof (FlagTable) ?
		FlagTable [CallConvKind] :
		FlagTable [ECallConv_Undefined];
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
GetCallConvString (ECallConv CallConvKind)
{
	static const char* StringTable [] =
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

	return (size_t) CallConvKind < countof (StringTable) ?
		StringTable [CallConvKind] :
		StringTable [ECallConv_Undefined];
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
GetCallConvDisplayString (ECallConv CallConvKind)
{
	static const char* StringTable [] =
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

	return (size_t) CallConvKind < countof (StringTable) ?
		StringTable [CallConvKind] :
		StringTable [ECallConv_Undefined];
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
GetCallConvSignature (ECallConv CallConvKind)
{
	static const char* StringTable [] =
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

	return (size_t) CallConvKind < countof (StringTable) ?
		StringTable [CallConvKind] :
		StringTable [ECallConv_Undefined];
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

ECallConv
GetCallConvKindFromModifiers (uint_t Modifiers)
{
#if (_AXL_CPU == AXL_CPU_X86)
	return
		(Modifiers & ETypeModifier_Thiscall) ? ECallConv_Thiscall :
		(Modifiers & ETypeModifier_Stdcall) ? ECallConv_Stdcall :
		(Modifiers & ETypeModifier_Cdecl) ? ECallConv_Cdecl : ECallConv_Default;
#else
	return (Modifiers & (ETypeModifier_Cdecl | ETypeModifier_Stdcall)) ?
		ECallConv_Cdecl :
		ECallConv_Default;
#endif
}

//.............................................................................

CCallConv::CCallConv ()
{
	m_pModule = GetCurrentThreadModule ();
	ASSERT (m_pModule);

	m_CallConvKind = ECallConv_Undefined;
}

llvm::FunctionType*
CCallConv::GetLlvmFunctionType (CFunctionType* pFunctionType)
{
	rtl::CArrayT <CFunctionArg*> ArgArray = pFunctionType->GetArgArray ();
	size_t ArgCount = ArgArray.GetCount ();

	char Buffer [256];
	rtl::CArrayT <llvm::Type*> LlvmArgTypeArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	LlvmArgTypeArray.SetCount (ArgCount);

	for (size_t i = 0; i < ArgCount; i++)
		LlvmArgTypeArray [i] = ArgArray [i]->GetType ()->GetLlvmType ();

	return llvm::FunctionType::get (
		pFunctionType->GetReturnType ()->GetLlvmType (),
		llvm::ArrayRef <llvm::Type*> (LlvmArgTypeArray, ArgCount),
		(pFunctionType->GetFlags () & EFunctionTypeFlag_VarArg) != 0
		);
}

llvm::Function*
CCallConv::CreateLlvmFunction (
	CFunctionType* pFunctionType,
	const char* pTag
	)
{
	llvm::FunctionType* pLlvmType = (llvm::FunctionType*) pFunctionType->GetLlvmType ();
	llvm::Function* pLlvmFunction = llvm::Function::Create (
		pLlvmType,
		llvm::Function::ExternalLinkage,
		pTag,
		m_pModule->GetLlvmModule ()
		);

	llvm::CallingConv::ID LlvmCallConv = GetLlvmCallConv ();
	if (LlvmCallConv)
		pLlvmFunction->setCallingConv (LlvmCallConv);

	return pLlvmFunction;
}

void
CCallConv::Call (
	const CValue& CalleeValue,
	CFunctionType* pFunctionType,
	rtl::CBoxListT <CValue>* pArgValueList,
	CValue* pResultValue
	)
{
	m_pModule->m_LlvmIrBuilder.CreateCall (
		CalleeValue,
		pFunctionType,
		*pArgValueList,
		pResultValue
		);
}

void
CCallConv::Return (
	CFunction* pFunction,
	const CValue& Value
	)
{
	m_pModule->m_LlvmIrBuilder.CreateRet (Value);
}

CValue
CCallConv::GetThisArgValue (CFunction* pFunction)
{
	ASSERT (pFunction->IsMember ());

	llvm::Function::arg_iterator LlvmArg = pFunction->GetLlvmFunction ()->arg_begin ();
	return CValue (LlvmArg, pFunction->GetThisArgType ());
}

void
CCallConv::CreateArgVariablesImpl (
	CFunction* pFunction,
	size_t BaseLlvmArgIdx
	)
{
	llvm::Function::arg_iterator LlvmArg = pFunction->GetLlvmFunction ()->arg_begin ();
	for (size_t i = 0; i < BaseLlvmArgIdx; i++)
		LlvmArg++;

	size_t i = 0;
	if (pFunction->IsMember ()) // skip this
	{
		i++;
		LlvmArg++;
	}

	rtl::CArrayT <CFunctionArg*> ArgArray = pFunction->GetType ()->GetArgArray ();
	size_t ArgCount = ArgArray.GetCount ();
	for (; i < ArgCount; i++, LlvmArg++)
	{
		CFunctionArg* pArg = ArgArray [i];
		if (!pArg->IsNamed ())
			continue;

		llvm::Value* pLlvmArgValue = LlvmArg;

		CVariable* pArgVariable = m_pModule->m_VariableMgr.CreateArgVariable (pArg, pLlvmArgValue);
		pFunction->GetScope ()->AddItem (pArgVariable);

		CValue ArgValue (pLlvmArgValue, pArg->GetType ());
		m_pModule->m_LlvmIrBuilder.CreateStore (ArgValue, pArgVariable);
	}
}

//.............................................................................

} // namespace jnc {
