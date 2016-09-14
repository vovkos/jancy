#include "pch.h"
#include "jnc_ct_FunctionType.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

FunctionType::FunctionType ()
{
	m_typeKind = TypeKind_Function;
	m_callConv = NULL;
	m_returnType = NULL;
	m_shortType = this;
	m_stdObjectMemberMethodType = NULL;
	m_functionPtrTypeTuple = NULL;
	m_reactorIfaceType = NULL;
}

DerivableType*
FunctionType::getThisTargetType ()
{
	Type* thisArgType = getThisArgType ();
	if (!thisArgType)
		return NULL;

	TypeKind thisArgTypeKind = thisArgType->getTypeKind ();
	switch (thisArgTypeKind)
	{
	case TypeKind_ClassPtr:
		return ((ClassPtrType*) thisArgType)->getTargetType ();

	case TypeKind_DataPtr:
		return (DerivableType*) ((DataPtrType*) thisArgType)->getTargetType ();

	default:
		ASSERT (false);
		return NULL;
	}
}

sl::String
FunctionType::getArgSignature ()
{
	if (m_argSignature.isEmpty ())
		m_argSignature = createArgSignature ();

	return m_argSignature;
}

FunctionPtrType*
FunctionType::getFunctionPtrType (
	TypeKind typeKind,
	FunctionPtrTypeKind ptrTypeKind,
	uint_t flags
	)
{
	return m_module->m_typeMgr.getFunctionPtrType (this, typeKind, ptrTypeKind, flags);
}

ClassType*
FunctionType::getMulticastType ()
{
	return m_module->m_typeMgr.getMulticastType (this);
}

FunctionType*
FunctionType::getMemberMethodType (
	DerivableType* parentType,
	uint_t thisArgTypeFlags
	)
{
	return m_module->m_typeMgr.getMemberMethodType (parentType, this, thisArgTypeFlags);
}

FunctionType*
FunctionType::getStdObjectMemberMethodType ()
{
	return m_module->m_typeMgr.getStdObjectMemberMethodType (this);
}

Function*
FunctionType::getAbstractFunction ()
{
	if (m_abstractFunction)
		return m_abstractFunction;

	Function* function = m_module->m_functionMgr.createFunction (FunctionKind_Internal, "abstractFunction", this);
	m_abstractFunction = function;
	m_module->markForCompile (this);
	return function;
}

bool
FunctionType::compile ()
{
	ASSERT (m_abstractFunction);

	m_module->m_functionMgr.internalPrologue (m_abstractFunction);
//	m_module->m_llvmIrBuilder.runtimeError (RuntimeErrorKind_AbstractFunction);
	m_module->m_functionMgr.internalEpilogue ();

	return true;
}

sl::String
FunctionType::createArgSignature (
	Type* const* argTypeArray,
	size_t argCount,
	uint_t flags
	)
{
	sl::String string = "(";

	for (size_t i = 0; i < argCount; i++)
	{
		Type* type = argTypeArray [i];
		string += type->getSignature ();
		string += ",";
	}

	string += (flags & FunctionTypeFlag_VarArg) ? ".)" : ")";
	return string;
}

sl::String
FunctionType::createArgSignature (
	FunctionArg* const* argArray,
	size_t argCount,
	uint_t flags
	)
{
	sl::String string = "(";

	for (size_t i = 0; i < argCount; i++)
	{
		FunctionArg* arg = argArray [i];

		string += arg->getType ()->getSignature ();
		string += ",";
	}

	string += (flags & FunctionTypeFlag_VarArg) ? ".)" : ")";
	return string;
}

sl::String
FunctionType::createFlagSignature (uint_t flags)
{
	sl::String string;

	if (flags & FunctionTypeFlag_Automaton)
		string += 'A';

	if (flags & FunctionTypeFlag_Unsafe)
		string += 'U';

	return string;
}

sl::String
FunctionType::createSignature (
	CallConv* callConv,
	Type* returnType,
	Type* const* argTypeArray,
	size_t argCount,
	uint_t flags
	)
{
	sl::String string = 'F';
	string += createFlagSignature (flags);
	string += getCallConvSignature (callConv->getCallConvKind ());
	string += returnType->getSignature ();
	string += createArgSignature (argTypeArray, argCount, flags);
	return string;
}

sl::String
FunctionType::createSignature (
	CallConv* callConv,
	Type* returnType,
	FunctionArg* const* argArray,
	size_t argCount,
	uint_t flags
	)
{
	sl::String string = 'F';
	string += createFlagSignature (flags);
	string += getCallConvSignature (callConv->getCallConvKind ());
	string += returnType->getSignature ();
	string += createArgSignature (argArray, argCount, flags);
	return string;
}

