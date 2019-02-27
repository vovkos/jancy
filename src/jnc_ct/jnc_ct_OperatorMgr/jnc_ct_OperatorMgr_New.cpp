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
#include "jnc_ct_Parser.llk.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
OperatorMgr::memSet(
	const Value& value,
	char c,
	size_t size,
	size_t alignment,
	bool isVolatile
	)
{
	Value ptrValue;

	bool result = castOperator(value, m_module->m_typeMgr.getStdType(StdType_BytePtr), &ptrValue);
	if (!result)
		return false;

	Value argValueArray[5] =
	{
		ptrValue,
		Value(c, m_module->m_typeMgr.getPrimitiveType(TypeKind_Int8)),
		Value(size, m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32)),
		Value(alignment, m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32)),
		Value(isVolatile, m_module->m_typeMgr.getPrimitiveType(TypeKind_Bool)),
	};

	Function* llvmMemset = m_module->m_functionMgr.getStdFunction(StdFunc_LlvmMemset);
	m_module->m_llvmIrBuilder.createCall(
		llvmMemset,
		llvmMemset->getType(),
		argValueArray,
		countof(argValueArray),
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
	size_t alignment,
	bool isVolatile
	)
{
	Value dstPtrValue;
	Value srcPtrValue;

	bool result =
		castOperator(dstValue, m_module->m_typeMgr.getStdType(StdType_BytePtr), &dstPtrValue) &&
		castOperator(srcValue, m_module->m_typeMgr.getStdType(StdType_BytePtr), &srcPtrValue);

	if (!result)
		return false;

	Value argValueArray[5] =
	{
		dstPtrValue,
		srcPtrValue,
		Value(size, m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32)),
		Value(alignment, m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32)),
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
OperatorMgr::zeroInitialize(const Value& value)
{
	ASSERT(value.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
	Type* type = ((DataPtrType*)value.getType())->getTargetType();

	if (type->getSize() <= TypeSizeLimit_StoreSize)
		m_module->m_llvmIrBuilder.createStore(type->getZeroValue(), value);
	else
		memSet(value, 0, type->getSize(), type->getAlignment());
}

bool
OperatorMgr::construct(
	const Value& rawOpValue,
	sl::BoxList<Value>* argList
	)
{
	Type* type = rawOpValue.getType();
	TypeKind ptrTypeKind = type->getTypeKind();

	switch(ptrTypeKind)
	{
	case TypeKind_DataRef:
	case TypeKind_DataPtr:
		type = ((DataPtrType*)type)->getTargetType();
		break;

	case TypeKind_ClassPtr:
	case TypeKind_ClassRef:
		type = ((ClassPtrType*)type)->getTargetType();
		break;

	default:
		err::setFormatStringError("'%s' is not a pointer or reference", type->getTypeString ().sz ());
		return false;
	}

	TypeKind typeKind = type->getTypeKind();

	Function* constructor = NULL;

	switch(typeKind)
	{
	case TypeKind_Struct:
	case TypeKind_Union:
		constructor = ((DerivableType*)type)->getConstructor();
		break;

	case TypeKind_Class:
		constructor = ((ClassType*)type)->getConstructor();
		break;
	}

	if (!constructor)
	{
		if (argList && !argList->isEmpty())
		{
			err::setFormatStringError("'%s' has no constructor", type->getTypeString ().sz ());
			return false;
		}

		return true;
	}

	DerivableType* derivableType = (DerivableType*)type;
	if (constructor->getAccessKind() != AccessKind_Public &&
		m_module->m_namespaceMgr.getAccessKind(derivableType) == AccessKind_Public)
	{
		err::setFormatStringError("'%s.construct' is protected", derivableType->getQualifiedName ().sz ());
		return false;
	}

	sl::BoxList<Value> emptyArgList;
	if (!argList)
		argList = &emptyArgList;

	Value opValue = rawOpValue;
	if (ptrTypeKind == TypeKind_DataRef || ptrTypeKind == TypeKind_ClassRef)
	{
		bool result = unaryOperator(UnOpKind_Addr, &opValue);
		if (!result)
			return false;
	}

	argList->insertHead(opValue);

	return callOperator(constructor, argList);
}

bool
OperatorMgr::parseInitializer(
	const Value& rawValue,
	const sl::ConstBoxList<Token>& constructorTokenList,
	const sl::ConstBoxList<Token>& initializerTokenList
	)
{
	bool result;

	Value value = rawValue;

	if (rawValue.getType()->getTypeKind() == TypeKind_DataRef)
		value.overrideType(((DataPtrType*)rawValue.getType())->getUnConstPtrType());
	else if (rawValue.getType()->getTypeKind() == TypeKind_ClassRef)
		value.overrideType(((ClassPtrType*)rawValue.getType())->getUnConstPtrType());

	sl::BoxList<Value> argList;
	if (!constructorTokenList.isEmpty())
	{
		Parser parser(m_module);
		parser.m_stage = Parser::Stage_Pass2;

		result = parser.parseTokenList(SymbolKind_expression_or_empty_list_save_list, constructorTokenList);
		if (!result)
			return false;

		sl::takeOver(&argList, &parser.m_expressionValueList);
	}

	result = construct(value, &argList);
	if (!result)
		return false;

	if (!initializerTokenList.isEmpty())
	{
		Parser parser(m_module);
		parser.m_stage = Parser::Stage_Pass2;

		if (initializerTokenList.getHead()->m_token == '{')
		{
			parser.m_curlyInitializerTargetValue = value;
			result = parser.parseTokenList(SymbolKind_curly_initializer, initializerTokenList);
			if (!result)
				return false;
		}
		else
		{
			result =
				parser.parseTokenList(SymbolKind_expression_save_value, initializerTokenList) &&
				m_module->m_operatorMgr.binaryOperator(BinOpKind_Assign, value, parser.m_expressionValue);

			if (!result)
				return false;
		}
	}

	return true;
}

bool
OperatorMgr::parseFunctionArgDefaultValue(
	ModuleItemDecl* decl,
	const Value& thisValue,
	const sl::ConstBoxList<Token> tokenList,
	Value* resultValue
	)
{
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
	const sl::ConstBoxList<Token> tokenList,
	Value* resultValue
	)
{
	Parser parser(m_module);
	parser.m_stage = Parser::Stage_Pass2;

	m_module->m_namespaceMgr.openNamespace(decl->getParentNamespace());
	m_module->m_namespaceMgr.lockSourcePos();

	bool result = parser.parseTokenList(SymbolKind_expression_save_value, tokenList);
	if (!result)
		return false;

	m_module->m_namespaceMgr.unlockSourcePos();
	m_module->m_namespaceMgr.closeNamespace();

	*resultValue = parser.m_expressionValue;
	return true;
}

bool
OperatorMgr::parseExpression(
	const sl::ConstBoxList<Token>& expressionTokenList,
	uint_t flags,
	Value* resultValue
	)
{
	Parser parser(m_module);
	parser.m_stage = Parser::Stage_Pass2;
	parser.m_flags |= flags;

	bool result = parser.parseTokenList(SymbolKind_expression_save_value, expressionTokenList);
	if (!result)
		return false;

	*resultValue = parser.m_expressionValue;
	return true;
}

bool
OperatorMgr::parseConstExpression(
	const sl::ConstBoxList<Token>& expressionTokenList,
	Value* resultValue
	)
{
	bool result = parseExpression(expressionTokenList, Parser::Flag_ConstExpression, resultValue);
	if (!result)
		return false;

	ASSERT(resultValue->getValueKind() == ValueKind_Const);
	return true;
}

bool
OperatorMgr::parseConstIntegerExpression(
	const sl::ConstBoxList<Token>& expressionTokenList,
	int64_t* integer
	)
{
	Value value;
	bool result = parseConstExpression(expressionTokenList, &value);
	if (!result)
		return false;

	if (!(value.getType()->getTypeKindFlags() & TypeKindFlag_Integer))
	{
		err::setFormatStringError("expression is not integer constant");
		return false;
	}

	*integer = 0;
	memcpy(integer, value.getConstData(), value.getType()->getSize());
	return true;
}

size_t
OperatorMgr::parseAutoSizeArrayInitializer(
	ArrayType* arrayType,
	const sl::ConstBoxList<Token>& initializerTokenList
	)
{
	int firstToken = initializerTokenList.getHead()->m_token;
	switch(firstToken)
	{
	case TokenKind_Literal:
	case TokenKind_BinLiteral:
		return parseAutoSizeArrayLiteralInitializer(initializerTokenList);

	case '{':
		return parseAutoSizeArrayCurlyInitializer(arrayType, initializerTokenList);

	default:
		err::setFormatStringError("invalid initializer for auto-size-array");
		return -1;
	}
}

// it's both more efficient AND easier to parse these by hand

size_t
OperatorMgr::parseAutoSizeArrayLiteralInitializer(const sl::ConstBoxList<Token>& initializerTokenList)
{
	size_t elementCount = 0;

	sl::ConstBoxIterator<Token> token = initializerTokenList.getHead();
	for (; token; token++)
	{
		switch(token->m_token)
		{
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
OperatorMgr::parseAutoSizeArrayCurlyInitializer(
	ArrayType* arrayType,
	const sl::ConstBoxList<Token>& initializerTokenList
	)
{
	intptr_t level = 0;
	size_t elementCount = 0;

	bool isCharArray = arrayType->getElementType()->getTypeKind() == TypeKind_Char;
	bool isElement = false;

	sl::ConstBoxIterator<Token> token = initializerTokenList.getHead();
	for (; token; token++)
	{
		switch(token->m_token)
		{
		case '{':
			if (level == 1)
				isElement = true;

			level++;
			break;

		case '}':
			if (level == 1 && isElement)
			{
				elementCount++;
				isElement = false;
			}

			level--;
			break;

		case ',':
			if (level == 1 && isElement)
			{
				elementCount++;
				isElement = false;
			}

			break;

		case TokenKind_Literal:
			if (level == 1)
			{
				if (isCharArray)
					elementCount += token->m_data.m_string.getLength();

				isElement = true; // account for null-terminator
			}

			break;

		case TokenKind_BinLiteral:
			if (level == 1)
			{
				if (isCharArray)
				{
					elementCount += token->m_data.m_binData.getCount();
					isElement = false;
				}
				else
				{
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
	)
{
	bool result;

	Value typeValue(&type, m_module->m_typeMgr.getStdType(StdType_BytePtr));

	Function* allocate;
	sl::BoxList<Value> allocateArgValueList;
	allocateArgValueList.insertTail(typeValue);

	Value ptrValue;

	if (type->getTypeKind() == TypeKind_Class)
	{
		if (type->getFlags() & (ClassTypeFlag_HasAbstractMethods | ClassTypeFlag_OpaqueNonCreatable))
		{
			err::setFormatStringError("cannot instantiate '%s'", type->getTypeString ().sz ());
			return false;
		}

		allocate = m_module->m_functionMgr.getStdFunction(StdFunc_AllocateClass);
	}
	else if (!rawElementCountValue)
	{
		allocate = m_module->m_functionMgr.getStdFunction(StdFunc_AllocateData);
	}
	else
	{
		allocate = m_module->m_functionMgr.getStdFunction(StdFunc_AllocateArray);

		Value countValue;
		result = castOperator(rawElementCountValue, TypeKind_SizeT, &countValue);
		if (!result)
			return false;

		allocateArgValueList.insertTail(countValue);
	}

	m_module->m_operatorMgr.callOperator(
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

bool
OperatorMgr::newOperator(
	Type* type,
	const Value& rawElementCountValue,
	sl::BoxList<Value>* argValueList,
	Value* resultValue
	)
{
	bool result;

	Value ptrValue;
	result = gcHeapAllocate(type, rawElementCountValue, &ptrValue);
	if (!result)
		return false;

	result = construct(ptrValue, argValueList);
	if (!result)
		return false;

	*resultValue = ptrValue;
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
