// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_BinOp.h"
#include "jnc_UnOp_Arithmetic.h"
#include "jnc_EnumType.h"

namespace jnc {

//.............................................................................

template <typename T>
class CBinOpT_Arithmetic: public CBinaryOperator
{
public:
	enum
	{
		IsIntegerOnly = false
	};

public:
	virtual
	CType*
	GetResultType (
		const CValue& OpValue1,
		const CValue& OpValue2
		)
	{
		return GetArithmeticResultType (OpValue1, OpValue2);
	}

	virtual
	bool
	Operator (
		const CValue& RawOpValue1,
		const CValue& RawOpValue2,
		CValue* pResultValue
		)
	{
		// BwOr overrides GetResultType, but here we need original one

		CType* pType = GetArithmeticResultType (RawOpValue1, RawOpValue2); 
		if (!pType)
			return false;

		CValue OpValue1;
		CValue OpValue2;

		bool Result = 
			CastOperator (m_pModule, RawOpValue1, pType, &OpValue1) &&
			CastOperator (m_pModule, RawOpValue2, pType, &OpValue2);
		
		if (!Result)
			return false;

		if (OpValue1.GetValueKind () == EValue_Const && OpValue2.GetValueKind () == EValue_Const)
		{
			EType TypeKind = pType->GetTypeKind ();
			switch (TypeKind)
			{
			case EType_Int32:
			case EType_Int32_u:
				pResultValue->SetConstInt32 (
					T::ConstOpInt32 (
						OpValue1.GetInt32 (), 
						OpValue2.GetInt32 (), 
						(pType->GetTypeKindFlags () & ETypeKindFlag_Unsigned) != 0
						), 
					pType
					);
				break;

			case EType_Int64:
			case EType_Int64_u:
				pResultValue->SetConstInt32 (
					T::ConstOpInt32 (
						OpValue1.GetInt32 (), 
						OpValue2.GetInt32 (), 
						(pType->GetTypeKindFlags () & ETypeKindFlag_Unsigned) != 0
						), 
					pType
					);
				break;

			case EType_Float:
				pResultValue->SetConstFloat (T::ConstOpFp32 (OpValue1.GetFloat (), OpValue2.GetFloat ()));
				break;

			case EType_Double:
				pResultValue->SetConstDouble (T::ConstOpFp64 (OpValue1.GetDouble (), OpValue2.GetDouble ()));
				break;

			default:
				ASSERT (false);
			}
		}
		else
		{
			EType TypeKind = pType->GetTypeKind ();
			switch (TypeKind)
			{
			case EType_Int32:
			case EType_Int32_u:
			case EType_Int64:
			case EType_Int64_u:
				static_cast <T*> (this)->LlvmOpInt (
					OpValue1, 
					OpValue2, 
					pType,
					pResultValue,
					(pType->GetTypeKindFlags () & ETypeKindFlag_Unsigned) != 0
					);
				break;

			case EType_Float:
			case EType_Double:
				static_cast <T*> (this)->LlvmOpFp (
					OpValue1, 
					OpValue2,
					pType,
					pResultValue
					);
				break;

			default:
				ASSERT (false);
			}

			if (!Result)
				return false;
		}

		return true;
	}

protected:
	CType*
	GetArithmeticResultType (
		const CValue& OpValue1,
		const CValue& OpValue2
		)
	{
		CType* pType = GetArithmeticOperatorResultType (OpValue1, OpValue2);
		if (!pType || T::IsIntegerOnly && !(pType->GetTypeKindFlags () & ETypeKindFlag_Integer))
		{
			SetOperatorError (OpValue1, OpValue2);
			return NULL;
		}

		return pType;
	}

};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename T>
class CBinOpT_IntegerOnly: public CBinOpT_Arithmetic <T>
{	
public:
	enum
	{
		IsIntegerOnly = true
	};

public:
	static
	float
	ConstOpFp32 (
		float OpValue1,
		float OpValue2
		)
	{
		return 0;
	}

	static
	double
	ConstOpFp64 (
		double OpValue1,
		double OpValue2
		)
	{
		return 0;
	}

	llvm::Value*
	LlvmOpFp (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CType* pResultType,
		CValue* pResultValue
		)
	{
		ASSERT (false);
		return NULL;
	}
};

//.............................................................................

class CBinOp_Add: public CBinOpT_Arithmetic <CBinOp_Add>
{	
public:
	CBinOp_Add ()
	{
		m_OpKind = EBinOp_Add;
	}

	virtual
	bool
	Operator (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CValue* pResultValue
		);

	static
	int32_t
	ConstOpInt32 (
		int32_t OpValue1,
		int32_t OpValue2,
		bool IsUnsigned
		) 
	{
		return OpValue1 + OpValue2;
	}

	static
	int64_t
	ConstOpInt64 (
		int64_t OpValue1,
		int64_t OpValue2,
		bool IsUnsigned
		)
	{
		return OpValue1 + OpValue2;
	}

	static
	float
	ConstOpFp32 (
		float OpValue1,
		float OpValue2
		)
	{
		return OpValue1 + OpValue2;
	}

