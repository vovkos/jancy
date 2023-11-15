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

#pragma once

#include "jnc_ct_BinOp.h"
#include "jnc_ct_UnOp_Arithmetic.h"
#include "jnc_String.h"

namespace jnc {
namespace ct {

//..............................................................................

Type*
getPtrCmpOperatorOperandType(
	const Value& opValue1,
	const Value& opValue2
);

bool
cmpStringOperator(
	BinOpKind opKind,
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
);

//..............................................................................

template <typename T>
class BinOp_Cmp: public BinaryOperator {
public:
	virtual
	bool
	op(
		const Value& rawOpValue1,
		const Value& rawOpValue2,
		Value* resultValue
	) {
		Type* type;

		if ((rawOpValue1.getType()->getTypeKind() == TypeKind_String) ||
			(rawOpValue2.getType()->getTypeKind() == TypeKind_String)) {
			type = getPrimitiveType(m_module, TypeKind_String);
		} else if ((rawOpValue1.getType()->getTypeKindFlags() & TypeKindFlag_Ptr) ||
			(rawOpValue2.getType()->getTypeKindFlags() & TypeKindFlag_Ptr)) {
			type = getPtrCmpOperatorOperandType(rawOpValue1, rawOpValue2);
		} else {
			type = getArithmeticOperatorResultType(rawOpValue1, rawOpValue2);
		}

		if (!type) {
			setOperatorError(rawOpValue1, rawOpValue2);
			return false;
		}

		Value opValue1;
		Value opValue2;

		bool result =
			castOperator(m_module, rawOpValue1, type, &opValue1) &&
			castOperator(m_module, rawOpValue2, type, &opValue2);

		if (!result)
			return false;

		if (!hasCodeGen(m_module)) {
			resultValue->setType(getPrimitiveType(m_module, TypeKind_Bool));
			return true;
		}

		if (opValue1.getValueKind() == ValueKind_Const && opValue2.getValueKind() == ValueKind_Const) {
			TypeKind typeKind = type->getTypeKind();
			switch (typeKind) {
			case TypeKind_String:
				resultValue->setConstBool(
					T::constOpString(
						(jnc::String*)opValue1.getConstData(),
						(jnc::String*)opValue2.getConstData()
					),
					m_module
				);
				break;

			case TypeKind_Int32:
			case TypeKind_Int32_u:
				resultValue->setConstBool(
					T::constOpInt32(
						opValue1.getInt32(),
						opValue2.getInt32(),
						(type->getTypeKindFlags() & TypeKindFlag_Unsigned) != 0
					),
					m_module
				);
				break;

			case TypeKind_Int64:
			case TypeKind_Int64_u:
				resultValue->setConstBool(
					T::constOpInt32(
						opValue1.getInt32(),
						opValue2.getInt32(),
						(type->getTypeKindFlags() & TypeKindFlag_Unsigned) != 0
					),
					m_module
				);
				break;

			case TypeKind_Float:
				resultValue->setConstBool(T::constOpFp32(opValue1.getFloat(), opValue2.getFloat()), m_module);
				break;

			case TypeKind_Double:
				resultValue->setConstBool(T::constOpFp64(opValue1.getDouble(), opValue2.getDouble()), m_module);
				break;

			default:
				ASSERT(false);
			}
		} else {
			TypeKind typeKind = type->getTypeKind();
			switch (typeKind) {
			case TypeKind_String:
				result = static_cast<T*>(this)->llvmOpString(
					opValue1,
					opValue2,
					resultValue
				);
				if (!result)
					return false;
				break;

			case TypeKind_Int32:
			case TypeKind_Int32_u:
			case TypeKind_Int64:
			case TypeKind_Int64_u:
				static_cast<T*>(this)->llvmOpInt(
					opValue1,
					opValue2,
					resultValue,
					(type->getTypeKindFlags() & TypeKindFlag_Unsigned) != 0
				);
				break;

			case TypeKind_Float:
			case TypeKind_Double:
				static_cast<T*>(this)->llvmOpFp(
					opValue1,
					opValue2,
					resultValue
				);
				break;

			default:
				ASSERT(false);
			}
		}

		return true;
	}

protected:
	bool
	llvmOpString(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	) {
		return cmpStringOperator(m_opKind, opValue1, opValue2, resultValue);
	}
};

//..............................................................................

class BinOp_Eq: public BinOp_Cmp<BinOp_Eq> {
public:
	BinOp_Eq() {
		m_opKind = BinOpKind_Eq;
	}

