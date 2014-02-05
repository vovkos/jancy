// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_BinOp.h"
#include "jnc_UnOp_Arithmetic.h"

namespace jnc {

//.............................................................................

CType*
GetPtrCmpOperatorOperandType (
	const CValue& OpValue1,
	const CValue& OpValue2
	);

//.............................................................................

template <typename T>
class CBinOpT_Cmp: public CBinaryOperator
{
public:
	virtual
	CType*
	GetResultType (
		const CValue& OpValue1,
		const CValue& OpValue2
		)
	{
		return GetSimpleType (m_pModule, EType_Bool);
	}

	virtual
	bool
	Operator (
		const CValue& RawOpValue1,
		const CValue& RawOpValue2,
		CValue* pResultValue
		)
	{
		CType* pType;

		if ((RawOpValue1.GetType ()->GetTypeKindFlags () & ETypeKindFlag_Ptr) ||
			(RawOpValue2.GetType ()->GetTypeKindFlags () & ETypeKindFlag_Ptr))
		{
			pType = GetPtrCmpOperatorOperandType (RawOpValue1, RawOpValue2);
		}
		else
		{
			pType = GetArithmeticOperatorResultType (RawOpValue1, RawOpValue2);
		}

		if (!pType)
		{
			SetOperatorError (RawOpValue1, RawOpValue2);
			return false;
		}

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
				pResultValue->SetConstBool (
					T::ConstOpInt32 (
						OpValue1.GetInt32 (), 
						OpValue2.GetInt32 (), 
						(pType->GetTypeKindFlags () & ETypeKindFlag_Unsigned) != 0
						)
					);
				break;

			case EType_Int64:
			case EType_Int64_u:
				pResultValue->SetConstBool (
					T::ConstOpInt32 (
						OpValue1.GetInt32 (), 
						OpValue2.GetInt32 (), 
						(pType->GetTypeKindFlags () & ETypeKindFlag_Unsigned) != 0
						)
					);
				break;

			case EType_Float:
				pResultValue->SetConstBool (T::ConstOpFp32 (OpValue1.GetFloat (), OpValue2.GetFloat ()));
				break;

			case EType_Double:
				pResultValue->SetConstBool (T::ConstOpFp64 (OpValue1.GetDouble (), OpValue2.GetDouble ()));
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
					pResultValue,
					(pType->GetTypeKindFlags () & ETypeKindFlag_Unsigned) != 0
					);
				break;

			case EType_Float:
			case EType_Double:
				static_cast <T*> (this)->LlvmOpFp (
					OpValue1, 
					OpValue2,
					pResultValue
					);
				break;

			default:
				ASSERT (false);
			}
		}

		return true;
	}
};

//.............................................................................

class CBinOp_Eq: public CBinOpT_Cmp <CBinOp_Eq>
{	
public:
	CBinOp_Eq ()
	{
		m_OpKind = EBinOp_Eq;
	}

	static
	bool
	ConstOpInt32 (
		int32_t OpValue1,
		int32_t OpValue2,
		bool IsUnsigned
		) 
	{
		return OpValue1 == OpValue2;
	}

	static
	bool
	ConstOpInt64 (
		int64_t OpValue1,
		int64_t OpValue2,
		bool IsUnsigned
		)
	{
		return OpValue1 == OpValue2;
	}

	static
	bool
	ConstOpFp32 (
		float OpValue1,
		float OpValue2
		)
	{
		return OpValue1 == OpValue2;
	}

	static
	bool
	ConstOpFp64 (
		double OpValue1,
		double OpValue2
		)
	{
		return OpValue1 == OpValue2;
	}

	llvm::Value*
	LlvmOpInt (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CValue* pResultValue,
		bool IsUnsigned
		);

	llvm::Value*
	LlvmOpFp (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CValue* pResultValue
		);
};

//.............................................................................

class CBinOp_Ne: public CBinOpT_Cmp <CBinOp_Ne>
{	
public:
	CBinOp_Ne ()
	{
		m_OpKind = EBinOp_Ne;
	}

	static
	bool
	ConstOpInt32 (
		int32_t OpValue1,
		int32_t OpValue2,
		bool IsUnsigned
		) 
	{
		return OpValue1 != OpValue2;
	}

	static
	bool
	ConstOpInt64 (
		int64_t OpValue1,
		int64_t OpValue2,
		bool IsUnsigned
		)
	{
		return OpValue1 != OpValue2;
	}

	static
	bool
	ConstOpFp32 (
		float OpValue1,
		float OpValue2
		)
	{
		return OpValue1 != OpValue2;
	}

	static
	bool
	ConstOpFp64 (
		double OpValue1,
		double OpValue2
		)
	{
		return OpValue1 != OpValue2;
	}

	llvm::Value*
	LlvmOpInt (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CValue* pResultValue,
		bool IsUnsigned
		);

	llvm::Value*
	LlvmOpFp (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CValue* pResultValue
		);
};

//.............................................................................

class CBinOp_Lt: public CBinOpT_Cmp <CBinOp_Lt>
{	
public:
	CBinOp_Lt ()
	{
		m_OpKind = EBinOp_Lt;
	}

