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
#include "jnc_ct_ReactorClassType.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_Parser.llk.h"

namespace jnc {
namespace ct {

//..............................................................................

ReactorClassType::ReactorClassType ()
{
	m_classTypeKind = ClassTypeKind_Reactor;
	m_reactionCount = 0;
	m_bindSiteCount = 0;
	memset (m_fieldArray, 0, sizeof (m_fieldArray));
	memset (m_methodArray, 0, sizeof (m_methodArray));
}

void
ReactorClassType::prepareTypeString ()
{
	TypeStringTuple* tuple = getTypeStringTuple ();
	tuple->m_typeStringPrefix.format ("reactor %s", m_qualifiedName.sz ());
	tuple->m_typeStringSuffix = m_methodArray [ReactorMethodKind_Start]->getType ()->getShortType ()->getTypeStringSuffix ();
}

void
ReactorClassType::prepareDoxyLinkedText ()
{
	TypeStringTuple* tuple = getTypeStringTuple ();
	tuple->m_doxyLinkedTextPrefix.format ("reactor %s", m_qualifiedName.sz ());
	tuple->m_doxyLinkedTextSuffix = m_methodArray [ReactorMethodKind_Start]->getType ()->getShortType ()->getDoxyLinkedTextSuffix ();
}

bool
ReactorClassType::setBody (sl::BoxList <Token>* tokenList)
{
	if (!m_body.isEmpty ())
	{
		err::setFormatStringError ("'%s' already has a body", m_tag.sz ());
		return false;
	}

	sl::takeOver (&m_body, tokenList);
	m_module->markForCompile (this);
	return true;
}

bool
ReactorClassType::calcLayout ()
{
	bool result;

	if (m_body.isEmpty ())
	{
		err::setFormatStringError ("reactor '%s' has no body", m_tag.sz ());
		return false;
	}

	// scan

	Parser parser (m_module);
	parser.m_stage = Parser::Stage_ReactorScan;
	parser.m_reactorType = this;

	Function* start = m_methodArray [ReactorMethodKind_Start];
	Function* prevFunction = m_module->m_functionMgr.setCurrentFunction (start);

	m_module->m_namespaceMgr.openNamespace (this);

	result = parser.parseTokenList (SymbolKind_reactor_body_0, m_body, false);
	if (!result)
		return false;

	m_module->m_namespaceMgr.closeNamespace ();
	m_module->m_functionMgr.setCurrentFunction (prevFunction);

	if (!parser.m_reactionCount)
	{
		err::setFormatStringError ("reactor '%s' has no reactions", m_tag.sz ());
		return false;
	}

	if (!parser.m_reactorTotalBindSiteCount)
	{
		err::setFormatStringError ("reactor '%s' has no bindings", m_tag.sz ());
		return false;
	}

	m_reactionCount = parser.m_reactionCount;
	m_bindSiteCount = parser.m_reactorTotalBindSiteCount;

	Type* bindSiteType = m_module->m_typeMgr.getStdType (StdType_ReactorBindSite);
	ArrayType* arrayType = bindSiteType->getArrayType (m_bindSiteCount);
	m_fieldArray [ReactorFieldKind_BindSiteArray] = createField ("!m_bindSiteArray", arrayType);

	arrayType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32)->getArrayType (m_reactionCount);
	m_fieldArray [ReactorFieldKind_ReactionStateArray] = createField ("!m_reactionStateArray", arrayType);

	result = ClassType::calcLayout ();
	if (!result)
		return false;

	return true;
}

bool
ReactorClassType::subscribe (const sl::ConstList <Reaction>& reactionList)
{
	bool result;

	StructType* bindSiteType = (StructType*) m_module->m_typeMgr.getStdType (StdType_ReactorBindSite);

	sl::Array <StructField*> fieldArray = bindSiteType->getMemberFieldArray ();
	StructField* eventPtrField = fieldArray [0];
	StructField* cookieField = fieldArray.getBack ();

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

	sl::ConstIterator <Reaction> reaction = reactionList.getHead ();
	size_t reactionIdx = 0;
	size_t bindSiteIdx = 0;
	for (; reaction; reaction++, reactionIdx++)
	{
		Function* function = reaction->m_function;
		function->m_reactionIndex = reactionIdx;

		sl::ConstBoxIterator <Value> value = reaction->m_bindSiteList.getHead ();
		for (; value; value++, bindSiteIdx++)
		{
			Value eventValue = *value;
			Value handlerValue = function;

			Closure* closure = handlerValue.createClosure ();
			closure->insertThisArgValue (thisValue);

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

			Value idxValue (bindSiteIdx, m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT));
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

			// bit-cast event pointers is enough

			m_module->m_llvmIrBuilder.createBitCast (eventValue, eventPtrField->getType (), &eventValue);
			m_module->m_llvmIrBuilder.createStore (eventValue, dstEventValue);
		}
	}

	ASSERT (reactionIdx == m_reactionCount);
	ASSERT (bindSiteIdx <= m_bindSiteCount);
	return true;
}

bool
ReactorClassType::callStopMethod ()
{
	Value thisValue = m_module->m_functionMgr.getThisValue ();
	ASSERT (thisValue);

	Value stopMethodValue = m_methodArray [ReactorMethodKind_Stop];
	Closure* closure = stopMethodValue.createClosure ();
	closure->insertThisArgValue (thisValue);
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

	m_module->m_functionMgr.prologue (startMethod, m_body.getHead ()->m_pos);

	Value thisValue = m_module->m_functionMgr.getThisValue ();
	ASSERT (thisValue);

	// stop

	result = callStopMethod ();
	if (!result)
		return false;

	// save arguments

	sl::Array <FunctionArg*> argArray = startMethod->getType ()->getArgArray ();
	size_t argCount = argArray.getCount ();
	size_t i = 1;

	sl::Iterator <StructField> argFieldIt = m_firstArgField;
	llvm::Function::arg_iterator llvmArgIt = startMethod->getLlvmFunction ()->arg_begin();
	llvmArgIt++;

	for (; i < argCount; i++, llvmArgIt++, argFieldIt++)
	{
		FunctionArg* arg = argArray [i];
		llvm::Value* llvmArg = &*llvmArgIt;
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

	Parser parser (m_module);
	parser.m_stage = Parser::Stage_ReactorStarter;
	parser.m_reactorType = this;

	result = parser.parseTokenList (SymbolKind_reactor_body, m_body, true);
	if (!result)
		return false;

	// modify state

	Value stateValue;
	result =
		m_module->m_operatorMgr.getField (thisValue, m_fieldArray [ReactorFieldKind_State], NULL, &stateValue) &&
		m_module->m_operatorMgr.storeDataRef (
			stateValue,
			Value ((int64_t) 1, m_module->m_typeMgr.getPrimitiveType (TypeKind_IntPtr))
			);

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

	StructType* bindSiteType = (StructType*) m_module->m_typeMgr.getStdType (StdType_ReactorBindSite);

	sl::Array <StructField*> fieldArray = bindSiteType->getMemberFieldArray ();
	StructField* eventPtrField = fieldArray [0];
	StructField* cookieField = fieldArray.getBack ();

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
		Value idxValue (i, m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT));
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

	result = m_module->m_operatorMgr.storeDataRef (
		stateValue,
		Value ((int64_t) 0, m_module->m_typeMgr.getPrimitiveType (TypeKind_IntPtr))
		);

	ASSERT (result);

	m_module->m_controlFlowMgr.follow (followBlock);

	m_module->m_functionMgr.internalEpilogue ();

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
