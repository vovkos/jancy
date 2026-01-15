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
#include "jnc_ct_PropertyType.h"
#include "jnc_ct_Decl.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

uint_t
PropertyType::createSignature(
	sl::String* string,
	FunctionType* getterType,
	const FunctionTypeOverload& setterType,
	uint_t flags
) {
	*string = "Y";

	if (flags & PropertyTypeFlag_Bindable)
		*string += 'b';

	*string += getterType->getSignature();

	uint_t signatureFlags = getterType->getFlags() & TypeFlag_SignatureFinal;
	uint_t propagageFlags = getterType->getFlags();

	size_t overloadCount = setterType.getOverloadCount();
	for (size_t i = 0; i < overloadCount; i++) {
		FunctionType* overloadType = setterType.getOverload(i);
		*string += overloadType->getSignature();
		uint_t overloadTypeFlags = overloadType->getFlags();
		signatureFlags &= overloadTypeFlags;
		propagageFlags |= overloadTypeFlags;
	}

	return
		signatureFlags |
		TypeFlag_SignatureReady |
		(propagageFlags & TypeFlag_PropagateMask);
}

sl::StringRef
PropertyType::getTypeModifierString() {
	sl::String string;

	if (m_flags & PropertyTypeFlag_Const)
		string += "const ";

	if (m_flags & PropertyTypeFlag_Bindable)
		string += "bindable ";

	// to make output cleaner: don't append 'indexed' to simple member properties
	// -- even though they ARE indexed

	size_t argCount = m_getterType->getArgArray().getCount();
	if (argCount >= 2 || argCount == 1 && !m_getterType->isMemberMethodType())
		string += "indexed ";

	if (!string.isEmpty())
		string.chop(1);

	return string;
}

void
PropertyType::prepareSignature() {
	sl::String signature;
	uint_t signatureFlags = createSignature(&signature, m_getterType, m_setterType, m_flags);
	m_signature = signature;
	m_flags |= signatureFlags & TypeFlag_SignatureMask;
}

sl::StringRef
PropertyType::createItemString(size_t index) {
	switch (index) {
	case TypeStringKind_Prefix:
	case TypeStringKind_DoxyLinkedTextPrefix: {
		Type* returnType = getReturnType();
		sl::String string = returnType->getItemString(index);
		sl::StringRef modifierString = getTypeModifierString();
		if (!modifierString.isEmpty()) {
			string += ' ';
			string += modifierString;
		}

		string += " property";
		return string;
		}

	case TypeStringKind_Suffix:
	case TypeStringKind_DoxyLinkedTextSuffix:
		return m_getterType->getItemString(index);

	case TypeStringKind_DoxyTypeString: {
		sl::String string = Type::createItemString(index);
		m_getterType->appendDoxyArgString(&string);
		return string;
		}

	default:
		return Type::createItemString(index);
	}
}

Type*
PropertyType::calcFoldedDualType(
	bool isAlien,
	uint_t ptrFlags
) {
	ASSERT(m_flags & TypeFlag_Dual);

	FunctionType* getterType = (FunctionType*)m_getterType->getActualTypeIfDual(isAlien, ptrFlags);
	FunctionTypeOverload setterType;

	size_t count = m_setterType.getOverloadCount();
	for (size_t i = 0; i < count; i++) {
		FunctionType* overloadType = (FunctionType*)m_setterType.getOverload(i)->getActualTypeIfDual(isAlien, ptrFlags);
		setterType.forceAddOverload(overloadType);
	}

	return m_module->m_typeMgr.getPropertyType(
		getterType,
		setterType,
		m_flags & PropertyTypeFlag__All
	);
}

bool
PropertyType::deduceTemplateArgs(
	sl::Array<Type*>* templateArgTypeArray,
	Type* referenceType
) {
	if (referenceType->getTypeKind() != TypeKind_Property) {
		setTemplateArgDeductionError(referenceType);
		return false;
	}

	PropertyType* type = (PropertyType*)referenceType;
	return m_getterType->getReturnType()->deduceTemplateArgs(
		templateArgTypeArray,
		type->m_getterType->getReturnType()
	);
}

//..............................................................................

} // namespace ct
} // namespace jnc