	static
	double
	ConstOpFp64 (
		double OpValue1,
		double OpValue2
		)
	{
		return OpValue1 + OpValue2;
	}

	llvm::Value*
	LlvmOpInt (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CType* pResultType,
		CValue* pResultValue,
		bool IsUnsigned
		);

	llvm::Value*
	LlvmOpFp (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CType* pResultType,
		CValue* pResultValue
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CBinOp_Sub: public CBinOpT_Arithmetic <CBinOp_Sub>
{	
public:
	CBinOp_Sub ()
	{
		m_OpKind = EBinOp_Sub;
	}

	virtual
	bool
	Operator (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CValue* pResultValue
		);

	static
	int32_t
	ConstOpInt32 (
		int32_t OpValue1,
		int32_t OpValue2,
		bool IsUnsigned
		) 
	{
		return OpValue1 - OpValue2;
	}

	static
	int64_t
	ConstOpInt64 (
		int64_t OpValue1,
		int64_t OpValue2,
		bool IsUnsigned
		)
	{
		return OpValue1 - OpValue2;
	}

	static
	float
	ConstOpFp32 (
		float OpValue1,
		float OpValue2
		)
	{
		return OpValue1 - OpValue2;
	}

	static
	double
	ConstOpFp64 (
		double OpValue1,
		double OpValue2
		)
	{
		return OpValue1 - OpValue2;
	}

	llvm::Value*
	LlvmOpInt (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CType* pResultType,
		CValue* pResultValue,
		bool IsUnsigned
		);

	llvm::Value*
	LlvmOpFp (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CType* pResultType,
		CValue* pResultValue
		);
};

//.............................................................................

class CBinOp_Mul: public CBinOpT_Arithmetic <CBinOp_Mul>
{	
public:
	CBinOp_Mul ()
	{
		m_OpKind = EBinOp_Mul;
	}

	static
	int32_t
	ConstOpInt32 (
		int32_t OpValue1,
		int32_t OpValue2,
		bool IsUnsigned
		) 
	{
		return OpValue1 * OpValue2;
	}

	static
	int64_t
	ConstOpInt64 (
		int64_t OpValue1,
		int64_t OpValue2,
		bool IsUnsigned
		)
	{
		return OpValue1 * OpValue2;
	}

	static
	float
	ConstOpFp32 (
		float OpValue1,
		float OpValue2
		)
	{
		return OpValue1 * OpValue2;
	}

	static
	double
	ConstOpFp64 (
		double OpValue1,
		double OpValue2
		)
	{
		return OpValue1 * OpValue2;
	}

	llvm::Value*
	LlvmOpInt (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CType* pResultType,
		CValue* pResultValue,
		bool IsUnsigned
		);

	llvm::Value*
	LlvmOpFp (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CType* pResultType,
		CValue* pResultValue
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CBinOp_Div: public CBinOpT_Arithmetic <CBinOp_Div>
{	
public:
	CBinOp_Div ()
	{
		m_OpKind = EBinOp_Div;
	}

	static
	int32_t
	ConstOpInt32 (
		int32_t OpValue1,
		int32_t OpValue2,
		bool IsUnsigned
		) 
	{
		return IsUnsigned ? (uint32_t) OpValue1 / (uint32_t) OpValue2 : OpValue1 / OpValue2;
	}

	static
	int64_t
	ConstOpInt64 (
		int64_t OpValue1,
		int64_t OpValue2,
		bool IsUnsigned
		)
	{
		return IsUnsigned ? (uint64_t) OpValue1 / (uint64_t) OpValue2 : OpValue1 / OpValue2;
	}

	static
	float
	ConstOpFp32 (
		float OpValue1,
		float OpValue2
		)
	{
		return OpValue1 / OpValue2;
	}

	static
	double
	ConstOpFp64 (
		double OpValue1,
		double OpValue2
		)
	{
		return OpValue1 / OpValue2;
	}

	llvm::Value*
	LlvmOpInt (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CType* pResultType,
		CValue* pResultValue,
		bool IsUnsigned
		);

	llvm::Value*
	LlvmOpFp (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CType* pResultType,
		CValue* pResultValue
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CBinOp_Mod: public CBinOpT_IntegerOnly <CBinOp_Mod>
{	
public:
	CBinOp_Mod ()
	{
		m_OpKind = EBinOp_Mod;
	}

	static
	int32_t
	ConstOpInt32 (
		int32_t OpValue1,
		int32_t OpValue2,
		bool IsUnsigned
		) 
	{
		return IsUnsigned ? (uint32_t) OpValue1 % (uint32_t) OpValue2 : OpValue1 % OpValue2;
	}

	static
	int64_t
	ConstOpInt64 (
		int64_t OpValue1,
		int64_t OpValue2,
		bool IsUnsigned
		)
	{
		return IsUnsigned ? (uint64_t) OpValue1 % (uint64_t) OpValue2 : OpValue1 % OpValue2;
	}

	llvm::Value*
	LlvmOpInt (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CType* pResultType,
		CValue* pResultValue,
		bool IsUnsigned
		);
};

//.............................................................................

class CBinOp_Shl: public CBinOpT_IntegerOnly <CBinOp_Shl>
{	
public:
	CBinOp_Shl ()
	{
		m_OpKind = EBinOp_Shl;
	}

	static
	int32_t
	ConstOpInt32 (
		int32_t OpValue1,
		int32_t OpValue2,
		bool IsUnsigned
		) 
	{
		return OpValue1 << OpValue2;
	}

	static
	int64_t
	ConstOpInt64 (
		int64_t OpValue1,
		int64_t OpValue2,
		bool IsUnsigned
		)
	{
		return OpValue1 << OpValue2;
	}

	llvm::Value*
	LlvmOpInt (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CType* pResultType,
		CValue* pResultValue,
		bool IsUnsigned
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CBinOp_Shr: public CBinOpT_IntegerOnly <CBinOp_Shr>
{	
public:
	CBinOp_Shr ()
	{
		m_OpKind = EBinOp_Shr;
	}

	static
	int32_t
	ConstOpInt32 (
		int32_t OpValue1,
		int32_t OpValue2,
		bool IsUnsigned
		) 
	{
		return OpValue1 >> OpValue2;
	}

	static
	int64_t
	ConstOpInt64 (
		int64_t OpValue1,
		int64_t OpValue2,
		bool IsUnsigned
		)
	{
		return OpValue1 >> OpValue2;
	}

	llvm::Value*
	LlvmOpInt (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CType* pResultType,
		CValue* pResultValue,
		bool IsUnsigned
		);
};

//.............................................................................

class CBinOp_BwAnd: public CBinOpT_IntegerOnly <CBinOp_BwAnd>
{	
public:
	CBinOp_BwAnd ();

	virtual
	CType*
	GetResultType (
		const CValue& OpValue1,
		const CValue& OpValue2
		)
	{
		return 
			IsFlagEnumType (OpValue1.GetType ()) ? OpValue1.GetType () :
			IsFlagEnumType (OpValue2.GetType ()) ? OpValue2.GetType () :
			GetArithmeticResultType (OpValue1, OpValue2);
	}

	virtual
	bool
	Operator (
		const CValue& RawOpValue1,
		const CValue& RawOpValue2,
		CValue* pResultValue
		);

	static
	int32_t
	ConstOpInt32 (
		int32_t OpValue1,
		int32_t OpValue2,
		bool IsUnsigned
		) 
	{
		return OpValue1 & OpValue2;
	}

	static
	int64_t
	ConstOpInt64 (
		int64_t OpValue1,
		int64_t OpValue2,
		bool IsUnsigned
		)
	{
		return OpValue1 & OpValue2;
	}

	llvm::Value*
	LlvmOpInt (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CType* pResultType,
		CValue* pResultValue,
		bool IsUnsigned
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CBinOp_BwOr: public CBinOpT_IntegerOnly <CBinOp_BwOr>
{	
public:
	CBinOp_BwOr ();

	virtual
	CType*
	GetResultType (
		const CValue& OpValue1,
		const CValue& OpValue2
		)
	{
		return IsFlagEnumOpType (OpValue1, OpValue2) ? 
			OpValue1.GetType () : 
			GetArithmeticResultType (OpValue1, OpValue2);
	}

	virtual
	bool
	Operator (
		const CValue& RawOpValue1,
		const CValue& RawOpValue2,
		CValue* pResultValue
		);

	static
	int32_t
	ConstOpInt32 (
		int32_t OpValue1,
		int32_t OpValue2,
		bool IsUnsigned
		) 
	{
		return OpValue1 | OpValue2;
	}

	static
	int64_t
	ConstOpInt64 (
		int64_t OpValue1,
		int64_t OpValue2,
		bool IsUnsigned
		)
	{
		return OpValue1 | OpValue2;
	}

	llvm::Value*
	LlvmOpInt (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CType* pResultType,
		CValue* pResultValue,
		bool IsUnsigned
		);

	bool
	IsFlagEnumOpType (
		const CValue& OpValue1,
		const CValue& OpValue2
		)
	{
		return OpValue1.GetType () == OpValue2.GetType () && IsFlagEnumType (OpValue1.GetType ());
	}

};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CBinOp_BwXor: public CBinOpT_IntegerOnly <CBinOp_BwXor>
{	
public:
	CBinOp_BwXor ()
	{
		m_OpKind = EBinOp_BwXor;
	}

	static
	int32_t
	ConstOpInt32 (
		int32_t OpValue1,
		int32_t OpValue2,
		bool IsUnsigned
		) 
	{
		return OpValue1 ^ OpValue2;
	}

	static
	int64_t
	ConstOpInt64 (
		int64_t OpValue1,
		int64_t OpValue2,
		bool IsUnsigned
		)
	{
		return OpValue1 ^ OpValue2;
	}

	llvm::Value*
	LlvmOpInt (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CType* pResultType,
		CValue* pResultValue,
		bool IsUnsigned
		);
};

//.............................................................................

} // namespace jnc {
