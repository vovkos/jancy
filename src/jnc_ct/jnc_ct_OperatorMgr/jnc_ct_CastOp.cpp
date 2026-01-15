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
#include "jnc_ct_CastOp.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

CastKind
getConstCastKind(
	uint_t srcFlags,
	uint_t dstFlags
) {
	static CastKind castKindTable[5][5] = {
		{	// 0 - mutable
			CastKind_Implicit,           // 0 - mutable
			CastKind_ImplicitCrossConst, // 1 - const
			CastKind_ImplicitCrossConst, // 2 - const?
			CastKind_None,               // 3 - invalid
			CastKind_ImplicitCrossConst, // 4 - constif
		},
		{	// 1 - const
			CastKind_None,               // 0 - mutable
			CastKind_Implicit,           // 1 - const
			CastKind_Implicit,           // 2 - const?
			CastKind_None,               // 3 - invalid
			CastKind_None,               // 4 - constif
		},
		{	// 2 - const?
			CastKind_None,               // 0 - mutable
			CastKind_Implicit,           // 1 - const
			CastKind_Implicit,           // 2 - const?
			CastKind_None,               // 3 - invalid
			CastKind_Implicit,           // 4 - constif
		},
		{	// 3 - invalid
		},
		{	// 4 - constif
			CastKind_None,               // 0 - mutable
			CastKind_Implicit,           // 1 - const
			CastKind_Implicit,           // 2 - const?
			CastKind_None,               // 3 - invalid
			CastKind_Implicit,           // 4 - constif
		},
	};

	size_t srcIdx = getConstPtrFlagIdx(srcFlags);
	size_t dstIdx = getConstPtrFlagIdx(dstFlags);
	ASSERT(srcIdx < countof(castKindTable) && dstIdx < countof(castKindTable));
	return castKindTable[srcIdx][dstIdx];
}

static
inline
sl::String
getConstTypeString(Type* type) {
	sl::String string = type->getTypeStringPrefix() + " const";

	const sl::String& suffix = type->getTypeStringSuffix();
	if (!suffix.isEmpty()) {
		string += ' ';
		string += suffix;
	}

	return string;
}

err::Error
setCastError(
	const Value& opValue,
	Type* dstType,
	CastKind castKind
) {
	const char* format;

	switch (castKind) {
	case CastKind_Explicit:
		format = "explicit cast is needed to convert from '%s' to '%s'";
		break;

	case CastKind_Dynamic:
		format = "dynamic cast is needed to convert from '%s' to '%s'";
		break;

	default:
		format = "cannot convert from '%s' to '%s'";
		break;
	}

	sl::StringRef opValueString;
	ValueKind valueKind = opValue.getValueKind();
	switch (valueKind) {
	case ValueKind_Void:
		opValueString = "void";
		break;

	case ValueKind_Null:
		opValueString = "null";
		break;

	case ValueKind_FunctionOverload:
		opValueString = opValue.getFunctionOverload()->getItemName();
		break;

	default:
		opValueString = opValue.getValueKind() == ValueKind_Const ?
			getConstTypeString(opValue.getType()) :
			opValue.getClosureAwareType()->getTypeString();
	}

	return err::setFormatStringError(format, opValueString.sz(), dstType->getTypeString().sz());
}

err::Error
setUnsafeCastError(
	Type* srcType,
	Type* dstType
) {
	return err::setFormatStringError(
		"'%s' to '%s' cast is only permitted in unsafe regions",
		srcType->getTypeString().sz(),
		dstType->getTypeString().sz()
	);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
castOperator(
	Module* module,
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	return module->m_operatorMgr.castOperator(opValue, type, resultValue);
}

//..............................................................................

CastOperator::CastOperator() {
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);

	m_opFlags = 0;
}

bool
CastOperator::cast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	if (opValue.getValueKind() != ValueKind_Const)
		return llvmCast(opValue, type, resultValue);

	if (type->getTypeKind() == TypeKind_Void) {
		resultValue->setVoid(m_module);
		return true;
	}

	return constCastImpl(opValue, type, resultValue);
}

//..............................................................................

CastKind
Cast_Typedef::getCastKind(
	const Value& opValue,
	Type* type
) {
	ASSERT(type->getTypeKind() == TypeKind_TypedefShadow);
	TypedefShadowType* shadowType = (TypedefShadowType*)type;
	return m_module->m_operatorMgr.getCastKind(opValue, shadowType->getActualType());
}

