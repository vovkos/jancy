#include "pch.h"
#include "jnc_OperatorMgr.h"
#include "jnc_Module.h"
#include "jnc_Parser.llk.h"

namespace jnc {

//.............................................................................

void
OperatorMgr::zeroInitialize (const Value& value)
{
	ASSERT (value.getType ()->getTypeKindFlags () & TypeKindFlag_DataPtr);
	Type* type = ((DataPtrType*) value.getType ())->getTargetType ();

	if (type->getSize () < OperatorMgrDef_StoreSizeLimit)
	{
		m_module->m_llvmIrBuilder.createStore (type->getZeroValue (), value);
		return;
	}

	Value ptrValue;
	m_module->m_llvmIrBuilder.createBitCast (value, m_module->m_typeMgr.getStdType (StdType_BytePtr), &ptrValue);

	Value argValueArray [5] = 
	{
		ptrValue,
		Value ((int64_t) 0, m_module->m_typeMgr.getPrimitiveType (TypeKind_Int8)),
		Value (type->getSize (), m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32)),
		Value (type->getAlignment (), m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32)),
		Value ((int64_t) false, m_module->m_typeMgr.getPrimitiveType (TypeKind_Bool)),
	};

	Function* llvmMemset = m_module->m_functionMgr.getStdFunction (StdFunction_LlvmMemset);
	m_module->m_llvmIrBuilder.createCall (
		llvmMemset,
		llvmMemset->getType (),
		argValueArray,
		countof (argValueArray),
		NULL
		);
}

bool
OperatorMgr::construct (
	const Value& rawOpValue,
	rtl::BoxList <Value>* argList
	)
{
	Type* type = rawOpValue.getType ();
	TypeKind ptrTypeKind = type->getTypeKind ();

	switch (ptrTypeKind)
	{
	case TypeKind_DataRef:
	case TypeKind_DataPtr:
		type = ((DataPtrType*) type)->getTargetType ();
		break;

	case TypeKind_ClassPtr:
	case TypeKind_ClassRef:
		type = ((ClassPtrType*) type)->getTargetType ();
		break;

	default:
		err::setFormatStringError ("'%s' is not a pointer or reference", type->getTypeString ().cc ());
		return false;
	}

	TypeKind typeKind = type->getTypeKind ();

	Function* constructor = NULL;

	switch (typeKind)
	{
	case TypeKind_Struct:
	case TypeKind_Union:
		constructor = ((DerivableType*) type)->getConstructor ();
		break;

	case TypeKind_Class:
		constructor = ((ClassType*) type)->getConstructor ();
		break;
	}

	if (!constructor)
	{
		if (argList && !argList->isEmpty ())
		{
			err::setFormatStringError ("'%s' has no constructor", type->getTypeString ().cc ());
			return false;
		}

		return true;
	}

	rtl::BoxList <Value> emptyArgList;
	if (!argList)
		argList = &emptyArgList;

	Value opValue = rawOpValue;
	if (ptrTypeKind == TypeKind_DataRef || ptrTypeKind == TypeKind_ClassRef)
	{
		bool result = unaryOperator (UnOpKind_Addr, &opValue);
		if (!result)
			return false;
	}

	argList->insertHead (opValue);

	return callOperator (constructor, argList);
}

bool
OperatorMgr::parseInitializer (
	const Value& rawValue,
	Unit* unit,
	const rtl::ConstBoxList <Token>& constructorTokenList,
	const rtl::ConstBoxList <Token>& initializerTokenList
	)
{
	if (unit)
		m_module->m_unitMgr.setCurrentUnit (unit);

	bool result;

	Value value = rawValue;

	if (rawValue.getType ()->getTypeKind () == TypeKind_DataRef)
		value.overrideType (((DataPtrType*) rawValue.getType ())->getUnConstPtrType ());
	else if (rawValue.getType ()->getTypeKind () == TypeKind_ClassRef)
		value.overrideType (((ClassPtrType*) rawValue.getType ())->getUnConstPtrType ());

	rtl::BoxList <Value> argList;
	if (!constructorTokenList.isEmpty ())
	{
		Parser parser (m_module);
		parser.m_stage = Parser::StageKind_Pass2;

		m_module->m_controlFlowMgr.resetJumpFlag ();

		result = parser.parseTokenList (SymbolKind_expression_or_empty_list_save_list, constructorTokenList);
		if (!result)
			return false;

		argList.takeOver (&parser.m_expressionValueList);
	}

	result = construct (value, &argList);
	if (!result)
		return false;

	if (!initializerTokenList.isEmpty ())
	{
		Parser parser (m_module);
		parser.m_stage = Parser::StageKind_Pass2;

		m_module->m_controlFlowMgr.resetJumpFlag ();

		if (initializerTokenList.getHead ()->m_token == '{')
		{
			parser.m_curlyInitializerTargetValue = value;
			result = parser.parseTokenList (SymbolKind_curly_initializer, initializerTokenList);
			if (!result)
				return false;
		}
		else
		{
			result =
				parser.parseTokenList (SymbolKind_expression_save_value, initializerTokenList) &&
				m_module->m_operatorMgr.binaryOperator (BinOpKind_Assign, value, parser.m_expressionValue);

			if (!result)
				return false;
		}
	}

	nullifyTmpStackGcRootList ();
	return true;
}