	static
	bool
	constOpString(
		const jnc::String* opValue1,
		const jnc::String* opValue2
	) {
		return opValue1->isEqual(opValue2);
	}

	static
	bool
	constOpInt32(
		int32_t opValue1,
		int32_t opValue2,
		bool isUnsigned
	) {
		return opValue1 == opValue2;
	}

	static
	bool
	constOpInt64(
		int64_t opValue1,
		int64_t opValue2,
		bool isUnsigned
	) {
		return opValue1 == opValue2;
	}

	static
	bool
	constOpFp32(
		float opValue1,
		float opValue2
	) {
		return opValue1 == opValue2;
	}

	static
	bool
	constOpFp64(
		double opValue1,
		double opValue2
	) {
		return opValue1 == opValue2;
	}

	bool
	llvmOpString(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	);

	llvm::Value*
	llvmOpInt(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue,
		bool isUnsigned
	);

	llvm::Value*
	llvmOpFp(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	);
};

//..............................................................................

class BinOp_Ne: public BinOp_Cmp<BinOp_Ne> {
public:
	BinOp_Ne() {
		m_opKind = BinOpKind_Ne;
	}

	static
	bool
	constOpString(
		const jnc::String* opValue1,
		const jnc::String* opValue2
	) {
		return !opValue1->isEqual(opValue2);
	}

	static
	bool
	constOpInt32(
		int32_t opValue1,
		int32_t opValue2,
		bool isUnsigned
	) {
		return opValue1 != opValue2;
	}

	static
	bool
	constOpInt64(
		int64_t opValue1,
		int64_t opValue2,
		bool isUnsigned
	) {
		return opValue1 != opValue2;
	}

	static
	bool
	constOpFp32(
		float opValue1,
		float opValue2
	) {
		return opValue1 != opValue2;
	}

	static
	bool
	constOpFp64(
		double opValue1,
		double opValue2
	) {
		return opValue1 != opValue2;
	}

	bool
	llvmOpString(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	);

	llvm::Value*
	llvmOpInt(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue,
		bool isUnsigned
	);

	llvm::Value*
	llvmOpFp(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	);
};

//..............................................................................

class BinOp_Lt: public BinOp_Cmp<BinOp_Lt> {
public:
	BinOp_Lt() {
		m_opKind = BinOpKind_Lt;
	}

	static
	bool
	constOpString(
		const jnc::String* opValue1,
		const jnc::String* opValue2
	) {
		return opValue1->cmp(opValue2) < 0;
	}

	static
	bool
	constOpInt32(
		int32_t opValue1,
		int32_t opValue2,
		bool isUnsigned
	) {
		return isUnsigned ? (uint32_t)opValue1 < (uint32_t)opValue2 : opValue1 < opValue2;
	}

	static
	bool
	constOpInt64(
		int64_t opValue1,
		int64_t opValue2,
		bool isUnsigned
	) {
		return isUnsigned ? (uint64_t)opValue1 < (uint64_t)opValue2 : opValue1 < opValue2;
	}

	static
	bool
	constOpFp32(
		float opValue1,
		float opValue2
	) {
		return opValue1 < opValue2;
	}

	static
	bool
	constOpFp64(
		double opValue1,
		double opValue2
	) {
		return opValue1 < opValue2;
	}

	llvm::Value*
	llvmOpInt(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue,
		bool isUnsigned
	);

