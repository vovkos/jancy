#include "pch.h"
#include "jnc_ReactorClassType.h"
#include "jnc_Module.h"
#include "jnc_Parser.llk.h"

namespace jnc {

//.............................................................................

ReactorClassType::ReactorClassType ()
{
	m_classTypeKind = ClassTypeKind_Reactor;
	m_bindSiteCount = 0;
	memset (m_fieldArray, 0, sizeof (m_fieldArray));
	memset (m_methodArray, 0, sizeof (m_methodArray));
}

Function*
ReactorClassType::createHandler (const rtl::Array <FunctionArg*>& argArray)
{
	FunctionType* type = m_module->m_typeMgr.getFunctionType (argArray);
	return createUnnamedMethod (StorageKind_Member, FunctionKind_Reaction, type);
}

bool
ReactorClassType::setBody (rtl::BoxList <Token>* tokenList)
{
	if (!m_body.isEmpty ())
	{
		err::setFormatStringError ("'%s' already has a body", m_tag.cc ());
		return false;
	}

	m_body.takeOver (tokenList);
	m_module->markForCompile (this);
	return true;
}

bool
ReactorClassType::calcLayout ()
{
	bool result;

	if (m_body.isEmpty ())
	{
		err::setFormatStringError ("reactor '%s' has no body", m_tag.cc ()); // thanks a lot gcc
		return false;
	}

	// scan

	Parser parser;
	parser.m_stage = Parser::StageKind_ReactorScan;
	parser.m_module = m_module;
	parser.m_reactorType = this;

	Function* start = m_methodArray [ReactorMethodKind_Start];
	Function* prevFunction = m_module->m_functionMgr.setCurrentFunction (start);

	m_module->m_namespaceMgr.openNamespace (this);

	result = parser.parseTokenList (SymbolKind_reactor_body_0, m_body, false);
	if (!result)
		return false;

	m_module->m_namespaceMgr.closeNamespace ();
	m_module->m_functionMgr.setCurrentFunction (prevFunction);

	ASSERT (parser.m_reactorBindSiteTotalCount);
	m_bindSiteCount = parser.m_reactorBindSiteTotalCount;

	Type* bindSiteType = m_module->m_typeMgr.getStdType (StdTypeKind_ReactorBindSite);
	ArrayType* bindSiteArrayType = bindSiteType->getArrayType (m_bindSiteCount);
	m_fieldArray [ReactorFieldKind_BindSiteArray] = createField ("!m_bindSiteArray", bindSiteArrayType);

	result = ClassType::calcLayout ();
	if (!result)
		return false;

	return true;
}

bool
ReactorClassType::bindHandlers (const rtl::ConstList <Reaction>& handlerList)
{
	bool result;

	StructType* bindSiteType = (StructType*) m_module->m_typeMgr.getStdType (StdTypeKind_ReactorBindSite);
	StructField* eventPtrField = *bindSiteType->getFieldList ().getHead ();
	StructField* cookieField = *bindSiteType->getFieldList ().getTail ();

	Value thisValue = m_module->m_functionMgr.getThisValue ();
	ASSERT (thisValue);

	Value bindSiteArrayValue;
	result = m_module->m_operatorMgr.getField (
		thisValue,
		m_fieldArray [ReactorFieldKind_BindSiteArray],
		NULL,
		&bindSiteArrayValue
		);

	if (!result)
		return false;

	rtl::Iterator <Reaction> handler = handlerList.getHead ();
	size_t i = 0;
	for (; handler; handler++)
	{
		Function* function = handler->m_function;

		rtl::BoxIterator <Value> value = handler->m_bindSiteList.getHead ();
		for (; value; value++, i++)
		{
			Value eventValue = *value;
			Value handlerValue = function;
			handlerValue.insertToClosureHead (thisValue);

			result = 
				m_module->m_operatorMgr.prepareOperand (&eventValue) &&
				m_module->m_operatorMgr.prepareOperand (&handlerValue);

			if (!result)
				return false;

			if (eventValue.getType ()->getTypeKind () == TypeKind_ClassRef) 
			{
				result = m_module->m_operatorMgr.unaryOperator (UnOpKind_Addr, &eventValue); // turn into a pointer
				ASSERT (result);
			}

			Value idxValue (i, TypeKind_SizeT);
			Value addMethodValue;
			Value cookieValue;
			Value bindSiteValue;
			Value dstEventValue;
			Value dstCookieValue;

			result =
				m_module->m_operatorMgr.memberOperator (eventValue, "add", &addMethodValue) &&
				m_module->m_operatorMgr.callOperator (addMethodValue, handlerValue, &cookieValue) &&
				m_module->m_operatorMgr.binaryOperator (BinOpKind_Idx, bindSiteArrayValue, idxValue, &bindSiteValue) &&
				m_module->m_operatorMgr.getStructField (bindSiteValue, eventPtrField, NULL, &dstEventValue) &&
				m_module->m_operatorMgr.getStructField (bindSiteValue, cookieField, NULL, &dstCookieValue) &&
				m_module->m_operatorMgr.storeDataRef (dstCookieValue, cookieValue);

			if (!result)
				return false;

			// force-cast event pointers (normal cast would result in dynamic cast)

			Type* eventType = m_module->m_typeMgr.getStdType (StdTypeKind_SimpleEventPtr);
			m_module->m_llvmIrBuilder.createBitCast (eventValue, eventType, &eventValue);
			m_module->m_llvmIrBuilder.createStore (eventValue, dstEventValue);
		}
	}

	ASSERT (i == m_bindSiteCount);
	return true;
}


bool
ReactorClassType::callStopMethod ()
{
	Value thisValue = m_module->m_functionMgr.getThisValue ();
	ASSERT (thisValue);

	Value stopMethodValue = m_methodArray [ReactorMethodKind_Stop];
	stopMethodValue.insertToClosureHead (thisValue);
	return m_module->m_operatorMgr.callOperator (stopMethodValue);
}

bool
ReactorClassType::compileConstructor ()
{
	ASSERT (m_constructor);

	bool result;

	size_t argCount = m_constructor->getType ()->getArgArray ().getCount ();
	ASSERT (argCount == 1 || argCount == 2);

	Value argValueArray [2];
	m_module->m_functionMgr.internalPrologue (m_constructor, argValueArray, argCount);

	if (argCount == 2)
	{
		StructField* field = m_fieldArray [ReactorFieldKind_Parent];
		ASSERT (field);

		Value parentFieldValue;
		result =
			m_module->m_operatorMgr.getClassField (argValueArray [0], field, NULL, &parentFieldValue) &&
			m_module->m_operatorMgr.storeDataRef (parentFieldValue, argValueArray [1]);

		if (!result)
			return false;
	}

	m_module->m_functionMgr.internalEpilogue ();
	return true;
}

bool
ReactorClassType::compileDestructor ()
{
	ASSERT (m_destructor);

	bool result;

	Value argValue;
	m_module->m_functionMgr.internalPrologue (m_destructor, &argValue, 1);

	result = callStopMethod ();
	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue ();
	return true;
}

bool
ReactorClassType::compileStartMethod ()
{
	bool result;

	Function* startMethod = m_methodArray [ReactorMethodKind_Start];
	Function* stopMethod = m_methodArray [ReactorMethodKind_Stop];

	result = m_module->m_functionMgr.prologue (startMethod, m_body.getHead ()->m_pos);
	if (!result)
		return false;

	Value thisValue = m_module->m_functionMgr.getThisValue ();
	ASSERT (thisValue);

	// stop

	result = callStopMethod ();
	if (!result)
		return false;

	// save arguments

	rtl::Array <FunctionArg*> argArray = startMethod->getType ()->getArgArray ();
	size_t argCount = argArray.getCount ();
	size_t i = 1;

	rtl::Iterator <StructField> argFieldIt = m_firstArgField;
	llvm::Function::arg_iterator llvmArgIt = startMethod->getLlvmFunction ()->arg_begin();
	llvmArgIt++;

	for (; i < argCount; i++, llvmArgIt++, argFieldIt++)
	{
		FunctionArg* arg = argArray [i];
		llvm::Value* llvmArg = llvmArgIt;
		StructField* argField = *argFieldIt;

		if (!arg->isNamed ())
			continue;

		Value argValue (llvmArg, arg->getType ());

		Value storeValue;
		result = m_module->m_operatorMgr.getField (thisValue, argField, NULL, &storeValue);
		if (!result)
			return false;

		m_module->m_llvmIrBuilder.createStore (argValue, storeValue);
	}

	// compile start

	Parser parser;
	parser.m_stage = Parser::StageKind_Pass2;
	parser.m_module = m_module;
	parser.m_reactorType = this;

	result = parser.parseTokenList (SymbolKind_reactor_body, m_body, true);
	if (!result)
		return false;

	// modify state

	Value stateValue;
	result =
		m_module->m_operatorMgr.getField (thisValue, m_fieldArray [ReactorFieldKind_State], NULL, &stateValue) &&
		m_module->m_operatorMgr.storeDataRef (stateValue, Value ((int64_t) 1, TypeKind_Int_p));

	if (!result)
		return false;

	// done

	result = m_module->m_functionMgr.epilogue ();
	if (!result)
		return false;

	return true;
}
bool
ReactorClassType::compileStopMethod ()
{
	bool result;

	StructType* bindSiteType = (StructType*) m_module->m_typeMgr.getStdType (StdTypeKind_ReactorBindSite);
	StructField* eventPtrField = *bindSiteType->getFieldList ().getHead ();
	StructField* cookieField = *bindSiteType->getFieldList ().getTail ();

	m_module->m_functionMgr.internalPrologue (m_methodArray [ReactorMethodKind_Stop]);

	Value thisValue = m_module->m_functionMgr.getThisValue ();
	ASSERT (thisValue);

	BasicBlock* unadviseBlock = m_module->m_controlFlowMgr.createBlock ("unadvise_block");
	BasicBlock* followBlock = m_module->m_controlFlowMgr.createBlock ("follow_block");

	Value stateValue;
	Value stateCmpValue;
	Value bindSiteArrayValue;

	result =
		m_module->m_operatorMgr.getField (thisValue, m_fieldArray [ReactorFieldKind_State], NULL, &stateValue) &&
		m_module->m_controlFlowMgr.conditionalJump (stateValue, unadviseBlock, followBlock);

	if (!result)
		return false;

	result = m_module->m_operatorMgr.getField (thisValue, m_fieldArray [ReactorFieldKind_BindSiteArray], NULL, &bindSiteArrayValue);
	if (!result)
		return false;

	for (size_t i = 0; i < m_bindSiteCount; i++)
	{
		Value idxValue (i, TypeKind_SizeT);
		Value bindSiteValue;
		Value eventValue;
		Value cookieValue;
		Value removeMethodValue;

		result =
			m_module->m_operatorMgr.binaryOperator (BinOpKind_Idx, bindSiteArrayValue, idxValue, &bindSiteValue) &&
			m_module->m_operatorMgr.getStructField (bindSiteValue, eventPtrField, NULL, &eventValue) &&
			m_module->m_operatorMgr.getStructField (bindSiteValue, cookieField, NULL, &cookieValue) &&
			m_module->m_operatorMgr.memberOperator (eventValue, "remove", &removeMethodValue) &&
			m_module->m_operatorMgr.callOperator (removeMethodValue, cookieValue);

		if (!result)
			return false;
	}

	result = m_module->m_operatorMgr.storeDataRef (stateValue, Value ((int64_t) 0, TypeKind_Int_p));
	ASSERT (result);

	m_module->m_controlFlowMgr.follow (followBlock);

	m_module->m_functionMgr.internalEpilogue ();

	return true;
}

//.............................................................................

} // namespace jnc {