bool
Cast_Typedef::constCast(
	const Value& opValue,
	Type* type,
	void* dst
) {
	ASSERT(type->getTypeKind() == TypeKind_TypedefShadow);
	TypedefShadowType* shadowType = (TypedefShadowType*)type;

	Value value;
	bool result = m_module->m_operatorMgr.castOperator(opValue, shadowType->getActualType(), &value);
	if (!result)
		return false;

	if (value.getValueKind() == ValueKind_Const) {
		ASSERT(type->getSize() == value.getType()->getSize());
		memcpy(dst, value.getConstData(), type->getSize());
	}

	return true;
}

bool
Cast_Typedef::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	ASSERT(type->getTypeKind() == TypeKind_TypedefShadow);
	TypedefShadowType* shadowType = (TypedefShadowType*)type;
	return m_module->m_operatorMgr.castOperator(opValue, shadowType->getActualType(), resultValue);
}

//..............................................................................

bool
Cast_Copy::constCast(
	const Value& opValue,
	Type* type,
	void* dst
) {
	size_t srcSize = opValue.getType()->getSize();
	size_t dstSize = type->getSize();

	ASSERT(srcSize == dstSize);

	memcpy(dst, opValue.getConstData(), dstSize);
	return true;
}

bool
Cast_Copy::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	m_module->m_llvmIrBuilder.createBitCast(opValue, type, resultValue);
	return true;
}

//..............................................................................

CastKind
Cast_Master::getCastKind(
	const Value& rawOpValue,
	Type* type
) {
	if (!rawOpValue.getType())
		return CastKind_None;

	CastOperator* op = getCastOperator(rawOpValue, type);
	if (!op)
		return CastKind_None;

	Value opValue = rawOpValue;

	uint_t opFlags = op->getOpFlags();
	if (opFlags != m_opFlags) {
		bool result = m_module->m_operatorMgr.prepareOperandType(&opValue, opFlags);
		if (!result)
			return CastKind_None;
	}

	return op->getCastKind(opValue, type);
}

bool
Cast_Master::cast(
	const Value& rawOpValue,
	Type* type,
	Value* resultValue
) {
	CastOperator* op = getCastOperator(rawOpValue, type);
	if (!op) {
		setCastError(rawOpValue, type);
		return false;
	}

	Value opValue = rawOpValue;

	uint_t opFlags = op->getOpFlags();
	if (opFlags != m_opFlags) {
		bool result = m_module->m_operatorMgr.prepareOperand(&opValue, opFlags);
		if (!result)
			return false;
	}

	return op->cast(opValue, type, resultValue);
}

//..............................................................................

CastKind
Cast_SuperMaster::getCastKind(
	const Value& rawOpValue,
	Type* type
) {
	if (!rawOpValue.getType())
		return CastKind_None;

	CastOperator* operator1 = NULL;
	CastOperator* operator2 = NULL;
	Type* intermediateType = NULL;

	bool result = getCastOperators(
		rawOpValue,
		type,
		&operator1,
		&operator2,
		&intermediateType
	);

	if (!result)
		return CastKind_None;

	ASSERT(operator1);

	Value opValue = rawOpValue;

	uint_t opFlags1 = operator1->getOpFlags();
	if (opFlags1 != m_opFlags) {
		result = m_module->m_operatorMgr.prepareOperandType(&opValue, opFlags1);
		if (!result)
			return CastKind_None;
	}

	if (!operator2)
		return operator1->getCastKind(opValue, type);

	CastKind castKind1 = operator1->getCastKind(opValue, intermediateType);
	CastKind castKind2 = operator2->getCastKind(intermediateType, type);
	return AXL_MIN(castKind1, castKind2);
}

bool
Cast_SuperMaster::cast(
	const Value& rawOpValue,
	Type* type,
	Value* resultValue
) {
	CastOperator* operator1 = NULL;
	CastOperator* operator2 = NULL;
	Type* intermediateType = NULL;

	bool result = getCastOperators(
		rawOpValue,
		type,
		&operator1,
		&operator2,
		&intermediateType
	);

	if (!result) {
		setCastError(rawOpValue, type);
		return false;
	}

	ASSERT(operator1);

	Value opValue = rawOpValue;

	uint_t opFlags1 = operator1->getOpFlags();
	if (opFlags1 != m_opFlags) {
		bool result = m_module->m_operatorMgr.prepareOperand(&opValue, opFlags1);
		if (!result)
			return false;
	}

	if (!operator2)
		return operator1->cast(opValue, type, resultValue);

	Value tmpValue;
	return
		operator1->cast(opValue, intermediateType, &tmpValue) &&
		operator2->cast(tmpValue, type, resultValue);
}

//..............................................................................

} // namespace ct
} // namespace jnc