bool
OperatorMgr::parseExpression (
	Unit* unit,
	const rtl::ConstBoxList <Token>& expressionTokenList,
	uint_t flags,
	Value* resultValue
	)
{
	if (unit)
		m_module->m_unitMgr.setCurrentUnit (unit);

	Parser parser (m_module);
	parser.m_stage = Parser::StageKind_Pass2;
	parser.m_flags |= flags;

	m_module->m_controlFlowMgr.resetJumpFlag ();

	bool result = parser.parseTokenList (SymbolKind_expression_save_value, expressionTokenList);
	if (!result)
		return false;

	*resultValue = parser.m_expressionValue;
	return true;
}

bool
OperatorMgr::parseConstExpression (
	Unit* unit,
	const rtl::ConstBoxList <Token>& expressionTokenList,
	Value* resultValue
	)
{
	bool result = parseExpression (unit, expressionTokenList, Parser::Flag_ConstExpression, resultValue);
	if (!result)
		return false;

	ASSERT (resultValue->getValueKind () == ValueKind_Const);
	return true;
}

bool
OperatorMgr::parseConstIntegerExpression (
	Unit* unit,
	const rtl::ConstBoxList <Token>& expressionTokenList,
	intptr_t* integer
	)
{
	Value value;
	bool result = parseConstExpression (unit, expressionTokenList, &value);
	if (!result)
		return false;

	if (!(value.getType ()->getTypeKindFlags () & TypeKindFlag_Integer))
	{
		err::setFormatStringError ("expression is not integer constant");
		return false;
	}

	*integer = 0;
	memcpy (integer, value.getConstData (), value.getType ()->getSize ());
	return true;
}

bool
OperatorMgr::parseAutoSizeArrayInitializer (
	const rtl::ConstBoxList <Token>& initializerTokenList,
	size_t* elementCount
	)
{
	int firstToken = initializerTokenList.getHead ()->m_token;
	switch (firstToken)
	{
	case TokenKind_Literal:
	case TokenKind_HexLiteral:
		*elementCount = parseAutoSizeArrayLiteralInitializer (initializerTokenList);
		break;

	case '{':
		*elementCount = parseAutoSizeArrayCurlyInitializer (initializerTokenList);
		break;

	default:
		err::setFormatStringError ("invalid initializer for auto-size-array");
		return false;
	}

	return true;
}

// it's both more efficient AND easier to parse these by hand

size_t
OperatorMgr::parseAutoSizeArrayLiteralInitializer (const rtl::ConstBoxList <Token>& initializerTokenList)
{
	size_t elementCount = 0;

	rtl::BoxIterator <Token> token = initializerTokenList.getHead ();
	for (; token; token++)
	{
		switch (token->m_token)
		{
		case TokenKind_Literal:
			elementCount += token->m_data.m_string.getLength ();
			break;

		case TokenKind_HexLiteral:
			elementCount += token->m_data.m_binData.getCount ();
			break;
		}
	}

	if (initializerTokenList.getTail ()->m_token == TokenKind_Literal)
		elementCount++;

	return elementCount;
}

