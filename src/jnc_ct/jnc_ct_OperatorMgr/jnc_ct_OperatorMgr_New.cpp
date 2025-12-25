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
#include "jnc_ct_ParseContext.h"
#include "jnc_ct_Parser.llk.h"
#include "jnc_rt_Runtime.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
OperatorMgr::newOperator(
	Type* type,
	const Value& elementCountValue,
	sl::BoxList<Value>* argValueList,
	Value* resultValue
) {
	ASSERT(!(type->getTypeKindFlags() & TypeKindFlag_Import));

	bool result = type->ensureLayout();
	if (!result)
		return false;

	if (type->getTypeKind() == TypeKind_Class) {
		result = ((ClassType*)type)->ensureCreatable();
		if (!result)
			return false;
	}

	return
		gcHeapAllocate(type, elementCountValue, resultValue) &&
		construct(*resultValue, argValueList);
}

const void*
OperatorMgr::createThinDataPtrToConst(const Value& value) {
	ASSERT(value.getValueKind() == ValueKind_Const);

	if (m_module->getCompileState() < ModuleCompileState_Compiled)
		return m_module->m_constMgr.saveValue(value).getConstData();

	TRACE("-- WARNING: creating a thin pointer to a newly allocated object on GC heap");
	rt::Runtime* runtime = rt::getCurrentThreadRuntime();
	return runtime ?
		runtime->getGcHeap()->allocateData(value.getType(), value.getConstData()).m_p :
		NULL;
}

DataPtr
OperatorMgr::createDataPtrToConst(const Value& value) {
	ASSERT(value.getValueKind() == ValueKind_Const);

	if (m_module->getCompileState() < ModuleCompileState_Compiled) {
		DataPtr ptr;
		ptr.m_p = (void*)m_module->m_constMgr.saveValue(value).getConstData();
		ptr.m_validator = m_module->m_constMgr.createConstDataPtrValidator(ptr.m_p, value.getType());
		return ptr;
	}

	rt::Runtime* runtime = rt::getCurrentThreadRuntime();
	return runtime ?
		runtime->getGcHeap()->allocateData(value.getType(), value.getConstData()) :
		g_nullDataPtr;
}

DataPtr
OperatorMgr::createDataPtrToLiteral(const sl::StringRef& string) {
	if (m_module->getCompileState() < ModuleCompileState_Compiled) {
		Value value;
		value.setCharArray(string, m_module);
		return createDataPtrToConst(value);
	}

	rt::Runtime* runtime = rt::getCurrentThreadRuntime();
	if (!runtime)
		return g_nullDataPtr;

	DataPtr ptr = rt::getCurrentThreadRuntime()->getGcHeap()->allocateArray(
		m_module->m_typeMgr.getPrimitiveType(TypeKind_Char),
		string.getLength() + 1
	);

	memcpy(ptr.m_p, string.cp(), string.getLength());
	return ptr;
}

bool
OperatorMgr::memSet(
	const Value& value,
	char c,
	size_t size,
	bool isVolatile
) {
	Value ptrValue;

	bool result = castOperator(value, m_module->m_typeMgr.getStdType(StdType_ByteThinPtr), &ptrValue);
	if (!result)
		return false;

	if (!m_module->hasCodeGen())
		return true;

	Value argValueArray[] = {
		ptrValue,
		Value(c, m_module->m_typeMgr.getPrimitiveType(TypeKind_Int8)),
		Value(size, m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32)),
#if (LLVM_VERSION_MAJOR < 7)
		Value(1, m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32)),
#endif
		Value(isVolatile, m_module->m_typeMgr.getPrimitiveType(TypeKind_Bool)),
	};

	Function* llvmMemset = m_module->m_functionMgr.getStdFunction(StdFunc_LlvmMemset);
	m_module->m_llvmIrBuilder.createCall(
		llvmMemset,
		llvmMemset->getType(),
		argValueArray,
		countof(argValueArray),
		m_module->m_typeMgr.getPrimitiveType(TypeKind_Void),
		NULL
	);

	return true;
}

bool
OperatorMgr::memCpy(
	StdFunc stdFunc,
	const Value& dstValue,
	const Value& srcValue,
	size_t size,
	bool isVolatile
) {
	Value dstPtrValue;
	Value srcPtrValue;

	bool result =
		castOperator(dstValue, m_module->m_typeMgr.getStdType(StdType_ByteThinPtr), &dstPtrValue) &&
		castOperator(srcValue, m_module->m_typeMgr.getStdType(StdType_CharConstThinPtr), &srcPtrValue);

	if (!result)
		return false;

	if (!m_module->hasCodeGen())
		return true;

	Value argValueArray[] = {
		dstPtrValue,
		srcPtrValue,
		Value(size, m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32)),
#if (LLVM_VERSION_MAJOR < 7)
		Value(1, m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32)),
