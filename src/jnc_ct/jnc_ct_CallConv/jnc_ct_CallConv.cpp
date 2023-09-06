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
#include "jnc_ct_CallConv.h"
#include "jnc_ct_Decl.h"
#include "jnc_ct_Module.h"

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
		"JA4",    // CallConvKind_Jnccall_gcc32,
		"JA8",    // CallConvKind_Jnccall_gcc64,
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

CallConv::CallConv() {
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);

	m_callConvKind = CallConvKind_Undefined;
}

llvm::FunctionType*
CallConv::prepareFunctionType(FunctionType* functionType) {
	sl::Array<FunctionArg*> argArray = functionType->getArgArray();
	size_t argCount = argArray.getCount();

	char buffer[256];
	sl::Array<llvm::Type*> llvmArgTypeArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	llvmArgTypeArray.setCount(argCount);

	for (size_t i = 0; i < argCount; i++)
		llvmArgTypeArray[i] = argArray[i]->getType()->getLlvmType();

	functionType->m_llvmType = llvm::FunctionType::get(
		functionType->getReturnType()->getLlvmType(),
		llvm::ArrayRef<llvm::Type*> (llvmArgTypeArray, argCount),
		(functionType->getFlags() & FunctionTypeFlag_VarArg) != 0
	);

	return (llvm::FunctionType*)functionType->m_llvmType;
}

llvm::Function*
CallConv::createLlvmFunction(
	FunctionType* functionType,
	const sl::StringRef& name
) {
	llvm::FunctionType* llvmType = (llvm::FunctionType*)functionType->getLlvmType();
	llvm::Function* llvmFunction = llvm::Function::Create(
		llvmType,
		llvm::Function::ExternalLinkage,
		name >> toLlvm,
		m_module->getLlvmModule()
	);

	llvm::CallingConv::ID llvmCallConv = getLlvmCallConv();
	if (llvmCallConv)
		llvmFunction->setCallingConv(llvmCallConv);

	return llvmFunction;
}

llvm::CallInst*
CallConv::call(
	const Value& calleeValue,
	FunctionType* functionType,
	sl::BoxList<Value>* argValueList,
	Value* resultValue
) {
	return m_module->m_llvmIrBuilder.createCall(
		calleeValue,
		functionType,
		*argValueList,
		resultValue
	);
}

llvm::ReturnInst*
CallConv::ret(
	Function* function,
	const Value& value
) {
	return m_module->m_llvmIrBuilder.createRet(value);
}

Value
CallConv::getThisArgValue(Function* function) {
	ASSERT(function->isMember());

	llvm::Function::arg_iterator llvmArg = function->getLlvmFunction()->arg_begin();
	return Value(&*llvmArg, function->getThisArgType());
}

Value
CallConv::getArgValue(
	llvm::Value* llvmValue,
	FunctionType* functionType,
	size_t argIdx
) {
	FunctionArg* arg = functionType->m_argArray[argIdx];
	return Value(llvmValue, arg->getType());
}

void
CallConv::getArgValueArrayImpl(
	Function* function,
	Value* argValueArray,
	size_t argCount,
	size_t baseLlvmArgIdx
) {
	llvm::Function::arg_iterator llvmArg = function->getLlvmFunction()->arg_begin();
	for (size_t i = 0; i < baseLlvmArgIdx; i++)
		llvmArg++;

	FunctionType* functionType = function->getType();
	for (size_t i = 0; i < argCount; i++, llvmArg++) {
		Value argValue = getArgValue(&*llvmArg, functionType, i);
		argValueArray[i] = argValue;
	}
}

void
CallConv::createArgVariablesImpl(
	Function* function,
	size_t baseLlvmArgIdx
) {
	llvm::Function::arg_iterator llvmArg = function->getLlvmFunction()->arg_begin();
	for (size_t i = 0; i < baseLlvmArgIdx; i++)
		llvmArg++;

	size_t i = 0;
	if (function->isMember()) { // skip this
		i++;
		llvmArg++;
	}

	sl::Array<FunctionArg*> argArray = function->getType()->getArgArray();
	size_t argCount = argArray.getCount();
	for (; i < argCount; i++, llvmArg++) {
		FunctionArg* arg = argArray[i];
		if (!arg->isNamed())
			continue;

		llvm::Value* llvmArgValue = &*llvmArg;

		Variable* argVariable = m_module->m_variableMgr.createArgVariable(arg, i);
		function->getScope()->addItem(argVariable);

		Value argValue(llvmArgValue, arg->getType());
		m_module->m_llvmIrBuilder.createStore(argValue, argVariable);
	}
}

void
CallConv::addIntExtAttributes(
	llvm::CallInst* llvmInst,
	const sl::BoxList<Value>& argValueList
) {
	sl::ConstBoxIterator<Value> it = argValueList.getHead();
	for (size_t i = 1; it; it++, i++) {
		Type* type = it->getType();
		if (!(type->getTypeKindFlags() & TypeKindFlag_Integer) || type->getSize() >= sizeof(int))
			continue;

		if (type->getTypeKind() == TypeKind_Enum)
			type = ((EnumType*)type)->getBaseType();

		llvmInst->addAttribute(
			i,
			(type->getTypeKindFlags() & TypeKindFlag_Unsigned) ?
				llvm::Attribute::AttrKind::ZExt :
				llvm::Attribute::AttrKind::SExt
		);
	}
}

//..............................................................................

} // namespace ct
} // namespace jnc