	static
	bool
	ConstOpInt32 (
		int32_t OpValue1,
		int32_t OpValue2,
		bool IsUnsigned
		) 
	{
		return IsUnsigned ? (uint32_t) OpValue1 < (uint32_t) OpValue2 : OpValue1 < OpValue2;
	}

	static
	bool
	ConstOpInt64 (
		int64_t OpValue1,
		int64_t OpValue2,
		bool IsUnsigned
		)
	{
		return IsUnsigned ? (uint64_t) OpValue1 < (uint64_t) OpValue2 : OpValue1 < OpValue2;
	}

	static
	bool
	ConstOpFp32 (
		float OpValue1,
		float OpValue2
		)
	{
		return OpValue1 < OpValue2;
	}

	static
	bool
	ConstOpFp64 (
		double OpValue1,
		double OpValue2
		)
	{
		return OpValue1 < OpValue2;
	}

	llvm::Value*
	LlvmOpInt (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CValue* pResultValue,
		bool IsUnsigned
		);

	llvm::Value*
	LlvmOpFp (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CValue* pResultValue
		);
};

//.............................................................................

class CBinOp_Le: public CBinOpT_Cmp <CBinOp_Le>
{	
public:
	CBinOp_Le ()
	{
		m_OpKind = EBinOp_Le;
	}

	static
	bool
	ConstOpInt32 (
		int32_t OpValue1,
		int32_t OpValue2,
		bool IsUnsigned
		) 
	{
		return IsUnsigned ? (uint32_t) OpValue1 <= (uint32_t) OpValue2 : OpValue1 <= OpValue2;
	}

	static
	bool
	ConstOpInt64 (
		int64_t OpValue1,
		int64_t OpValue2,
		bool IsUnsigned
		)
	{
		return IsUnsigned ? (uint64_t) OpValue1 <= (uint64_t) OpValue2 : OpValue1 <= OpValue2;
	}

	static
	bool
	ConstOpFp32 (
		float OpValue1,
		float OpValue2
		)
	{
		return OpValue1 <= OpValue2;
	}

	static
	bool
	ConstOpFp64 (
		double OpValue1,
		double OpValue2
		)
	{
		return OpValue1 <= OpValue2;
	}

	llvm::Value*
	LlvmOpInt (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CValue* pResultValue,
		bool IsUnsigned
		);

	llvm::Value*
	LlvmOpFp (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CValue* pResultValue
		);
};

//.............................................................................

class CBinOp_Gt: public CBinOpT_Cmp <CBinOp_Gt>
{	
public:
	CBinOp_Gt ()
	{
		m_OpKind = EBinOp_Gt;
	}

	static
	bool
	ConstOpInt32 (
		int32_t OpValue1,
		int32_t OpValue2,
		bool IsUnsigned
		) 
	{
		return IsUnsigned ? (uint32_t) OpValue1 > (uint32_t) OpValue2 : OpValue1 > OpValue2;
	}

	static
	bool
	ConstOpInt64 (
		int64_t OpValue1,
		int64_t OpValue2,
		bool IsUnsigned
		)
	{
		return IsUnsigned ? (uint64_t) OpValue1 > (uint64_t) OpValue2 : OpValue1 > OpValue2;
	}

	static
	bool
	ConstOpFp32 (
		float OpValue1,
		float OpValue2
		)
	{
		return OpValue1 > OpValue2;
	}

	static
	bool
	ConstOpFp64 (
		double OpValue1,
		double OpValue2
		)
	{
		return OpValue1 > OpValue2;
	}

	llvm::Value*
	LlvmOpInt (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CValue* pResultValue,
		bool IsUnsigned
		);

	llvm::Value*
	LlvmOpFp (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CValue* pResultValue
		);
};

//.............................................................................

class CBinOp_Ge: public CBinOpT_Cmp <CBinOp_Ge>
{	
public:
	CBinOp_Ge ()
	{
		m_OpKind = EBinOp_Ge;
	}

	static
	bool
	ConstOpInt32 (
		int32_t OpValue1,
		int32_t OpValue2,
		bool IsUnsigned
		) 
	{
		return IsUnsigned ? (uint32_t) OpValue1 >= (uint32_t) OpValue2 : OpValue1 >= OpValue2;
	}

	static
	bool
	ConstOpInt64 (
		int64_t OpValue1,
		int64_t OpValue2,
		bool IsUnsigned
		)
	{
		return IsUnsigned ? (uint64_t) OpValue1 >= (uint64_t) OpValue2 : OpValue1 >= OpValue2;
	}

	static
	bool
	ConstOpFp32 (
		float OpValue1,
		float OpValue2
		)
	{
		return OpValue1 >= OpValue2;
	}

	static
	bool
	ConstOpFp64 (
		double OpValue1,
		double OpValue2
		)
	{
		return OpValue1 >= OpValue2;
	}

	llvm::Value*
	LlvmOpInt (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CValue* pResultValue,
		bool IsUnsigned
		);

	llvm::Value*
	LlvmOpFp (
		const CValue& OpValue1,
		const CValue& OpValue2,
		CValue* pResultValue
		);
};

//.............................................................................

} // namespace jnc {
