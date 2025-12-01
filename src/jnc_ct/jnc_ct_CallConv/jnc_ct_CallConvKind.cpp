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
#include "jnc_ct_CallConvKind.h"
#include "jnc_ct_Decl.h"

namespace jnc {
namespace ct {

//..............................................................................

llvm::CallingConv::ID
getLlvmCallConv(CallConvKind callConvKind) {
	llvm::CallingConv::ID llvmCallConvTable[CallConvKind__Count] = {
		llvm::CallingConv::C,            // CallConvKind_Undefined = 0,
		llvm::CallingConv::C,            // CallConvKind_Jnccall_msc32,
		llvm::CallingConv::C,            // CallConvKind_Jnccall_msc64,
		llvm::CallingConv::C,            // CallConvKind_Jnccall_gcc32,
		llvm::CallingConv::C,            // CallConvKind_Jnccall_gcc64,
		llvm::CallingConv::C,            // CallConvKind_Jnccall_arm32,
		llvm::CallingConv::C,            // CallConvKind_Jnccall_arm64,
		llvm::CallingConv::C,            // CallConvKind_Cdecl_msc32,
		llvm::CallingConv::C,            // CallConvKind_Cdecl_msc64,
		llvm::CallingConv::C,            // CallConvKind_Cdecl_gcc32,
		llvm::CallingConv::C,            // CallConvKind_Cdecl_gcc64,
		llvm::CallingConv::C,            // CallConvKind_Cdecl_arm32,
		llvm::CallingConv::C,            // CallConvKind_Cdecl_arm64,
		llvm::CallingConv::X86_StdCall,  // CallConvKind_Stdcall_msc32,
		llvm::CallingConv::X86_StdCall,  // CallConvKind_Stdcall_gcc32,
		llvm::CallingConv::X86_ThisCall, // CallConvKind_Thiscall_msc32,
	};

	return (size_t)callConvKind < countof(llvmCallConvTable) ?
		llvmCallConvTable[callConvKind] :
		llvmCallConvTable[CallConvKind_Undefined];
}

uint_t
getCallConvFlags(CallConvKind callConvKind) {
	uint_t flagTable[CallConvKind__Count] = {
		0,                      // CallConvKind_Undefined = 0,
		CallConvFlag_Jnccall,   // CallConvKind_Jnccall_msc32,
		CallConvFlag_Jnccall,   // CallConvKind_Jnccall_msc64,
		CallConvFlag_Jnccall,   // CallConvKind_Jnccall_gcc32,
		CallConvFlag_Jnccall,   // CallConvKind_Jnccall_gcc64,
		CallConvFlag_Jnccall,   // CallConvKind_Jnccall_arm32,
		CallConvFlag_Jnccall,   // CallConvKind_Jnccall_arm64,

		CallConvFlag_Cdecl |    // CallConvKind_Cdecl_msc32,
		CallConvFlag_Msc |
		CallConvFlag_UnsafeVarArg,

		CallConvFlag_Cdecl |    // CallConvKind_Cdecl_msc64,
		CallConvFlag_Msc |
		CallConvFlag_UnsafeVarArg,

		CallConvFlag_Cdecl |    // CallConvKind_Cdecl_gcc32,
		CallConvFlag_Gcc |
		CallConvFlag_UnsafeVarArg,

		CallConvFlag_Cdecl |    // CallConvKind_Cdecl_gcc64,
		CallConvFlag_Gcc |
		CallConvFlag_UnsafeVarArg,

		CallConvFlag_Cdecl |    // CallConvKind_Cdecl_arm32,
		CallConvFlag_Gcc |
		CallConvFlag_UnsafeVarArg,

		CallConvFlag_Cdecl |    // CallConvKind_Cdecl_arm64,
		CallConvFlag_Gcc |
		CallConvFlag_UnsafeVarArg,

		CallConvFlag_Stdcall |  // CallConvKind_Stdcall_msc32,
		CallConvFlag_Msc |
		CallConvFlag_NoVarArg,

		CallConvFlag_Stdcall |  // CallConvKind_Stdcall_gcc32,
		CallConvFlag_Gcc |
		CallConvFlag_NoVarArg,
	};

	return (size_t)callConvKind < countof(flagTable) ?
		flagTable[callConvKind] :
		flagTable[CallConvKind_Undefined];
}

const char*
getCallConvString(CallConvKind callConvKind) {
	static const char* stringTable[CallConvKind__Count] = {
		"undefinded-call-conv",  // CallConvKind_Undefined = 0,
		"jnccall-msc32",         // CallConvKind_Jnccall_msc32,
		"jnccall-msc64",         // CallConvKind_Jnccall_msc64,
		"jnccall-gcc32",         // CallConvKind_Jnccall_gcc32,
		"jnccall-gcc64",         // CallConvKind_Jnccall_gcc64,
		"jnccall-arm32",         // CallConvKind_Jnccall_arm32,
		"jnccall-arm64",         // CallConvKind_Jnccall_arm64,
		"cdecl-msc32",           // CallConvKind_Cdecl_msc32,
		"cdecl-msc64",           // CallConvKind_Cdecl_msc64,
		"cdecl-gcc32",           // CallConvKind_Cdecl_gcc32,
		"cdecl-gcc64",           // CallConvKind_Cdecl_gcc64,
		"cdecl-arm32",           // CallConvKind_Cdecl_gcc32,
		"cdecl-arm64",           // CallConvKind_Cdecl_gcc64,
		"stdcall-msc32",         // CallConvKind_Stdcall_msc32,
		"stdcall-gcc32",         // CallConvKind_Stdcall_gcc32,
	};

	return (size_t)callConvKind < countof(stringTable) ?
		stringTable[callConvKind] :
		stringTable[CallConvKind_Undefined];
}

const char*
getCallConvDisplayString(CallConvKind callConvKind) {
	static const char* stringTable[CallConvKind__Count] = {
		"undefinded-call-conv",  // CallConvKind_Undefined = 0,
		"jnccall",               // CallConvKind_Jnccall_msc32,
		"jnccall",               // CallConvKind_Jnccall_msc64,
		"jnccall",               // CallConvKind_Jnccall_gcc32,
		"jnccall",               // CallConvKind_Jnccall_gcc64,
		"jnccall",               // CallConvKind_Jnccall_arm32,
		"jnccall",               // CallConvKind_Jnccall_arm64,
		"cdecl",                 // CallConvKind_Cdecl_msc32,
		"cdecl",                 // CallConvKind_Cdecl_msc64,
		"cdecl",                 // CallConvKind_Cdecl_gcc32,
		"cdecl",                 // CallConvKind_Cdecl_gcc64,
		"cdecl",                 // CallConvKind_Cdecl_arm32,
		"cdecl",                 // CallConvKind_Cdecl_arm64,
		"stdcall",               // CallConvKind_Stdcall_msc32,
		"stdcall",               // CallConvKind_Stdcall_gcc32,
	};

	return (size_t)callConvKind < countof(stringTable) ?
		stringTable[callConvKind] :
		stringTable[CallConvKind_Undefined];
}

const char*
getCallConvSignature(CallConvKind callConvKind) {
	static const char* stringTable[CallConvKind__Count] = {
		"CC0",    // CallConvKind_Undefined = 0,
		"JM4",    // CallConvKind_Jnccall_msc32,
		"JM8",    // CallConvKind_Jnccall_msc64,
		"JG4",    // CallConvKind_Jnccall_gcc32,
		"JG8",    // CallConvKind_Jnccall_gcc64,
		"JA4",    // CallConvKind_Jnccall_arm32,
		"JA8",    // CallConvKind_Jnccall_arm64,
		"CM4",    // CallConvKind_Cdecl_msc32,
		"CM8",    // CallConvKind_Cdecl_msc64,
		"CG4",    // CallConvKind_Cdecl_gcc32,
		"CG8",    // CallConvKind_Cdecl_gcc64,
		"CA4",    // CallConvKind_Cdecl_arm32,
		"CA8",    // CallConvKind_Cdecl_arm64,
		"SM4",    // CallConvKind_Stdcall_msc32,
		"SG4",    // CallConvKind_Stdcall_gcc32,
		"TM4",    // CallConvKind_Thiscall_msc32,
	};

	return (size_t)callConvKind < countof(stringTable) ?
		stringTable[callConvKind] :
		stringTable[CallConvKind_Undefined];
}

CallConvKind
getCallConvKindFromModifiers(uint_t modifiers) {
#if (_JNC_CPU_X86)
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

//..............................................................................

} // namespace ct
} // namespace jnc
