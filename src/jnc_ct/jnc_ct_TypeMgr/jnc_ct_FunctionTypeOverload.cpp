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
#include "jnc_ct_FunctionTypeOverload.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

size_t
FunctionTypeOverload::findOverload(FunctionType* type) const {
	if (!m_type)
		return -1;

	bool result = ensureLayout();
	if (!result)
		return -1;

	if (type->cmp(m_type) == 0)
		return 0;

	size_t count = m_overloadArray.getCount();
	for (size_t i = 0; i < count; i++) {
		FunctionType* overloadType = m_overloadArray[i];
		if (type->cmp(overloadType) == 0)
			return i + 1;
	}

	return -1;
}

size_t
FunctionTypeOverload::findShortOverload(FunctionType* type) const {
	if (!m_type)
		return -1;

	bool result = ensureLayout();
	if (!result)
		return -1;

	if (type->cmp(m_type->getShortType()) == 0)
		return 0;

	size_t count = m_overloadArray.getCount();
	for (size_t i = 0; i < count; i++) {
		FunctionType* overloadType = m_overloadArray[i];
		if (type->cmp(overloadType->getShortType()) == 0)
			return i + 1;
	}

	return -1;
}

size_t
FunctionTypeOverload::chooseOverload(
	Closure* closure,
	FunctionArg* const* argArray,
	size_t argCount,
	CastKind* castKind
) const {
	ASSERT(m_type);

	bool result = ensureLayout();
	if (!result)
		return -1;

	Module* module = m_type->getModule();
	CastKind bestCastKind = module->m_operatorMgr.getArgCastKind(closure, m_type, argArray, argCount);
	size_t bestOverload = bestCastKind ? 0 : -1;
	bool isAmbiguous = false;

	size_t count = m_overloadArray.getCount();
	for (size_t i = 0; i < count; i++) {
		FunctionType* overloadType = m_overloadArray[i];
		CastKind castKind = module->m_operatorMgr.getArgCastKind(closure, overloadType, argArray, argCount);
		if (!castKind)
			continue;

		if (castKind == bestCastKind)
			isAmbiguous = true;

		if (castKind > bestCastKind) {
			bestOverload = i + 1;
			bestCastKind = castKind;
			isAmbiguous = false;
		}
	}

	if (bestOverload == -1) {
		err::setFormatStringError("none of the %d overloads accept the specified argument list", count + 1);
		return -1;
	}

	if (isAmbiguous) {
		err::setFormatStringError("ambiguous call to overloaded function");
		return -1;
	}

	if (castKind)
		*castKind = bestCastKind;

	return bestOverload;
}

size_t
FunctionTypeOverload::chooseOverload(
	const Value* argValueArray,
	size_t argCount,
	CastKind* castKind
) const {
	ASSERT(m_type);

	bool result = ensureLayout();
	if (!result)
		return -1;

	Module* module = m_type->getModule();
	CastKind bestCastKind = module->m_operatorMgr.getArgCastKind(m_type, argValueArray, argCount);
	size_t bestOverload = bestCastKind ? 0 : -1;
	bool isAmbiguous = false;

	size_t count = m_overloadArray.getCount();
	for (size_t i = 0; i < count; i++) {
		FunctionType* overloadType = m_overloadArray[i];
		CastKind castKind = module->m_operatorMgr.getArgCastKind(overloadType, argValueArray, argCount);
		if (!castKind)
			continue;

		if (castKind == bestCastKind)
			isAmbiguous = true;

		if (castKind > bestCastKind) {
			bestOverload = i + 1;
			bestCastKind = castKind;
			isAmbiguous = false;
		}
	}

	if (bestOverload == -1) {
		err::setFormatStringError("none of the %d overloads accept the specified argument list", count + 1);
		return -1;
	}

	if (isAmbiguous) {
		err::setFormatStringError("ambiguous call to overloaded function");
		return -1;
	}

	if (castKind)
		*castKind = bestCastKind;

	return bestOverload;
}

