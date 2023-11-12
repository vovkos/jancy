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
#include "jnc_ct_FunctionType.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

sl::StringRef
getFunctionTypeFlagString(uint_t flags) {
	flags &= (FunctionTypeFlag__User);
	if (!flags)
		return sl::StringRef();

	FunctionTypeFlag flag = getFirstFlag<FunctionTypeFlag>(flags);
	sl::StringRef string0 = jnc::getFunctionTypeFlagString(flag);
	flags &= ~flag;
	if (!flags)
		return string0;

	sl::String string = string0;
	while (flags) {
		flag = getFirstFlag<FunctionTypeFlag>(flags);
		string += ' ';
		string += jnc::getFunctionTypeFlagString(flag);
		flags &= ~flag;
	}

	return string;
}

//..............................................................................

FunctionType::FunctionType() {
	m_typeKind = TypeKind_Function;
	m_callConv = NULL;
	m_returnType = NULL;
	m_asyncReturnType = NULL;
	m_shortType = this;
	m_stdObjectMemberMethodType = NULL;
	m_functionPtrTypeTuple = NULL;
	m_functionPtrTypeTuple = NULL;
}

DerivableType*
FunctionType::getThisTargetType() {
	Type* thisArgType = getThisArgType();
	if (!thisArgType)
		return NULL;

	TypeKind thisArgTypeKind = thisArgType->getTypeKind();
	switch (thisArgTypeKind) {
	case TypeKind_ClassPtr:
		return ((ClassPtrType*)thisArgType)->getTargetType();

	case TypeKind_DataPtr:
		return (DerivableType*)((DataPtrType*)thisArgType)->getTargetType();

	default:
		ASSERT(false);
		return NULL;
	}
}

FunctionPtrType*
FunctionType::getFunctionPtrType(
	TypeKind typeKind,
	FunctionPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	return m_module->m_typeMgr.getFunctionPtrType(this, typeKind, ptrTypeKind, flags);
}

ClassType*
FunctionType::getMulticastType() {
	return m_module->m_typeMgr.getMulticastType(this);
}

FunctionType*
FunctionType::getMemberMethodType(
	DerivableType* parentType,
	uint_t thisArgTypeFlags
) {
	return m_module->m_typeMgr.getMemberMethodType(parentType, this, thisArgTypeFlags);
}

FunctionType*
FunctionType::getStdObjectMemberMethodType() {
	return m_module->m_typeMgr.getStdObjectMemberMethodType(this);
}

void
FunctionType::appendFlagSignature(
	sl::String* string,
	uint_t flags
) {
	if (flags & FunctionTypeFlag_Unsafe)
		*string += 'u';

	if (flags & FunctionTypeFlag_Async)
		*string += 'a';

	if (flags & (FunctionTypeFlag_ErrorCode | FunctionTypeFlag_AsyncErrorCode))
		*string += 'e';
}

uint_t
FunctionType::appendArgSignature(
	sl::String* string,
	Type* const* argTypeArray,
	size_t argCount,
	uint_t typeFlags
) {
	uint_t signatureFlags = TypeFlag_SignatureFinal;
	*string += '(';

	for (size_t i = 0; i < argCount; i++) {
		Type* type = argTypeArray[i];
		*string += type->getSignature();
		*string += ',';
		signatureFlags &= type->getFlags() & TypeFlag_SignatureFinal;
	}

	if (typeFlags & FunctionTypeFlag_VarArg)
		*string += '.';

	*string += ')';
	return signatureFlags;
}

uint_t
FunctionType::appendArgSignature(
	sl::String* string,
	FunctionArg* const* argArray,
	size_t argCount,
	uint_t typeFlags
) {
	uint_t signatureFlags = TypeFlag_SignatureFinal;
	*string += '(';

	for (size_t i = 0; i < argCount; i++) {
		Type* type = argArray[i]->getType();
		*string += type->getSignature();
		*string += ',';
		signatureFlags &= type->getFlags() & TypeFlag_SignatureFinal;
	}

	if (typeFlags & FunctionTypeFlag_VarArg)
		*string += '.';

	*string += ')';
	return signatureFlags;
}