	llvm::Value*
	llvmOpFp(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	);
};

//..............................................................................

class BinOp_Le: public BinOp_Cmp<BinOp_Le> {
public:
	BinOp_Le() {
		m_opKind = BinOpKind_Le;
	}

	static
	bool
	constOpString(
		const jnc::String* opValue1,
		const jnc::String* opValue2
	) {
		return opValue1->cmp(opValue2) <= 0;
	}

	static
	bool
	constOpInt32(
		int32_t opValue1,
		int32_t opValue2,
		bool isUnsigned
	) {
		return isUnsigned ? (uint32_t)opValue1 <= (uint32_t)opValue2 : opValue1 <= opValue2;
	}

	static
	bool
	constOpInt64(
		int64_t opValue1,
		int64_t opValue2,
		bool isUnsigned
	) {
		return isUnsigned ? (uint64_t)opValue1 <= (uint64_t)opValue2 : opValue1 <= opValue2;
	}

	static
	bool
	constOpFp32(
		float opValue1,
		float opValue2
	) {
		return opValue1 <= opValue2;
	}

	static
	bool
	constOpFp64(
		double opValue1,
		double opValue2
	) {
		return opValue1 <= opValue2;
	}

	llvm::Value*
	llvmOpInt(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue,
		bool isUnsigned
	);

	llvm::Value*
	llvmOpFp(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	);
};

//..............................................................................

class BinOp_Gt: public BinOp_Cmp<BinOp_Gt> {
public:
	BinOp_Gt() {
		m_opKind = BinOpKind_Gt;
	}

	static
	bool
	constOpString(
		const jnc::String* opValue1,
		const jnc::String* opValue2
	) {
		return opValue1->cmp(opValue2) > 0;
	}

	static
	bool
	constOpInt32(
		int32_t opValue1,
		int32_t opValue2,
		bool isUnsigned
	) {
		return isUnsigned ? (uint32_t)opValue1 > (uint32_t)opValue2 : opValue1 > opValue2;
	}

	static
	bool
	constOpInt64(
		int64_t opValue1,
		int64_t opValue2,
		bool isUnsigned
	) {
		return isUnsigned ? (uint64_t)opValue1 > (uint64_t)opValue2 : opValue1 > opValue2;
	}

	static
	bool
	constOpFp32(
		float opValue1,
		float opValue2
	) {
		return opValue1 > opValue2;
	}

	static
	bool
	constOpFp64(
		double opValue1,
		double opValue2
	) {
		return opValue1 > opValue2;
	}

	llvm::Value*
	llvmOpInt(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue,
		bool isUnsigned
	);

	llvm::Value*
	llvmOpFp(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	);
};

//..............................................................................

class BinOp_Ge: public BinOp_Cmp<BinOp_Ge> {
public:
	BinOp_Ge() {
		m_opKind = BinOpKind_Ge;
	}

	static
	bool
	constOpString(
		const jnc::String* opValue1,
		const jnc::String* opValue2
	) {
		return opValue1->cmp(opValue2) >= 0;
	}

	static
	bool
	constOpInt32(
		int32_t opValue1,
		int32_t opValue2,
		bool isUnsigned
	) {
		return isUnsigned ? (uint32_t)opValue1 >= (uint32_t)opValue2 : opValue1 >= opValue2;
	}

	static
	bool
	constOpInt64(
		int64_t opValue1,
		int64_t opValue2,
		bool isUnsigned
	) {
		return isUnsigned ? (uint64_t)opValue1 >= (uint64_t)opValue2 : opValue1 >= opValue2;
	}

	static
	bool
	constOpFp32(
		float opValue1,
		float opValue2
	) {
		return opValue1 >= opValue2;
	}

	static
	bool
	constOpFp64(
		double opValue1,
		double opValue2
	) {
		return opValue1 >= opValue2;
	}

	llvm::Value*
	llvmOpInt(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue,
		bool isUnsigned
	);

	llvm::Value*
	llvmOpFp(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	);
};

//..............................................................................

} // namespace ct
} // namespace jnc