size_t
FunctionTypeOverload::chooseOverload(
	const sl::ConstBoxList<Value>& argList,
	CastKind* castKind
) const {
	ASSERT(m_type);

	bool result = ensureLayout();
	if (!result)
		return -1;

	Module* module = m_type->getModule();
	CastKind bestCastKind = module->m_operatorMgr.getArgCastKind(m_type, argList);
	size_t bestOverload = bestCastKind ? 0 : -1;
	bool isAmbiguous = false;

	size_t count = m_overloadArray.getCount();
	for (size_t i = 0; i < count; i++) {
		FunctionType* overloadType = m_overloadArray[i];
		CastKind castKind = module->m_operatorMgr.getArgCastKind(overloadType, argList);
		if (!castKind)
			continue;

		if (castKind == bestCastKind)
			isAmbiguous = true;

		if (castKind > bestCastKind) {
			bestOverload = i + 1;
			bestCastKind = castKind;
			isAmbiguous = false;
		}
	}

	if (bestOverload == -1) {
		err::setFormatStringError("none of the %d overloads accept the specified argument list", count + 1);
		return -1;
	}

	if (isAmbiguous) {
		err::setFormatStringError("ambiguous call to overloaded function");
		return -1;
	}

	if (castKind)
		*castKind = bestCastKind;

	return bestOverload;
}

size_t
FunctionTypeOverload::chooseSetterOverload(
	const Value& value,
	CastKind* castKind
) const {
	ASSERT(m_type);

	bool result = ensureLayout();
	if (!result)
		return -1;

	Module* module = m_type->getModule();
	size_t setterValueIdx = m_type->getArgArray().getCount() - 1;
	ASSERT(setterValueIdx != -1);
	Type* setterValueArgType = m_type->getArgArray()[setterValueIdx]->getType();
	CastKind bestCastKind = module->m_operatorMgr.getCastKind(value, setterValueArgType);
	size_t bestOverload = bestCastKind ? 0 : -1;
	bool isAmbiguous = false;

	size_t count = m_overloadArray.getCount();
	for (size_t i = 0; i < count; i++) {
		FunctionType* overloadType = m_overloadArray[i];
		Type* setterValueArgType = overloadType->getArgArray() [setterValueIdx]->getType();

		CastKind castKind = module->m_operatorMgr.getCastKind(value, setterValueArgType);
		if (!castKind)
			continue;

		if (castKind == bestCastKind)
			isAmbiguous = true;

		if (castKind > bestCastKind) {
			bestOverload = i + 1;
			bestCastKind = castKind;
			isAmbiguous = false;
		}
	}

	if (bestOverload == -1) {
		err::setFormatStringError("none of the %d overloads accept the specified argument list", count + 1);
		return -1;
	}

	if (isAmbiguous) {
		err::setFormatStringError("ambiguous call to overloaded function");
		return -1;
	}

	if (castKind)
		*castKind = bestCastKind;

	return bestOverload;
}

size_t
FunctionTypeOverload::addOverload(FunctionType* type) {
	if (!m_type) {
		m_type = type;
		return 0;
	} else if (type->getArgSignature().cmp(m_type->getArgSignature()) == 0) {
		err::setFormatStringError("illegal function overload: duplicate argument signature");
		return -1;
	}

	size_t count = m_overloadArray.getCount();
	for (size_t i = 0; i < count; i++) {
		FunctionType* overloadType = m_overloadArray[i];

		if (type->getArgSignature().cmp(overloadType->getArgSignature()) == 0) {
			err::setFormatStringError("illegal function overload: duplicate argument signature");
			return -1;
		}
	}

	m_overloadArray.append(type);
	return m_overloadArray.getCount();
}

void
FunctionTypeOverload::copy(
	FunctionType* const* typeArray,
	size_t count
) {
	if (count) {
		m_type = typeArray[0];
		m_overloadArray.copy(typeArray + 1, count - 1);
	} else {
		m_type = NULL;
		m_overloadArray.clear();
	}
}

bool
FunctionTypeOverload::prepareImports() const {
	ASSERT(!(m_flags & (ModuleItemFlag_LayoutReady | TypeFlag_NoImports)));
	ASSERT(m_type);

	bool result = m_type->ensureNoImports();
	if (!result)
		return false;

	size_t count = m_overloadArray.getCount();
	for (size_t i = 0; i < count; i++) {
		result = m_overloadArray[i]->ensureNoImports();
		if (!result)
			return false;
	}

	m_flags |= TypeFlag_NoImports;
	return true;
}

bool
FunctionTypeOverload::prepareLayout() const {
	ASSERT(!(m_flags & ModuleItemFlag_LayoutReady));
	ASSERT(m_type);

	bool result = m_type->ensureLayout();
	if (!result)
		return false;

	size_t count = m_overloadArray.getCount();
	for (size_t i = 0; i < count; i++) {
		result = m_overloadArray[i]->ensureLayout();
		if (!result)
			return false;
	}

	m_flags |= ModuleItemFlag_LayoutReady;
	return true;
}

//.....................................................	........................

} // namespace ct
} // namespace jnc