uint_t
FunctionType::createSignature(
	sl::String* string,
	sl::StringRef* argSignature,
	CallConv* callConv,
	Type* returnType,
	Type* const* argTypeArray,
	size_t argCount,
	uint_t flags
) {
	*string = 'F';
	appendFlagSignature(string, flags);
	*string += getCallConvSignature(callConv->getCallConvKind());
	*string += returnType->getSignature();

	size_t length = string->getLength();
	uint_t signatureFlags = appendArgSignature(string, argTypeArray, argCount, flags);
	*argSignature = string->getSubString(length);
	return signatureFlags;
}

uint_t
FunctionType::createSignature(
	sl::String* string,
	sl::StringRef* argSignature,
	CallConv* callConv,
	Type* returnType,
	FunctionArg* const* argArray,
	size_t argCount,
	uint_t flags
) {
	*string = 'F';
	appendFlagSignature(string, flags);
	*string += getCallConvSignature(callConv->getCallConvKind());
	*string += returnType->getSignature();

	size_t length = string->getLength();
	uint_t signatureFlags = appendArgSignature(string, argArray, argCount, flags);
	*argSignature = string->getSubString(length);
	return signatureFlags;
}

sl::StringRef
FunctionType::getTypeModifierString() {
	sl::String string;

	if (m_flags & (FunctionTypeFlag_ErrorCode | FunctionTypeFlag_AsyncErrorCode))
		string += "errorcode ";

	if (m_flags & FunctionTypeFlag_Async)
		string += "async ";

	if (m_flags & FunctionTypeFlag_Unsafe)
		string += "unsafe ";

	if (!m_callConv->isDefault()) {
		string = m_callConv->getCallConvDisplayString();
		string += ' ';
	}

	if (!string.isEmpty())
		string.chop(1);

	return string;
}

bool
FunctionType::resolveImports() {
	bool result = m_returnType->ensureNoImports();
	if (!result)
		return false;

	size_t count = m_argArray.getCount();
	for (size_t i = 0; i < count; i++) {
		result = m_argArray[i]->getType()->ensureNoImports();
		if (!result)
			return false;
	}

	return true;
}

bool
FunctionType::calcLayout() {
	bool result = m_returnType->ensureLayout();
	if (!result)
		return false;

	if ((m_flags & FunctionTypeFlag_ErrorCode) &&
		!(m_returnType->getTypeKindFlags() & TypeKindFlag_ErrorCode)) {
		err::setFormatStringError("'%s' cannot be used as error code", m_returnType->getTypeString().sz());
		return false;
	}

	size_t count = m_argArray.getCount();
	for (size_t i = 0; i < count; i++) {
		result = m_argArray[i]->getType()->ensureLayout();
		if (!result)
			return false;
	}

	return true;
}

void
FunctionType::prepareSignature() {
	sl::String signature;
	uint_t signatureFlags = createSignature(
		&signature,
		&m_argSignature,
		m_callConv,
		m_returnType,
		m_argArray,
		m_argArray.getCount(),
		m_flags
	);

	m_signature = signature;
	m_flags |= signatureFlags;
}