#endif
		Value(isVolatile, m_module->m_typeMgr.getPrimitiveType(TypeKind_Bool)),
	};

	Function* llvmMemcpy = m_module->m_functionMgr.getStdFunction(stdFunc);
	m_module->m_llvmIrBuilder.createCall(
		llvmMemcpy,
		llvmMemcpy->getType(),
		argValueArray,
		countof(argValueArray),
		NULL
	);

	return true;
}

void
OperatorMgr::zeroInitialize(const Value& value) {
	if (!m_module->hasCodeGen())
		return;

	ASSERT(value.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
	Type* type = ((DataPtrType*)value.getType())->getTargetType();

	if (type->getSize() <= TypeSizeLimit_StoreSize)
		m_module->m_llvmIrBuilder.createStore(type->getZeroValue(), value);
	else
		memSet(value, 0, type->getSize());
}

bool
OperatorMgr::construct(
	const Value& rawOpValue,
	sl::BoxList<Value>* argList
) {
	Type* type = rawOpValue.getType();
	TypeKind ptrTypeKind = type->getTypeKind();

	switch (ptrTypeKind) {
	case TypeKind_DataRef:
	case TypeKind_DataPtr:
		type = ((DataPtrType*)type)->getTargetType();
		break;

	case TypeKind_ClassPtr:
	case TypeKind_ClassRef:
		type = ((ClassPtrType*)type)->getTargetType();
		break;

	default:
		err::setFormatStringError("'%s' is not a pointer or reference", type->getTypeString().sz());
		return false;
	}

	OverloadableFunction constructor;

	if (type->getTypeKind() == TypeKind_String) {
		if (!argList || argList->isEmpty())
			return true;
		constructor = m_module->m_functionMgr.getStdFunction(StdFunc_StringConstruct);
	} else if (type->getTypeKindFlags() & TypeKindFlag_Derivable) {
		DerivableType* derivableType = (DerivableType*)type;
		constructor = ((DerivableType*)type)->getConstructor();
		if (constructor &&
			constructor->getItemKind() == ModuleItemKind_Function &&
			!checkAccess(constructor.getFunction())
		)
			return false;
	}

	if (!constructor) {
		if (argList && !argList->isEmpty()) {
			if (argList->getCount() == 1)
				return binaryOperator(BinOpKind_Assign, rawOpValue, *argList->getHead());

			err::setFormatStringError("'%s' has no constructor", type->getTypeString().sz());
			return false;
		}

		return true;
	}

	sl::BoxList<Value> emptyArgList;
	if (!argList)
		argList = &emptyArgList;

	Value opValue = rawOpValue;
	if (ptrTypeKind == TypeKind_DataRef || ptrTypeKind == TypeKind_ClassRef) {
		bool result = unaryOperator(UnOpKind_Addr, &opValue);
		if (!result)
			return false;
	}

	argList->insertHead(opValue);

	return callOperator(constructor, argList);
}

bool
OperatorMgr::initialize(
	const Value& rawValue,
	sl::List<Token>* constructorTokenList,
	sl::List<Token>* initializerTokenList
) {
	Value value = rawValue;
	if (rawValue.getType()->getTypeKind() == TypeKind_DataRef)
		value.overrideType(((DataPtrType*)rawValue.getType())->getUnConstPtrType());
	else if (rawValue.getType()->getTypeKind() == TypeKind_ClassRef)
		value.overrideType(((ClassPtrType*)rawValue.getType())->getUnConstPtrType());

	bool hasConstructor = !constructorTokenList->isEmpty();
	bool hasInitializer = !initializerTokenList->isEmpty();

	if (hasConstructor) {
		Parser parser(m_module, NULL, Parser::Mode_Compile);
		bool result = parser.parseTokenList(SymbolKind_expression_or_empty_list_save, constructorTokenList);
		if (!result)
			return false;

		sl::BoxList<Value> argList;
		parser.takeOverLastExpressionValueList(&argList);
		result = construct(value, &argList);
		if (!result)
			return false;
	}

	if (!hasInitializer)
		return hasConstructor ? true : construct(value);

	Parser parser(m_module, NULL, Parser::Mode_Compile);
	const Token* token = *initializerTokenList->getHead();
	switch (token->m_token) {
	case TokenKind_Body:
		parser.m_curlyInitializerTargetValue = value;
		return
			(hasConstructor || construct(value)) &&
			parser.parseBody(SymbolKind_curly_initializer, token->m_pos, token->m_data.m_string);

	case '{':
		parser.m_curlyInitializerTargetValue = value;
		return
			(hasConstructor || construct(value)) &&
			parser.parseTokenList(SymbolKind_curly_initializer, initializerTokenList);

	default:
		bool result = parser.parseTokenList(SymbolKind_expression_save, initializerTokenList);
		if (!result)
			return false;

		if (hasConstructor)
			return m_module->m_operatorMgr.binaryOperator(BinOpKind_Assign, value, parser.getLastExpressionValue());

		sl::BoxList<Value> argList;
		argList.insertTail(parser.getLastExpressionValue());
		return construct(value, &argList);
	}
}

bool
OperatorMgr::parseReactiveInitializer(
	const Value& value,
	sl::List<Token>* tokenList
) {
	Parser parser(m_module, NULL, Parser::Mode_Compile);

	m_module->m_controlFlowMgr.enterReactiveExpression();

	bool result =
		parser.parseTokenList(SymbolKind_expression_save, tokenList) &&
		m_module->m_operatorMgr.binaryOperator(BinOpKind_Assign, value, parser.getLastExpressionValue());

	m_module->m_controlFlowMgr.finalizeReactiveExpressionStmt();
	return result;
}

bool
OperatorMgr::parseFunctionArgDefaultValue(
	ModuleItemDecl* decl,
	const Value& thisValue,
	const sl::List<Token>& tokenList,
	Value* resultValue
) {
	Value prevThisValue = m_module->m_functionMgr.overrideThisValue(thisValue);
	bool result = parseFunctionArgDefaultValue(decl, tokenList, resultValue);
	if (!result)
		return false;

	m_module->m_functionMgr.overrideThisValue(prevThisValue);
	return true;
}

bool
OperatorMgr::parseFunctionArgDefaultValue(
	ModuleItemDecl* decl,
	const sl::List<Token>& tokenList,
	Value* resultValue
) {
	ParseContext parseContext(ParseContextKind_Expression, m_module, *decl);
	Parser parser(m_module, decl->getPragmaConfig(), Parser::Mode_Compile);

	m_module->m_namespaceMgr.lockSourcePos();
	sl::List<Token> tmpTokenList;
	cloneTokenList(&tmpTokenList, tokenList);
	bool result = parser.parseTokenList(SymbolKind_expression_save, &tmpTokenList);
	m_module->m_namespaceMgr.unlockSourcePos();

	*resultValue = parser.getLastExpressionValue();
	return result;
}

bool
OperatorMgr::parseExpression(
	sl::List<Token>* expressionTokenList,
	Value* resultValue
) {
	Parser parser(m_module, NULL, Parser::Mode_Compile);

	bool result = parser.parseTokenList(SymbolKind_expression_save, expressionTokenList);
	if (!result)
		return false;

	*resultValue = parser.getLastExpressionValue();
	return true;
}
bool
OperatorMgr::parseConstIntegerExpression(
	sl::List<Token>* expressionTokenList,
	int64_t* integer
) {
	Value value;
	bool result = parseExpression(expressionTokenList, &value);
	if (!result)
		return false;

	if (value.getValueKind() != ValueKind_Const ||
		!(value.getType()->getTypeKindFlags() & TypeKindFlag_Integer)
	) {
		err::setError("expression is not integer constant");
		return false;
	}

	*integer = 0;
	memcpy(integer, value.getConstData(), value.getType()->getSize());
	return true;
}

size_t
OperatorMgr::getAutoSizeArrayElementCount(
	ArrayType* arrayType,
	const sl::List<Token>& initializerTokenList
) {
	const Token* token = *initializerTokenList.getHead();
	switch (token->m_token) {
	case TokenKind_Literal:
	case TokenKind_BinLiteral:
		return getAutoSizeArrayElementCount_literal(initializerTokenList);

	case TokenKind_Body:
		return getAutoSizeArrayElementCount_curly(arrayType, token->m_pos, token->m_data.m_string);

	case '{':
		return getAutoSizeArrayElementCount_curly(arrayType, initializerTokenList);

	default:
		err::setError("invalid initializer for auto-size-array");
		return -1;
	}
}

// it's both more efficient AND easier to parse these by hand

size_t
OperatorMgr::getAutoSizeArrayElementCount_literal(const sl::List<Token>& initializerTokenList) {
	size_t elementCount = 0;

	sl::ConstIterator<Token> token = initializerTokenList.getHead();
	for (; token; token++) {
		switch (token->m_token) {
		case TokenKind_Literal:
			elementCount += token->m_data.m_string.getLength();
			break;

		case TokenKind_BinLiteral:
			elementCount += token->m_data.m_binData.getCount();
			break;
		}
	}

	if (initializerTokenList.getTail()->m_token == TokenKind_Literal)
		elementCount++;

	return elementCount;
}

size_t
OperatorMgr::getAutoSizeArrayElementCount_curly(
	ArrayType* arrayType,
	const lex::LineColOffset& pos,
	const sl::StringRef& initializer
) {
	Unit* unit = m_module->m_unitMgr.getCurrentUnit();
	ASSERT(unit);

	Lexer lexer;
	lexer.create(unit->getFilePath(), initializer);
	lexer.setLineColOffset(pos);

	sl::List<Token> tokenList;

	for (;;) {
		const Token* token = lexer.getToken();
		switch (token->m_token) {
		case TokenKind_Eof: // no need to add EOF token
			return getAutoSizeArrayElementCount_curly(arrayType, tokenList);

		case TokenKind_Error:
			err::setFormatStringError("invalid character '\\x%02x'", (uchar_t) token->m_data.m_integer);
			lex::pushSrcPosError(unit->getFilePath(), token->m_pos);
			return -1;
		}

		tokenList.insertTail(lexer.takeToken());
	}
}

size_t
OperatorMgr::getAutoSizeArrayElementCount_curly(
	ArrayType* arrayType,
	const sl::List<Token>& initializer
) {
	intptr_t level = 0;
	size_t elementCount = 0;

	bool isCharArray = arrayType->getElementType()->getTypeKind() == TypeKind_Char;
	bool isElement = false;

	sl::ConstIterator<Token> token = initializer.getHead();
	for (; token; token++) {
		switch (token->m_token) {
		case '{':
			if (level == 1)
				isElement = true;

			level++;
			break;

		case '}':
			if (level == 1 && isElement) {
				elementCount++;
				isElement = false;
			}

			level--;
			break;

		case ',':
			if (level == 1 && isElement) {
				elementCount++;
				isElement = false;
			}

			break;

		case TokenKind_Literal:
			if (level == 1) {
				if (isCharArray)
					elementCount += token->m_data.m_string.getLength();

				isElement = true; // account for null-terminator
			}

			break;

		case TokenKind_BinLiteral:
			if (level == 1) {
				if (isCharArray) {
					elementCount += token->m_data.m_binData.getCount();
					isElement = false;
				} else {
					isElement = true;
				}
			}

			break;

		default:
			if (level == 1)
				isElement = true;
		}
	}

	return elementCount;
}

bool
OperatorMgr::gcHeapAllocate(
	Type* type,
	const Value& rawElementCountValue,
	Value* resultValue
) {
	bool result;

	if (!m_module->hasCodeGen()) {
		resultValue->setType(
			type->getTypeKind() == TypeKind_Class ?
				(Type*)((ClassType*)type)->getClassPtrType() :
				type->getDataPtrType()
			);

		return true;
	}

	Value typeValue(&type, m_module->m_typeMgr.getStdType(StdType_ByteThinPtr));

	Function* allocate;
	sl::BoxList<Value> allocateArgValueList;
	allocateArgValueList.insertTail(typeValue);

	Value ptrValue;

	if (type->getTypeKind() == TypeKind_Class) {
		if (type->getFlags() & (ClassTypeFlag_HasAbstractMethods | ClassTypeFlag_OpaqueNonCreatable)) {
			err::setFormatStringError("cannot instantiate '%s'", type->getTypeString().sz());
			return false;
		}

		allocate = m_module->m_functionMgr.getStdFunction(StdFunc_AllocateClass);
	} else if (!rawElementCountValue) {
		allocate = m_module->m_functionMgr.getStdFunction(StdFunc_AllocateData);
	} else {
		allocate = m_module->m_functionMgr.getStdFunction(StdFunc_AllocateArray);

		Value countValue;
		result = castOperator(rawElementCountValue, TypeKind_SizeT, &countValue);
		if (!result)
			return false;

		allocateArgValueList.insertTail(countValue);
	}

	result = m_module->m_operatorMgr.callOperator(
		allocate,
		&allocateArgValueList,
		&ptrValue
	);


	if (type->getTypeKind() == TypeKind_Class)
		m_module->m_llvmIrBuilder.createBitCast(ptrValue, ((ClassType*)type)->getClassPtrType(), resultValue);
	else
		resultValue->overrideType(ptrValue, type->getDataPtrType());

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