size_t
OperatorMgr::parseAutoSizeArrayCurlyInitializer (const rtl::ConstBoxList <Token>& initializerTokenList)
{
	intptr_t level = 0;
	size_t elementCount = 0;

	bool isElement = false;

	rtl::BoxIterator <Token> token = initializerTokenList.getHead ();
	for (; token; token++)
	{
		switch (token->m_token)
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
			if (level == 1)
			{
				elementCount++;
				isElement = false;
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
OperatorMgr::evaluateAlias (
	ModuleItemDecl* decl,
	const Value& thisValue,
	const rtl::ConstBoxList <Token> tokenList,
	Value* resultValue
	)
{
	Value prevThisValue = m_module->m_functionMgr.overrideThisValue (thisValue);
	bool result = evaluateAlias (decl, tokenList, resultValue);
	if (!result)
		return false;

	m_module->m_functionMgr.overrideThisValue (prevThisValue);
	return true;
}

bool
OperatorMgr::evaluateAlias (
	ModuleItemDecl* decl,
	const rtl::ConstBoxList <Token> tokenList,
	Value* resultValue
	)
{
	Parser parser (m_module);
	parser.m_stage = Parser::StageKind_Pass2;

	m_module->m_namespaceMgr.openNamespace (decl->getParentNamespace ());
	m_module->m_namespaceMgr.lockSourcePos ();

	bool result = parser.parseTokenList (SymbolKind_expression_save_value, tokenList);
	if (!result)
		return false;

	m_module->m_namespaceMgr.unlockSourcePos ();
	m_module->m_namespaceMgr.closeNamespace ();

	*resultValue = parser.m_expressionValue;
	return true;
}

bool
OperatorMgr::newOperator (
	Type* type,
	const Value& rawElementCountValue,
	rtl::BoxList <Value>* argList,
	Value* resultValue
	)
{
	bool result;

	Value typeValue (&type, m_module->m_typeMgr.getStdType (StdType_BytePtr));

	if (isOpaqueClassType (type))
	{
		Function* operatorNew = ((ClassType*) type)->getOperatorNew ();
		if (!operatorNew)
		{
			err::setFormatStringError ("opaque '%s' has no operator new", type->getTypeString ().cc ());
			return false;
		}

		argList->insertHead (typeValue);
		return callOperator (operatorNew, argList, resultValue);
	}

	Function* allocate;

	Value argValueArray [2] =
	{
		typeValue
	};
	
	size_t argCount = 1;

	if (rawElementCountValue)
	{
		allocate = m_module->m_functionMgr.getStdFunction (StdFunction_AllocateArray);

		result = castOperator (rawElementCountValue, TypeKind_SizeT, &argValueArray [1]);
		if (!result)
			return false;

		argCount = 2;
	}
	else if (type->getTypeKind () == TypeKind_Class)
	{
		allocate = m_module->m_functionMgr.getStdFunction (StdFunction_AllocateClass);
	}
	else
	{
		allocate = m_module->m_functionMgr.getStdFunction (StdFunction_AllocateData);
	}

	Value ptrValue;
	m_module->m_llvmIrBuilder.createCall (
		allocate,
		allocate->getType (),
		argValueArray,
		argCount,
		&ptrValue
		);

	markStackGcRoot (ptrValue, type->getDataPtrType_c (), true);

	result = construct (ptrValue, argList);
	if (!result)
		return false;

	*resultValue = ptrValue;
	return true;
}

void
OperatorMgr::nullifyGcRootList (const rtl::ConstBoxList <Value>& list)
{
	if (list.isEmpty ())
		return;

	Value nullValue = m_module->m_typeMgr.getStdType (StdType_BytePtr)->getZeroValue ();

	rtl::BoxIterator <Value> it = list.getTail ();
	for (; it; it--)
	{
		Value value = *it;
		ASSERT (value.getType ()->getTypeKind () == TypeKind_DataPtr);

		m_module->m_llvmIrBuilder.createStore (nullValue, value);
	}
}

void
OperatorMgr::createTmpStackGcRoot (const Value& value)
{
	Type* type = value.getType ();
	ASSERT (
		(type->getFlags () & TypeFlag_GcRoot) ||
		type->getTypeKind () == TypeKind_DataPtr // heap new creates tmp stack root
		);

	Value ptrValue;
	m_module->m_llvmIrBuilder.createAlloca (type, "tmpGcRoot", NULL, &ptrValue);
	m_module->m_llvmIrBuilder.createStore (value, ptrValue);
	markStackGcRoot (ptrValue, type, true);
}

void
OperatorMgr::nullifyTmpStackGcRootList ()
{
	if (m_module->m_controlFlowMgr.getCurrentBlock ()->getFlags () & BasicBlockFlag_Reachable)
		nullifyGcRootList (m_tmpStackGcRootList);

	m_tmpStackGcRootList.clear ();
}

void
OperatorMgr::markStackGcRoot (
	const Value& ptrValue,
	Type* type,
	bool isTmpGcRoot
	)
{
	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	function->markGc ();
/*
	BasicBlock* prevBlock = m_module->m_controlFlowMgr.setCurrentBlock (function->getEntryBlock ());

	Type* bytePtrType = m_module->m_typeMgr.getStdType (StdType_BytePtr);

	Value gcRootValue;
	m_module->m_llvmIrBuilder.createAlloca (
		bytePtrType,
		"gcRoot",
		bytePtrType->getDataPtrType_c (),
		&gcRootValue
		);

	if (isTmpGcRoot)
	{
		m_tmpStackGcRootList.insertTail (gcRootValue);
	}
	else
	{
		Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
		ASSERT (scope);
		scope->addToGcRootList (gcRootValue);
	}

	Function* markGcRoot = m_module->m_functionMgr.getStdFunction (StdFunction_MarkGcRoot);
	ASSERT (markGcRoot);

	Value argValueArray [2];
	argValueArray [0] = gcRootValue;
	argValueArray [1].createConst (&type, m_module->m_typeMgr.getStdType (StdType_BytePtr));

	Value resultValue;
	m_module->m_llvmIrBuilder.createCall (
		markGcRoot,
		markGcRoot->getType (),
		argValueArray, 2,
		&resultValue
		);

	m_module->m_controlFlowMgr.setCurrentBlock (prevBlock);

	Value bytePtrValue;
	m_module->m_llvmIrBuilder.createBitCast (ptrValue, bytePtrType, &bytePtrValue);
	m_module->m_llvmIrBuilder.createStore (bytePtrValue, gcRootValue); */
}

//.............................................................................

} // namespace jnc {

/*

bool
OperatorMgr::allocate (
	StorageKind storageKind,
	Type* type,
	const Value& elementCountValue,
	const char* tag,
	Value* resultValue
	)
{
	ASSERT (!elementCountValue || elementCountValue.getType ()->getTypeKind () == TypeKind_SizeT);

	Type* ptrType = type->getDataPtrType_c ();

	bool isNonConstSizeArray = elementCountValue && elementCountValue.getValueKind () != ValueKind_Const;
	if (isNonConstSizeArray)
	{
		if (storageKind != StorageKind_Heap)
		{
			err::setFormatStringError ("cannot create non-const-sized arrays with '%s new'", getStorageKindString (storageKind));
			return false;
		}
	}
	else if (elementCountValue.getValueKind () == ValueKind_Const)
	{
		size_t count = elementCountValue.getSizeT ();
		if (count > 1)
			type = m_module->m_typeMgr.getArrayType (type, count);
	}

	Variable* variable;
	Function* function;
	BasicBlock* prevBlock;

	Value typeValue;
	Value ptrValue;

	switch (storageKind)
	{
	case StorageKind_Static:
		variable = m_module->m_variableMgr.createVariable (StorageKind_Static, tag, tag, type);

		function = m_module->getConstructor ();
		prevBlock = m_module->m_controlFlowMgr.setCurrentBlock (function->getEntryBlock ());
		m_module->m_variableMgr.allocatePrimeStaticVariable (variable);
		m_module->m_controlFlowMgr.setCurrentBlock (prevBlock);
		resultValue->setVariable (variable);
		return true;

	case StorageKind_Thread:
		variable = m_module->m_variableMgr.createVariable (StorageKind_Thread, tag, tag, type);
		resultValue->setVariable (variable);
		return true;

	case StorageKind_Stack:
		m_module->m_llvmIrBuilder.createAlloca (type, tag, ptrType, &ptrValue);

		if (type->getFlags () & TypeFlag_GcRoot)
			markStackGcRoot (ptrValue, type);
		break;

	case StorageKind_Heap:
		function = m_module->m_functionMgr.getStdFunction (StdFunction_AllocateClass);

		typeValue.createConst (&type, m_module->m_typeMgr.getStdType (StdType_BytePtr));

		m_module->m_llvmIrBuilder.createCall2 (
			function,
			function->getType (),
			typeValue,
			isNonConstSizeArray ?
				elementCountValue :
				Value (1, m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT)),
			&ptrValue
			);

		break;

	default:
		err::setFormatStringError ("invalid storage specifier '%s' in 'new' operator", getStorageKindString (storageKind));
		return false;
	}

	m_module->m_llvmIrBuilder.createBitCast (ptrValue, ptrType, resultValue);
	return true;
}

bool
OperatorMgr::prime (
	StorageKind storageKind,
	const Value& ptrValue,
	Type* type,
	const Value& elementCountValue,
	Value* resultValue
	)
{
	ASSERT (type->getTypeKind () != TypeKind_Class);

	zeroInitialize (ptrValue, type);

	Value objHdrValue;
	switch (storageKind)
	{
	case StorageKind_Static:
	case StorageKind_Heap:
		m_module->m_llvmIrBuilder.createBitCast (ptrValue, m_module->m_typeMgr.getStdType (StdType_BoxPtr), &objHdrValue);
		m_module->m_llvmIrBuilder.createGep (objHdrValue, -1, objHdrValue.getType (), &objHdrValue);
		break;

	case StorageKind_Thread:
	case StorageKind_Stack:
		#pragma AXL_TODO ("cleanup the whole new/allocate/prime/initialize mess")
		ASSERT (false);

	default:
		err::setFormatStringError ("cannot prime '%s' value", getStorageKindString (storageKind));
		return false;
	}

	Value sizeValue (type->getSize (), m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT));
	if (elementCountValue)
	{
		bool result = binaryOperator (BinOpKind_Mul, &sizeValue, elementCountValue);
		if (!result)
			return false;
	}

	resultValue->setLeanDataPtr (
		ptrValue.getLlvmValue (),
		type->getDataPtrType (DataPtrTypeKind_Lean),
		objHdrValue,
		ptrValue,
		sizeValue
		);

	return true;
}

 */
