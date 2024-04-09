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
#include "jnc_ct_OperatorMgr.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_ArrayType.h"

namespace jnc {
namespace ct {

//..............................................................................

#if (_JNC_DEBUG)
void
OperatorMgr::callTraceFunction(
	const sl::StringRef& functionName,
	const sl::StringRef& string
) {
	Function* function = (Function*)m_module->m_namespaceMgr.getGlobalNamespace()->findItem(functionName).m_item;
	ASSERT(function && function->getItemKind() == ModuleItemKind_Function);

	Value literalValue;
	literalValue.setCharArray(string, m_module);
	m_module->m_operatorMgr.callOperator(function, literalValue);
}

void
OperatorMgr::traceBlock(BasicBlock* block) {
	llvm::BasicBlock* llvmBlock = block->getLlvmBlock();
	llvm::BasicBlock::iterator it = llvmBlock->begin();

	m_module->m_llvmIrBuilder.setInsertPoint(&*it);
	m_module->m_operatorMgr.callTraceFunction("print_u", block->getName() + "\n------------\n");

	for (; it != llvmBlock->end() && !it->isTerminator(); it++) {
		std::string s;
		llvm::raw_string_ostream stream(s);
		it->print(stream);
		stream.flush();
		s += '\n';

		m_module->m_llvmIrBuilder.setInsertPoint(&*it);
		m_module->m_operatorMgr.callTraceFunction("print_u", s.c_str ());
	}
}
#endif

bool
OperatorMgr::closureOperator(
	const Value& rawOpValue,
	sl::BoxList<Value>* argValueList,
	Value* resultValue
) {
	if (argValueList->isEmpty()) {
		err::setError("closure operator without arguments has no effect");
		return false;
	}

	Value opValue;
	bool result = prepareOperand(rawOpValue, &opValue);
	if (!result)
		return false;

	TypeKind typeKind = opValue.getType()->getTypeKind();
	if (typeKind != TypeKind_FunctionRef && typeKind != TypeKind_FunctionPtr) {
		err::setFormatStringError(
			"closure operator cannot be applied to '%s'",
			opValue.getType()->getTypeString().sz()
		);
		return false;
	}

	*resultValue = opValue;

	Closure* closure = resultValue->getClosure();
	if (!closure)
		closure = resultValue->createClosure();

	closure->append(*argValueList);
	return true;
}

Type*
OperatorMgr::getCdeclVarArgType(Type* type) {
	for (;;) {
		Type* prevType = type;

		TypeKind typeKind = type->getTypeKind();
		switch (typeKind) {
		case TypeKind_DataRef:
			type = ((DataPtrType*)type)->getTargetType();
			break;

		case TypeKind_ClassRef:
			type = ((ClassPtrType*)type)->getTargetType()->getClassPtrType(
				((ClassPtrType*)type)->getPtrTypeKind(),
				type->getFlags() & PtrTypeFlag__All
			);
			break;

		case TypeKind_FunctionRef:
			type = ((FunctionPtrType*)type)->getTargetType()->getFunctionPtrType(
				((FunctionPtrType*)type)->getPtrTypeKind(),
				type->getFlags() & PtrTypeFlag__All
			);
			break;

		case TypeKind_PropertyRef:
			type = ((PropertyPtrType*)type)->getTargetType()->getReturnType();
			break;

		case TypeKind_Enum:
			type = ((EnumType*)type)->getBaseType();
			break;

		case TypeKind_Float:
			type = m_module->m_typeMgr.getPrimitiveType(TypeKind_Double);
			break;

		case TypeKind_Array:
			type = ((ArrayType*)type)->getElementType()->getDataPtrType_c(TypeKind_DataPtr, PtrTypeFlag_Const);
			break;

		case TypeKind_DataPtr:
			type = ((DataPtrType*)type)->getTargetType()->getDataPtrType_c(TypeKind_DataPtr, PtrTypeFlag_Const);
			break;

		case TypeKind_String:
			type = m_module->m_typeMgr.getStdType(StdType_CharConstThinPtr);
			break;

		default:
			if (type->getTypeKindFlags() & TypeKindFlag_Integer) {
				type = type->getSize() > 4 ?
					m_module->m_typeMgr.getPrimitiveType(TypeKind_Int64) :
					m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32);
			}
		}

		if (type == prevType)
			return type;
	}
}

bool
OperatorMgr::callOperator(
	const Value& rawOpValue,
	sl::BoxList<Value>* argValueList,
	Value* resultValue
) {
	bool result;

	Value opValue;
	Value unusedReturnValue;
	sl::BoxList<Value> emptyArgValueList;

	if (!resultValue)
		resultValue = &unusedReturnValue;

	if (!argValueList)
		argValueList = &emptyArgValueList;

	result = prepareOperand(rawOpValue, &opValue, 0);
	if (!result)
		return false;

	if (opValue.getType()->getTypeKind() == TypeKind_ClassPtr) {
		ClassPtrType* ptrType = (ClassPtrType*)opValue.getType();

		OverloadableFunction callOperator = ptrType->getTargetType()->getCallOperator();
		if (!callOperator) {
			err::setFormatStringError("cannot call '%s'", ptrType->getTypeString().sz());
			return false;
		}

		if ((callOperator->getFlags() & MulticastMethodFlag_InaccessibleViaEventPtr) &&
			(ptrType->getFlags() & PtrTypeFlag_Event)) {
			err::setFormatStringError("'call' is inaccessible via 'event' pointer");
			return false;
		}

		Value objValue = opValue;
		opValue = callOperator;

		Closure* closure = opValue.createClosure();
		closure->insertThisArgValue(objValue);
	}

	Closure* closure = opValue.getClosure();
	if (closure) {
		result = closure->apply(argValueList);
		if (!result)
			return false;
	}

	ValueKind valueKind = opValue.getValueKind();
	switch (valueKind) {
		Function* function;
		Namespace* nspace;

	case ValueKind_FunctionOverload:
		function = opValue.getFunctionOverload()->chooseOverload(*argValueList);
		if (!function || !checkAccess(function))
			return false;

		result = opValue.trySetFunction(function);
		if (!result)
			return false;

		opValue.setClosure(closure);
		break;

	case ValueKind_Function:
		function = opValue.getFunction();
		if (function->isVirtual()) {
			result = getVirtualMethod(function, closure, &opValue);
			if (!result)
				return false;
		}

		return callImpl(opValue, function->getType(), argValueList, resultValue);

	case ValueKind_Namespace:
		nspace = opValue.getNamespace();
		if (nspace->getNamespaceKind() == NamespaceKind_Type) {
			NamedType* type = (NamedType*)nspace;
			if (type->getStdType() == StdType_StringStruct) {
				opValue = m_module->m_functionMgr.getStdFunction(StdFunc_StringCreate);
				break;
			}

			Value ptrValue;
			return
				newOperator(type, argValueList, &ptrValue) &&
				unaryOperator(UnOpKind_Indir, ptrValue, resultValue);
		}

		err::setFormatStringError("cannot call '%s'", nspace->getQualifiedName().sz());
		return false;
	}

	Type* opType = opValue.getType();
	if (!(opType->getTypeKindFlags() & TypeKindFlag_FunctionPtr) ||
		((FunctionPtrType*)opType)->getPtrTypeKind() == FunctionPtrTypeKind_Weak
	) {
		err::setFormatStringError("cannot call '%s'", opType->getTypeString().sz());
		return false;
	}

	FunctionPtrType* functionPtrType = (FunctionPtrType*)opValue.getType();
	return functionPtrType->hasClosure() ?
		callClosureFunctionPtr(opValue, argValueList, resultValue) :
		callImpl(opValue, functionPtrType->getTargetType(), argValueList, resultValue);
}

FunctionTypeOverload
OperatorMgr::getValueFunctionTypeOverload(
	const Value& rawOpValue,
	size_t* baseArgumentIdx
) {
	Value opValue;

	bool result = prepareOperand(rawOpValue, &opValue);
	if (!result)
		return NULL;

	*baseArgumentIdx = 0;

	if (opValue.getType()->getTypeKind() == TypeKind_ClassPtr) {
		ClassPtrType* ptrType = (ClassPtrType*)opValue.getType();
		OverloadableFunction callOperator = ptrType->getTargetType()->getCallOperator();
		if (!callOperator)
			return NULL;

		*baseArgumentIdx = 1;

		return callOperator->getItemKind() == ModuleItemKind_Function ?
			FunctionTypeOverload(callOperator.getFunction()->getType()) :
			FunctionTypeOverload(callOperator.getFunctionOverload()->getTypeOverload());
	}

	Closure* closure = opValue.getClosure();
	if (closure)
		*baseArgumentIdx = closure->getArgValueList()->getCount();

	if (opValue.getValueKind() == ValueKind_FunctionOverload)
		return opValue.getFunctionOverload()->getTypeOverload();

	if (opValue.getValueKind() == ValueKind_Function)
		return opValue.getFunction()->getType();

	Type* opType = opValue.getType();
	if (!(opType->getTypeKindFlags() & TypeKindFlag_FunctionPtr))
		return NULL;

	FunctionPtrType* functionPtrType = ((FunctionPtrType*)opType);
	if (functionPtrType->hasClosure())
		(*baseArgumentIdx)++;

	return functionPtrType->getTargetType();
}

bool
OperatorMgr::callOperatorVararg(
	Function* operatorVararg,
	DerivableType* type,
	const Value& value,
	Value* resultValue
) {
	Value tmpValue;

	Type* valueType = value.getType();

	if (valueType->getTypeKind() == TypeKind_DataRef &&
		((DataPtrType*)valueType)->getTargetType() == type) {
		return
			unaryOperator(UnOpKind_Addr, value, &tmpValue) &&
			callOperator(operatorVararg, tmpValue, resultValue);
	} else {
		Variable* tmpVariable = m_module->m_variableMgr.createSimpleStackVariable("tmpStruct", type);

		return
			storeDataRef(tmpVariable, value) &&
			unaryOperator(UnOpKind_Addr, tmpVariable, &tmpValue) &&
			callOperator(operatorVararg, tmpValue, resultValue);
	}
}

bool
OperatorMgr::castArgValueList(
	FunctionType* functionType,
	Closure* closure,
	sl::BoxList<Value>* argValueList
) {
	bool result;

	sl::Array<FunctionArg*> argArray = functionType->getArgArray();

	size_t formalArgCount = argArray.getCount();
	size_t actualArgCount = argValueList->getCount();

	bool isVarArg = (functionType->getFlags() & FunctionTypeFlag_VarArg) != 0;
	bool isCdeclVarArg = (functionType->getCallConv()->getFlags() & CallConvFlag_Cdecl) != 0;

	size_t commonArgCount;

	if (actualArgCount <= formalArgCount) {
		commonArgCount = actualArgCount;
	} else if (isVarArg) {
		commonArgCount = formalArgCount;
	} else {
		err::setFormatStringError("too many arguments in a call to '%s'", functionType->getTypeString().sz());
		return false;
	}

	size_t i = 0;
	sl::BoxIterator<Value> argValueIt = argValueList->getHead();

	// common for both formal and actual

	for (; i < commonArgCount; argValueIt++, i++) {
		Value argValue = *argValueIt;

		FunctionArg* arg = argArray[i];
		if (argValue.isEmpty()) {
			if (!arg->hasInitializer()) {
				err::setFormatStringError(
					"argument (%d) of '%s' has no default value",
					i + 1,
					functionType->getTypeString().sz()
				);
				return false;
			}

			result = parseFunctionArgDefaultValue(arg, closure, *arg->getInitializer(), &argValue);
			if (!result)
				return false;
		}

		Type* formalArgType = arg->getType();

		result =
			checkCastKind(argValue, formalArgType) &&
			castOperator(argValue, formalArgType, &*argValueIt); // store it in the same list entry

		if (!result)
			return false;
	}

	// default formal arguments

	for (; i < formalArgCount; i++) {
		Value argValue;

		FunctionArg* arg = argArray[i];
		if (!arg->hasInitializer()) {
			err::setFormatStringError(
				"argument (%d) of '%s' has no default value",
				i + 1,
				functionType->getTypeString().sz()
			);
			return false;
		}

		result = parseFunctionArgDefaultValue(arg, closure, *arg->getInitializer(), &argValue);
		if (!result)
			return false;

		Type* formalArgType = arg->getType();

		result =
			checkCastKind(argValue, formalArgType) &&
			castOperator(&argValue, formalArgType);

		if (!result)
			return false;

		argValueList->insertTail(argValue);
	}

	if (!isVarArg)
		return true;

	// vararg arguments

	if (!isCdeclVarArg) {
		err::setFormatStringError("only 'cdecl' vararg is currently supported");
		return false;
	}

	for (; argValueIt; argValueIt++) {
		Value argValue = *argValueIt;
		if (argValue.isEmpty()) {
			err::setFormatStringError("vararg arguments cannot be skipped");
			return false;
		}

		Value typeValue;
		bool result = prepareOperandType(argValue, &typeValue);
		if (!result)
			return false;

		Type* type = typeValue.getType();
		if (type->getTypeKindFlags() & TypeKindFlag_Derivable) {
			DerivableType* derivableType = (DerivableType*)type;

			Function* operatorVararg = derivableType->getOperatorCdeclVararg();
			if (!operatorVararg)
				operatorVararg = derivableType->getOperatorVararg();

			if (operatorVararg) {
				result = callOperatorVararg(operatorVararg, derivableType, &argValue);
				if (!result)
					return false;
			}
		}

		Type* formalArgType = argValue.getValueKind() == ValueKind_Null ?
			m_module->m_typeMgr.getStdType(StdType_ByteThinPtr) :
			getCdeclVarArgType(argValue.getType());

		result = castOperator(argValue, formalArgType, &*argValueIt); // store it in the same list entry
		if (!result)
			return false;
	}

	return true;
}

bool
OperatorMgr::callClosureFunctionPtr(
	const Value& opValue,
	sl::BoxList<Value>* argValueList,
	Value* resultValue
) {
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_FunctionPtr);