sl::String
FunctionType::getArgString ()
{
	if (!m_argString.isEmpty ())
		return m_argString;

	bool isUserType = (m_flags & ModuleItemFlag_User) != 0;

	m_argString = "(";

	if (!m_argArray.isEmpty ())
	{
		FunctionArg* arg = m_argArray [0];
		m_argString += arg->getType ()->getTypeString ();

		if (arg->getStorageKind () == StorageKind_This)
		{
			m_argString += " this";
		}
		else if (isUserType)
		{
				if (!arg->getName ().isEmpty ())
					m_argString.appendFormat (" %s", arg->getName ().cc ());

				if (!arg->getInitializer ().isEmpty ())
					m_argString.appendFormat (" = %s", arg->getInitializerString ().cc ());
		}

		size_t argCount = m_argArray.getCount ();
		for (size_t i = 1; i < argCount; i++)
		{
			arg = m_argArray [i];

			m_argString.appendFormat (", %s", arg->getType ()->getTypeString ().cc ());

			if (isUserType)
			{
				if (!arg->getName ().isEmpty ())
					m_argString.appendFormat (" %s", arg->getName ().cc ());

				if (!arg->getInitializer ().isEmpty ())
					m_argString.appendFormat (" = %s", arg->getInitializerString ().cc ());
			}
		}

		if (m_flags & FunctionTypeFlag_VarArg)
			m_argString += ", ";
	}

	if (!(m_flags & FunctionTypeFlag_VarArg))
		m_argString += ")";
	else
		m_argString += "...)";

	return m_argString;
}

sl::String
FunctionType::createArgDoxyLinkedText ()
{
	bool isUserType = (m_flags & ModuleItemFlag_User) != 0;

	sl::String argString = "(";

	if (!m_argArray.isEmpty ())
	{
		FunctionArg* arg = m_argArray [0];
		argString += arg->getType ()->getDoxyBlock ()->getLinkedText ();

		if (arg->getStorageKind () == StorageKind_This)
		{
			argString += " this";
		}
		else if (isUserType)
		{
				if (!arg->getName ().isEmpty ())
					argString.appendFormat (" %s", arg->getName ().cc ());

				if (!arg->getInitializer ().isEmpty ())
					argString.appendFormat (" = %s", arg->getInitializerString ().cc ());
		}

		size_t argCount = m_argArray.getCount ();
		for (size_t i = 1; i < argCount; i++)
		{
			arg = m_argArray [i];

			argString.appendFormat (", %s", arg->getType ()->getDoxyBlock ()->getLinkedText ().cc ());

			if (isUserType)
			{
				if (!arg->getName ().isEmpty ())
					argString.appendFormat (" %s", arg->getName ().cc ());

				if (!arg->getInitializer ().isEmpty ())
					argString.appendFormat (" = %s", arg->getInitializerString ().cc ());
			}
		}

		if (m_flags & FunctionTypeFlag_VarArg)
			argString += ", ";
	}

	if (!(m_flags & FunctionTypeFlag_VarArg))
		argString += ")";
	else
		argString += "...)";

	return argString;
}

sl::String
FunctionType::getTypeModifierString ()
{
	if (!m_typeModifierString.isEmpty ())
		return m_typeModifierString;

	if (m_flags & FunctionTypeFlag_ErrorCode)
		m_argString += "errorcode ";

	if (!m_callConv->isDefault ())
	{
		m_typeModifierString = m_callConv->getCallConvDisplayString ();
		m_typeModifierString += ' ';
	}

	if (m_flags & FunctionTypeFlag_Unsafe)
		m_typeModifierString += "unsafe ";

	if (m_flags & FunctionTypeFlag_Automaton)
		m_typeModifierString += "automaton ";

	return m_typeModifierString;
}

void
FunctionType::prepareTypeString ()
{
	m_typeString = m_returnType->getTypeString ();
	m_typeString += ' ';
	m_typeString += getTypeModifierString ();
	m_typeString += getArgString ();
}

sl::String
FunctionType::createDoxyLinkedText ()
{
	sl::String string = m_returnType->getDoxyBlock ()->getLinkedText ();
	string += ' ';
	string += getTypeModifierString ();
	string += createArgDoxyLinkedText ();

	return string;
}

sl::String
FunctionType::createDeclarationString (const char* name)
{
	sl::String string = m_returnType->getTypeString ();
	string += ' ';
	string += getTypeModifierString ();
	string += name;
	string += ' ';
	string += getArgString ();
	return string;
}

void
FunctionType::prepareLlvmType ()
{
	m_callConv->prepareFunctionType (this);
	ASSERT (m_llvmType);
}

void
FunctionType::prepareLlvmDiType ()
{
	m_llvmDiType = m_module->m_llvmDiBuilder.createSubroutineType (this);
}

bool
FunctionType::calcLayout ()
{
	bool result;

	result = m_returnType->ensureLayout ();
	if (!result)
		return false;

	size_t argCount = m_argArray.getCount ();
	for (size_t i = 0; i < argCount; i++)
	{
		result = m_argArray [i]->getType ()->ensureLayout ();
		if (!result)
			return false;
	}

	if (m_shortType != this)
	{
		result = m_shortType->ensureLayout ();
		if (!result)
			return false;
	}

	// update signature

	sl::String signature = createSignature (
		m_callConv,
		m_returnType,
		m_argArray,
		m_argArray.getCount (),
		m_flags
		);

	m_module->m_typeMgr.updateTypeSignature (this, signature);
	return true;
}

void
FunctionType::generateArgDocumentation (sl::String* itemXml)
{
	size_t count = m_argArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		FunctionArg* arg = m_argArray [i];
		if (arg->getStorageKind () == StorageKind_This)
			continue;

		itemXml->appendFormat (
			"<param>\n"
			"    <type>%s</type>\n"
			"    <declname>%s</declname>\n"
			"</param>\n",
			arg->getType ()->getDoxyBlock ()->getLinkedText ().cc (),
			arg->getName ().cc ()
			);
	}
}

//.............................................................................

} // namespace ct
} // namespace jnc