void
FunctionType::prepareTypeString() {
	TypeStringTuple* tuple = getTypeStringTuple();
	Type* returnType = (m_flags & FunctionTypeFlag_Async) ? m_asyncReturnType : m_returnType;

	sl::StringRef modifierString = getTypeModifierString();
	tuple->m_typeStringPrefix = modifierString.isEmpty() ?
		returnType->getTypeStringPrefix() :
		returnType->getTypeStringPrefix() + ' ' + modifierString;

	tuple->m_typeStringSuffix = "(";

	if (!m_argArray.isEmpty()) {
		tuple->m_typeStringSuffix += m_argArray[0]->getArgString();

		size_t count = m_argArray.getCount();
		for (size_t i = 1; i < count; i++) {
			tuple->m_typeStringSuffix += ", ";
			tuple->m_typeStringSuffix += m_argArray[i]->getArgString();
		}

		if (m_flags & FunctionTypeFlag_VarArg)
			tuple->m_typeStringSuffix += ", ";
	}

	if (!(m_flags & FunctionTypeFlag_VarArg))
		tuple->m_typeStringSuffix += ")";
	else
		tuple->m_typeStringSuffix += "...)";

	tuple->m_typeStringSuffix += returnType->getTypeStringSuffix();
}

void
FunctionType::prepareDoxyLinkedText() {
	TypeStringTuple* tuple = getTypeStringTuple();
	Type* returnType = (m_flags & FunctionTypeFlag_Async) ? m_asyncReturnType : m_returnType;

	tuple->m_doxyLinkedTextPrefix = returnType->getDoxyLinkedTextPrefix();

	sl::String modifierString = getTypeModifierString();
	if (!modifierString.isEmpty()) {
		tuple->m_doxyLinkedTextPrefix += ' ';
		tuple->m_doxyLinkedTextPrefix += getTypeModifierString();
	}

	bool isUserType = (m_flags & ModuleItemFlag_User) != 0;

	tuple->m_doxyLinkedTextSuffix = "(";

	if (!m_argArray.isEmpty()) {
		tuple->m_doxyLinkedTextSuffix += m_argArray[0]->getArgDoxyLinkedText();

		size_t count = m_argArray.getCount();
		for (size_t i = 1; i < count; i++) {
			tuple->m_doxyLinkedTextSuffix += ", ";
			tuple->m_doxyLinkedTextSuffix += m_argArray[i]->getArgDoxyLinkedText();
		}

		if (m_flags & FunctionTypeFlag_VarArg)
			tuple->m_doxyLinkedTextSuffix += ", ";
	}

	if (!(m_flags & FunctionTypeFlag_VarArg))
		tuple->m_doxyLinkedTextSuffix += ")";
	else
		tuple->m_doxyLinkedTextSuffix += "...)";

	tuple->m_doxyLinkedTextSuffix += returnType->getDoxyLinkedTextSuffix();
}

void
FunctionType::prepareDoxyTypeString() {
	Type::prepareDoxyTypeString();
	appendDoxyArgString(&getTypeStringTuple()->m_doxyTypeString);
}

void
FunctionType::prepareLlvmType() {
	m_callConv->prepareFunctionType(this);
	ASSERT(m_llvmType);
}

void
FunctionType::prepareLlvmDiType() {
	m_llvmDiType = m_module->m_llvmDiBuilder.createSubroutineType(this);
}

void
FunctionType::appendDoxyArgString(sl::String* string) {
	size_t count = m_argArray.getCount();
	for (size_t i = 0; i < count; i++) {
		FunctionArg* arg = m_argArray[i];
		if (arg->getStorageKind() == StorageKind_This)
			continue;

		Type* type = arg->getType();
		string->appendFormat(
			"<param>\n"
			"<declname>%s</declname>\n"
			"<type>%s</type>\n"
			"<array>%s</array>\n",
			arg->getName().sz(),
			type->getDoxyLinkedTextPrefix().sz(),
			type->getTypeStringSuffix().sz()
		);

		if (arg->hasInitializer())
			string->appendFormat(
				"<defval>%s</defval>\n",
				arg->getInitializerString().sz()
			);

		string->append("</param>\n");
	}

	if (m_flags & FunctionTypeFlag_VarArg)
		string->append(
			"<param>\n"
			"<type>...</type>\n"
			"</param>\n"
		);
}

//..............................................................................

} // namespace ct
} // namespace jnc