	FunctionPtrType* functionPtrType = (FunctionPtrType*)opValue.getType();
	FunctionType* functionType = functionPtrType->getTargetType();
	FunctionType* abstractMethodType = functionType->getStdObjectMemberMethodType();
	FunctionPtrType* functionThinPtrType = abstractMethodType->getFunctionPtrType(FunctionPtrTypeKind_Thin);

	Value pfnValue;
	Value ifaceValue;

	if (!m_module->hasCodeGen()) {
		pfnValue.setType(functionThinPtrType);
		ifaceValue.setType(m_module->m_typeMgr.getStdType(StdType_AbstractClassPtr));
	} else {
		m_module->m_llvmIrBuilder.createExtractValue(opValue, 0, NULL, &pfnValue);
		m_module->m_llvmIrBuilder.createExtractValue(opValue, 1, m_module->m_typeMgr.getStdType(StdType_AbstractClassPtr), &ifaceValue);
		m_module->m_llvmIrBuilder.createBitCast(pfnValue, functionThinPtrType, &pfnValue);
	}

	argValueList->insertHead(ifaceValue);
	return callImpl(pfnValue, abstractMethodType, argValueList, resultValue);
}

bool
OperatorMgr::callImpl(
	const Value& pfnValue,
	FunctionType* functionType,
	sl::BoxList<Value>* argValueList,
	Value* resultValue
) {
	uint_t flags = functionType->getFlags();

	if ((flags & FunctionTypeFlag_Unsafe) && !isUnsafeRgn()) {
		err::setFormatStringError("can only call unsafe functions from unsafe regions");
		return false;
	}

	bool result = castArgValueList(functionType, pfnValue.getClosure(), argValueList);
	if (!result)
		return false;

	if (!m_module->hasCodeGen()) {
		resultValue->setType(functionType->getReturnType());
		return true;
	}

	llvm::CallInst* llvmInst = functionType->getCallConv()->call(
		pfnValue,
		functionType,
		argValueList,
		resultValue
	);

	if (flags & FunctionTypeFlag_IntExtArgs)
		CallConv::addIntExtAttributes(llvmInst, *argValueList);

	if (resultValue->getType()->getFlags() & TypeFlag_GcRoot)
		m_module->m_gcShadowStackMgr.createTmpGcRoot(*resultValue);

	if (functionType->getFlags() & FunctionTypeFlag_ErrorCode)
		m_module->m_controlFlowMgr.checkErrorCode(*resultValue, functionType->getReturnType());

	m_callCount++;
	return true;
}

void
OperatorMgr::gcSafePoint() {
	if (!m_module->hasCodeGen())
		return;

	if (m_module->getCompileFlags() & ModuleCompileFlag_SimpleGcSafePoint) {
		Function* function = m_module->m_functionMgr.getStdFunction(StdFunc_GcSafePoint);
		m_module->m_llvmIrBuilder.createCall(function, function->getType(), NULL);
	} else {
		Variable* variable = m_module->m_variableMgr.getStdVariable(StdVariable_GcSafePointTrigger);

		Value ptrValue;
		Value value = m_module->m_typeMgr.getPrimitiveType(TypeKind_IntPtr)->getZeroValue();
		m_module->m_llvmIrBuilder.createLoad(variable, variable->getType(), &ptrValue);
		m_module->m_llvmIrBuilder.createRmw(
			llvm::AtomicRMWInst::Xchg,
			ptrValue,
			value,
#if (LLVM_VERSION < 0x030900)
			llvm::AcquireRelease,
#else
			llvm::AtomicOrdering::AcquireRelease,
#endif
			llvm::DefaultSynchronizationScope_vn,
			&value
		);
	}
}

//..............................................................................

} // namespace ct
} // namespace jnc
