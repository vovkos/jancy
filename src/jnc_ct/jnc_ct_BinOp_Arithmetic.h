// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_BinOp.h"
#include "jnc_ct_UnOp_Arithmetic.h"
#include "jnc_ct_EnumType.h"

namespace jnc {
namespace ct {

//..............................................................................

template <typename T>
class BinOp_Arithmetic: public BinaryOperator
{
public:
	enum
	{
		isIntegerOnly = false
	};

public:
	virtual
	Type*
	getResultType (
		const Value& opValue1,
		const Value& opValue2
		)
	{
		return getArithmeticResultType (opValue1, opValue2);
	}

	virtual
	bool
	op (
		const Value& rawOpValue1,
		const Value& rawOpValue2,
		Value* resultValue
		)
	{
		// BwOr overrides GetResultType, but here we need original one

		Type* type = getArithmeticResultType (rawOpValue1, rawOpValue2);
		if (!type)
			return false;

		Value opValue1;
		Value opValue2;

		bool result =
			castOperator (m_module, rawOpValue1, type, &opValue1) &&
			castOperator (m_module, rawOpValue2, type, &opValue2);

		if (!result)
			return false;

		if (opValue1.getValueKind () == ValueKind_Const && opValue2.getValueKind () == ValueKind_Const)
		{
			TypeKind typeKind = type->getTypeKind ();
			switch (typeKind)
			{
			case TypeKind_Int32:
			case TypeKind_Int32_u:
				resultValue->setConstInt32 (
					T::constOpInt32 (
						opValue1.getInt32 (),
						opValue2.getInt32 (),
						(type->getTypeKindFlags () & TypeKindFlag_Unsigned) != 0
						),
					type
					);
				break;

			case TypeKind_Int64:
			case TypeKind_Int64_u:
				resultValue->setConstInt64 (
					T::constOpInt64 (
						opValue1.getInt64 (),
						opValue2.getInt64 (),
						(type->getTypeKindFlags () & TypeKindFlag_Unsigned) != 0
						),
					type
					);
				break;

			case TypeKind_Float:
				resultValue->setConstFloat (T::constOpFp32 (opValue1.getFloat (), opValue2.getFloat ()), m_module);
				break;

			case TypeKind_Double:
				resultValue->setConstDouble (T::constOpFp64 (opValue1.getDouble (), opValue2.getDouble ()), m_module);
				break;

			default:
				ASSERT (false);
			}
		}
		else
		{
			TypeKind typeKind = type->getTypeKind ();
			switch (typeKind)
			{
			case TypeKind_Int32:
			case TypeKind_Int32_u:
			case TypeKind_Int64:
			case TypeKind_Int64_u:
				static_cast <T*> (this)->llvmOpInt (
					opValue1,
					opValue2,
					type,
					resultValue,
					(type->getTypeKindFlags () & TypeKindFlag_Unsigned) != 0
					);
				break;

			case TypeKind_Float:
			case TypeKind_Double:
				static_cast <T*> (this)->llvmOpFp (
					opValue1,
					opValue2,
					type,
					resultValue
					);
				break;

			default:
				ASSERT (false);
			}

			if (!result)
				return false;
		}

		return true;
	}

protected:
	Type*
	getArithmeticResultType (
		const Value& opValue1,
		const Value& opValue2
		)
	{
		Type* type = getArithmeticOperatorResultType (opValue1, opValue2);
		if (!type || T::isIntegerOnly && !(type->getTypeKindFlags () & TypeKindFlag_Integer))
		{
			setOperatorError (opValue1, opValue2);
			return NULL;
		}

		return type;
	}

};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename T>
class BinOp_IntegerOnly: public BinOp_Arithmetic <T>
{
public:
	enum
	{
		isIntegerOnly = true
	};

public:
	static
	float
	constOpFp32 (
		float opValue1,
		float opValue2
		)
	{
		return 0;
	}

	static
	double
	constOpFp64 (
		double opValue1,
		double opValue2
		)
	{
		return 0;
	}

	llvm::Value*
	llvmOpFp (
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue
		)
	{
		ASSERT (false);
		return NULL;
	}
};

//..............................................................................

class BinOp_Add: public BinOp_Arithmetic <BinOp_Add>
{
public:
	BinOp_Add ()
	{
		m_opKind = BinOpKind_Add;
	}

	virtual
	bool
	op (
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
		);

	static
	int32_t
	constOpInt32 (
		int32_t opValue1,
		int32_t opValue2,
		bool isUnsigned
		)
	{
		return opValue1 + opValue2;
	}

