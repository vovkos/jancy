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

DerivableType*
FunctionType::getThisTargetType() const {
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

sl::StringRef
FunctionType::getTypeModifierString() const {
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

	if (m_flags & FunctionTypeFlag_Async) {
		result = m_asyncReturnType->ensureLayout();
		if (!result)
			return false;
	}

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

	return m_shortType == this || m_shortType->ensureLayout();
}

void
FunctionType::prepareSignature() {
	sl::String signature;
	uint_t flags = createSignature(
		&signature,
		&m_argSignature,
		m_callConv->getCallConvKind(),
		m_returnType,
		m_argArray,
		m_flags
	);

	m_signature = signature;
	m_flags |= flags;
}

sl::StringRef
FunctionType::createItemString(size_t index) {
	switch (index) {
	case TypeStringKind_Prefix:
	case TypeStringKind_DoxyLinkedTextPrefix: {
		Type* returnType = (m_flags & FunctionTypeFlag_Async) ? m_asyncReturnType : m_returnType;
		sl::StringRef modifierString = getTypeModifierString();
		return modifierString.isEmpty() ?
			returnType->getItemString(index) :
			returnType->getItemString(index) + ' ' + modifierString;
		}

	case TypeStringKind_Suffix:
		return createArgString<false>();

	case TypeStringKind_DoxyLinkedTextSuffix:
		return createArgString<true>();

	case TypeStringKind_DoxyTypeString: {
		sl::String string = Type::createItemString(index);
		appendDoxyArgString(&string);
		return string;
		}

	default:
		return Type::createItemString(index);
	}
}

template <bool IsDoxyLinkedText>
sl::String
FunctionType::createArgString() {
	sl::String string;

	if (m_argArray.isEmpty())
		string = (m_flags & FunctionTypeFlag_VarArg) ? "(...)" : "()";
	else {
		string = '(';
		m_argArray[0]->appendArgString<IsDoxyLinkedText>(&string);

		size_t count = m_argArray.getCount();
		for (size_t i = 1; i < count; i++) {
			string += ", ";
			m_argArray[i]->appendArgString<IsDoxyLinkedText>(&string);
		}

		if (m_flags & FunctionTypeFlag_VarArg)
			string += ", ...)";
		else
			string += ')';
	}

	Type* returnType = (m_flags & FunctionTypeFlag_Async) ? m_asyncReturnType : m_returnType;
	string += returnType->getTypeStringSuffix();
	return string;
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
FunctionType::appendDoxyArgString(sl::String* string) const {
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
				arg->getInitializerString_xml().sz()
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

bool
FunctionType::deduceTemplateArgs(
	sl::Array<Type*>* templateArgTypeArray,
	Type* referenceType
) {
	if (referenceType->getTypeKind() != TypeKind_Function) {
		setTemplateArgDeductionError(referenceType);
		return false;
	}

	FunctionType* type = (FunctionType*)referenceType;

	bool result = m_returnType->deduceTemplateArgs(templateArgTypeArray, type->m_returnType);

	size_t selfArgCount = m_argArray.getCount();
	size_t referenceArgCount = type->m_argArray.getCount();
	size_t argCount = AXL_MIN(selfArgCount, referenceArgCount);

	for (size_t i = 0; i < argCount; i++) {
		FunctionArg* selfArg = m_argArray[i];
		FunctionArg* referenceArg = type->m_argArray[i];
		result = selfArg->getType()->deduceTemplateArgs(
			templateArgTypeArray,
			referenceArg->getType()
		) && result;
	}

	if (selfArgCount != referenceArgCount) {
		setTemplateArgDeductionError(referenceType);
		return false;
	}

	return result;
}

//..............................................................................

} // namespace ct
} // namespace jnc
