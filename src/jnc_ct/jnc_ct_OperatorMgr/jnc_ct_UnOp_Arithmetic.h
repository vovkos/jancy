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

#include "jnc_ct_UnOp.h"
#include "jnc_ct_CastOp.h"

namespace jnc {
namespace ct {

//..............................................................................

Type*
getArithmeticOperatorResultType(Type* opType);

JNC_INLINE
Type*
getArithmeticOperatorResultType(const Value& opValue)
{
	return getArithmeticOperatorResultType(opValue.getType());
}

JNC_INLINE
Type*
getArithmeticOperatorResultType(
	Type* opType1,
	Type* opType2
	)
{
	return getArithmeticOperatorResultType(
		opType1->getTypeKind() > opType2->getTypeKind() ?
			opType1 :
			opType2
		);
}

JNC_INLINE
Type*
getArithmeticOperatorResultType(
	const Value& opValue1,
	const Value& opValue2
	)
{
	return getArithmeticOperatorResultType(opValue1.getType(), opValue2.getType());
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename T>
class UnOp_Arithmetic: public UnaryOperator
{
public:
	enum
	{
		isIntegerOnly = false
	};

public:
	virtual
	bool
	op(
		const Value& rawOpValue,
		Value* resultValue
		)
	{
		Type* type = getArithmeticOperatorResultType(rawOpValue);

		Value opValue;
		bool result = castOperator(m_module, rawOpValue, type, &opValue);
		if (!result)
			return false;

		if (!m_module->hasCodeGen())
		{
			resultValue->setType(type);
			return true;
		}

		if (opValue.getValueKind() == ValueKind_Const)
		{
			TypeKind typeKind = type->getTypeKind();
			switch (typeKind)
			{
			case TypeKind_Int32:
			case TypeKind_Int32_u:
				resultValue->setConstInt32(T::constOpInt32(opValue.getInt32()), type);
				break;

			case TypeKind_Int64:
			case TypeKind_Int64_u:
				resultValue->setConstInt64(T::constOpInt64(opValue.getInt64()), type);
				break;

			case TypeKind_Float:
				resultValue->setConstFloat(T::constOpFp32(opValue.getFloat()), m_module);
				break;

			case TypeKind_Double:
				resultValue->setConstDouble(T::constOpFp64(opValue.getDouble()), m_module);
				break;

			default:
				ASSERT(false);
			}
		}
		else
		{
			TypeKind typeKind = type->getTypeKind();
			switch (typeKind)
			{
			case TypeKind_Int32:
			case TypeKind_Int32_u:
			case TypeKind_Int64:
			case TypeKind_Int64_u:
				static_cast<T*> (this)->llvmOpInt(opValue, type, resultValue);
				break;

			case TypeKind_Float:
			case TypeKind_Double:
				static_cast<T*> (this)->llvmOpFp(opValue, type, resultValue);
				break;

			default:
				ASSERT(false);
			}
		}

		return true;
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename T>
class UnOp_IntegerOnly: public UnOp_Arithmetic<T>
{
public:
	enum
	{
		isIntegerOnly = true
	};

public:
	static
	float
	constOpFp32(float opValue)
	{
		return 0;
	}

	static
	double
	constOpFp64(double opValue)
	{
		return 0;
	}

	llvm::Value*
	llvmOpFp(
		const Value& opValue,
		Type* resultType,
		Value* resultValue
		)
	{
		ASSERT(false);
		return NULL;
	}
};

//..............................................................................

class UnOp_Plus: public UnOp_Arithmetic<UnOp_Plus>
{
public:
	UnOp_Plus()
	{
		m_opKind = UnOpKind_Plus;
	}

	static
	int32_t
	constOpInt32(int32_t opValue)
	{
		return +opValue;
	}

	static
	int64_t
	constOpInt64(int64_t opValue)
	{
		return +opValue;
	}

	static
	float
	constOpFp32(float opValue)
	{
		return +opValue;
	}

	static
	double
	constOpFp64(double opValue)
	{
		return +opValue;
	}

	static
	llvm::Value*
	llvmOpInt(
		const Value& opValue,
		Type* resultType,
		Value* resultValue
		)
	{
		*resultValue = opValue;
		return resultValue->getLlvmValue();
	}

	static
	llvm::Value*
	llvmOpFp(
		const Value& opValue,
		Type* resultType,
		Value* resultValue
		)
	{
		*resultValue = opValue;
		return resultValue->getLlvmValue();
	}
};

//..............................................................................

class UnOp_Minus: public UnOp_Arithmetic<UnOp_Minus>
{
public:
	UnOp_Minus()
	{
		m_opKind = UnOpKind_Minus;
	}

	static
	int32_t
	constOpInt32(int32_t opValue)
	{
		return -opValue;
	}

	static
	int64_t
	constOpInt64(int64_t opValue)
	{
		return -opValue;
	}

	static
	float
	constOpFp32(float opValue)
	{
		return -opValue;
	}

	static
	double
	constOpFp64(double opValue)
	{
		return -opValue;
	}

	llvm::Value*
	llvmOpInt(
		const Value& opValue,
		Type* resultType,
		Value* resultValue
		);

	llvm::Value*
	llvmOpFp(
		const Value& opValue,
		Type* resultType,
		Value* resultValue
		);
};

//..............................................................................

class UnOp_BwNot: public UnOp_IntegerOnly<UnOp_BwNot>
{
public:
	UnOp_BwNot()
	{
		m_opKind = UnOpKind_BwNot;
	};

	static
	int32_t
	constOpInt32(int32_t opValue)
	{
		return ~opValue;
	}

	static
	int64_t
	constOpInt64(int64_t opValue)
	{
		return ~opValue;
	}

	llvm::Value*
	llvmOpInt(
		const Value& opValue,
		Type* resultType,
		Value* resultValue
		);

};

//..............................................................................

} // namespace ct
} // namespace jnc