	static
	int64_t
	constOpInt64 (
		int64_t opValue1,
		int64_t opValue2,
		bool isUnsigned
		)
	{
		return opValue1 + opValue2;
	}

	static
	float
	constOpFp32 (
		float opValue1,
		float opValue2
		)
	{
		return opValue1 + opValue2;
	}

	static
	double
	constOpFp64 (
		double opValue1,
		double opValue2
		)
	{
		return opValue1 + opValue2;
	}

	llvm::Value*
	llvmOpInt (
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue,
		bool isUnsigned
		);

	llvm::Value*
	llvmOpFp (
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue
		);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class BinOp_Sub: public BinOp_Arithmetic <BinOp_Sub>
{
public:
	BinOp_Sub ()
	{
		m_opKind = BinOpKind_Sub;
	}

	virtual
	bool
	op (
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
		);

	static
	int32_t
	constOpInt32 (
		int32_t opValue1,
		int32_t opValue2,
		bool isUnsigned
		)
	{
		return opValue1 - opValue2;
	}

	static
	int64_t
	constOpInt64 (
		int64_t opValue1,
		int64_t opValue2,
		bool isUnsigned
		)
	{
		return opValue1 - opValue2;
	}

	static
	float
	constOpFp32 (
		float opValue1,
		float opValue2
		)
	{
		return opValue1 - opValue2;
	}

	static
	double
	constOpFp64 (
		double opValue1,
		double opValue2
		)
	{
		return opValue1 - opValue2;
	}

	llvm::Value*
	llvmOpInt (
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue,
		bool isUnsigned
		);

	llvm::Value*
	llvmOpFp (
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue
		);
};

//..............................................................................

class BinOp_Mul: public BinOp_Arithmetic <BinOp_Mul>
{
public:
	BinOp_Mul ()
	{
		m_opKind = BinOpKind_Mul;
	}

	static
	int32_t
	constOpInt32 (
		int32_t opValue1,
		int32_t opValue2,
		bool isUnsigned
		)
	{
		return opValue1 * opValue2;
	}

	static
	int64_t
	constOpInt64 (
		int64_t opValue1,
		int64_t opValue2,
		bool isUnsigned
		)
	{
		return opValue1 * opValue2;
	}

	static
	float
	constOpFp32 (
		float opValue1,
		float opValue2
		)
	{
		return opValue1 * opValue2;
	}

	static
	double
	constOpFp64 (
		double opValue1,
		double opValue2
		)
	{
		return opValue1 * opValue2;
	}

	llvm::Value*
	llvmOpInt (
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue,
		bool isUnsigned
		);

	llvm::Value*
	llvmOpFp (
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue
		);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class BinOp_Div: public BinOp_Arithmetic <BinOp_Div>
{
public:
	BinOp_Div ()
	{
		m_opKind = BinOpKind_Div;
	}

	static
	int32_t
	constOpInt32 (
		int32_t opValue1,
		int32_t opValue2,
		bool isUnsigned
		)
	{
		return isUnsigned ? (uint32_t) opValue1 / (uint32_t) opValue2 : opValue1 / opValue2;
	}

	static
	int64_t
	constOpInt64 (
		int64_t opValue1,
		int64_t opValue2,
		bool isUnsigned
		)
	{
		return isUnsigned ? (uint64_t) opValue1 / (uint64_t) opValue2 : opValue1 / opValue2;
	}

	static
	float
	constOpFp32 (
		float opValue1,
		float opValue2
		)
	{
		return opValue1 / opValue2;
	}

	static
	double
	constOpFp64 (
		double opValue1,
		double opValue2
		)
	{
		return opValue1 / opValue2;
	}

	llvm::Value*
	llvmOpInt (
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue,
		bool isUnsigned
		);

	llvm::Value*
	llvmOpFp (
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue
		);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class BinOp_Mod: public BinOp_IntegerOnly <BinOp_Mod>
{
public:
	BinOp_Mod ()
	{
		m_opKind = BinOpKind_Mod;
	}

	static
	int32_t
	constOpInt32 (
		int32_t opValue1,
		int32_t opValue2,
		bool isUnsigned
		)
	{
		return isUnsigned ? (uint32_t) opValue1 % (uint32_t) opValue2 : opValue1 % opValue2;
	}

	static
	int64_t
	constOpInt64 (
		int64_t opValue1,
		int64_t opValue2,
		bool isUnsigned
		)
	{
		return isUnsigned ? (uint64_t) opValue1 % (uint64_t) opValue2 : opValue1 % opValue2;
	}

	llvm::Value*
	llvmOpInt (
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue,
		bool isUnsigned
		);
};

//..............................................................................

class BinOp_Shl: public BinOp_IntegerOnly <BinOp_Shl>
{
public:
	BinOp_Shl ()
	{
		m_opKind = BinOpKind_Shl;
	}

	static
	int32_t
	constOpInt32 (
		int32_t opValue1,
		int32_t opValue2,
		bool isUnsigned
		)
	{
		return opValue1 << opValue2;
	}

	static
	int64_t
	constOpInt64 (
		int64_t opValue1,
		int64_t opValue2,
		bool isUnsigned
		)
	{
		return opValue1 << opValue2;
	}

	llvm::Value*
	llvmOpInt (
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue,
		bool isUnsigned
		);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class BinOp_Shr: public BinOp_IntegerOnly <BinOp_Shr>
{
public:
	BinOp_Shr ()
	{
		m_opKind = BinOpKind_Shr;
	}

	static
	int32_t
	constOpInt32 (
		int32_t opValue1,
		int32_t opValue2,
		bool isUnsigned
		)
	{
		return opValue1 >> opValue2;
	}

	static
	int64_t
	constOpInt64 (
		int64_t opValue1,
		int64_t opValue2,
		bool isUnsigned
		)
	{
		return opValue1 >> opValue2;
	}

	llvm::Value*
	llvmOpInt (
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue,
		bool isUnsigned
		);
};

//..............................................................................

class BinOp_BwAnd: public BinOp_IntegerOnly <BinOp_BwAnd>
{
public:
	BinOp_BwAnd ();

	virtual
	Type*
	getResultType (
		const Value& opValue1,
		const Value& opValue2
		)
	{
		return
			isBitFlagEnumType (opValue1.getType ()) ? opValue1.getType () :
			isBitFlagEnumType (opValue2.getType ()) ? opValue2.getType () :
			getArithmeticResultType (opValue1, opValue2);
	}

	virtual
	bool
	op (
		const Value& rawOpValue1,
		const Value& rawOpValue2,
		Value* resultValue
		);

	static
	int32_t
	constOpInt32 (
		int32_t opValue1,
		int32_t opValue2,
		bool isUnsigned
		)
	{
		return opValue1 & opValue2;
	}

	static
	int64_t
	constOpInt64 (
		int64_t opValue1,
		int64_t opValue2,
		bool isUnsigned
		)
	{
		return opValue1 & opValue2;
	}

	llvm::Value*
	llvmOpInt (
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue,
		bool isUnsigned
		);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class BinOp_BwOr: public BinOp_IntegerOnly <BinOp_BwOr>
{
public:
	BinOp_BwOr ();

	virtual
	Type*
	getResultType (
		const Value& opValue1,
		const Value& opValue2
		)
	{
		return isFlagEnumOpType (opValue1, opValue2) ?
			opValue1.getType () :
			getArithmeticResultType (opValue1, opValue2);
	}

	virtual
	bool
	op (
		const Value& rawOpValue1,
		const Value& rawOpValue2,
		Value* resultValue
		);

	static
	int32_t
	constOpInt32 (
		int32_t opValue1,
		int32_t opValue2,
		bool isUnsigned
		)
	{
		return opValue1 | opValue2;
	}

	static
	int64_t
	constOpInt64 (
		int64_t opValue1,
		int64_t opValue2,
		bool isUnsigned
		)
	{
		return opValue1 | opValue2;
	}

	llvm::Value*
	llvmOpInt (
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue,
		bool isUnsigned
		);

	bool
	isFlagEnumOpType (
		const Value& opValue1,
		const Value& opValue2
		)
	{
		return opValue1.getType () == opValue2.getType () && isBitFlagEnumType (opValue1.getType ());
	}

};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class BinOp_BwXor: public BinOp_IntegerOnly <BinOp_BwXor>
{
public:
	BinOp_BwXor ()
	{
		m_opKind = BinOpKind_BwXor;
	}

	static
	int32_t
	constOpInt32 (
		int32_t opValue1,
		int32_t opValue2,
		bool isUnsigned
		)
	{
		return opValue1 ^ opValue2;
	}

	static
	int64_t
	constOpInt64 (
		int64_t opValue1,
		int64_t opValue2,
		bool isUnsigned
		)
	{
		return opValue1 ^ opValue2;
	}

	llvm::Value*
	llvmOpInt (
		const Value& opValue1,
		const Value& opValue2,
		Type* resultType,
		Value* resultValue,
		bool isUnsigned
		);
};

//..............................................................................

} // namespace ct
} // namespace jnc
