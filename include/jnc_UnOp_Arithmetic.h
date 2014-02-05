// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_UnOp.h"
#include "jnc_CastOp.h"

namespace jnc {

//.............................................................................

CType*
GetArithmeticOperatorResultType (CType* pOpType);

inline
CType*
GetArithmeticOperatorResultType (const CValue& OpValue)
{
	return GetArithmeticOperatorResultType (OpValue.GetType ());
}

inline 
CType*
GetArithmeticOperatorResultType (
	CType* pOpType1,
	CType* pOpType2
	)
{
	return GetArithmeticOperatorResultType (
		pOpType1->GetTypeKind () > pOpType2->GetTypeKind () ? 
			pOpType1 : 
			pOpType2
		);
}

inline
CType*
GetArithmeticOperatorResultType (
	const CValue& OpValue1,
	const CValue& OpValue2
	)
{
	return GetArithmeticOperatorResultType (OpValue1.GetType (), OpValue2.GetType ());
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename T>
class CUnOpT_Arithmetic: public CUnaryOperator
{
public:
	enum
	{
		IsIntegerOnly = false
	};

public:
	virtual
	CType*
	GetResultType (const CValue& OpValue)
	{
		CType* pType = GetArithmeticOperatorResultType (OpValue);
		if (!pType || T::IsIntegerOnly && !(pType->GetTypeKindFlags () & ETypeKindFlag_Integer))
		{
			SetOperatorError (OpValue);
			return NULL;
		}

		return pType;
	}

	virtual
	bool
	Operator (
		const CValue& RawOpValue,
		CValue* pResultValue
		)
	{
		CType* pType = GetResultType (RawOpValue);

		CValue OpValue;
		bool Result = CastOperator (m_pModule, RawOpValue, pType, &OpValue);
		if (!Result)
			return false;

		if (OpValue.GetValueKind () == EValue_Const)
		{
			EType TypeKind = pType->GetTypeKind ();
			switch (TypeKind)
			{
			case EType_Int32:
			case EType_Int32_u:
				pResultValue->SetConstInt32 (T::ConstOpInt32 (OpValue.GetInt32 ()), pType);
				break;

			case EType_Int64:
			case EType_Int64_u:
				pResultValue->SetConstInt32 (T::ConstOpInt32 (OpValue.GetInt32 ()), pType);
				break;

			case EType_Float:
				pResultValue->SetConstFloat (T::ConstOpFp32 (OpValue.GetFloat ()));
				break;

			case EType_Double:
				pResultValue->SetConstDouble (T::ConstOpFp64 (OpValue.GetDouble ()));
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
				static_cast <T*> (this)->LlvmOpInt (OpValue, pType, pResultValue);
				break;

			case EType_Float:
			case EType_Double:
				static_cast <T*> (this)->LlvmOpFp (OpValue, pType, pResultValue);
				break;

			default:
				ASSERT (false);
			}

			if (!Result)
				return false;
		}

		return true;
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename T>
class CUnOpT_IntegerOnly: public CUnOpT_Arithmetic <T>
{
public:
	enum
	{
		IsIntegerOnly = true
	};

public:
	static
	float
	ConstOpFp32 (float OpValue)
	{
		return 0;
	}

	static
	double
	ConstOpFp64 (double OpValue)
	{
		return 0;
	}

	llvm::Value*
	LlvmOpFp (
		const CValue& OpValue,
		CType* pResultType,
		CValue* pResultValue
		)
	{
		ASSERT (false);
		return NULL;
	}
};

//.............................................................................

class CUnOp_Plus: public CUnOpT_Arithmetic <CUnOp_Plus>
{
public:
	CUnOp_Plus ()
	{
		m_OpKind = EUnOp_Plus;
	}

	static
	int32_t
	ConstOpInt32 (int32_t OpValue)
	{
		return +OpValue;
	}

	static
	int64_t
	ConstOpInt64 (int64_t OpValue)
	{
		return +OpValue;
	}

	static
	float
	ConstOpFp32 (float OpValue)
	{
		return +OpValue;
	}

	static
	double
	ConstOpFp64 (double OpValue)
	{
		return +OpValue;
	}

	static
	llvm::Value*
	LlvmOpInt (
		const CValue& OpValue,
		CType* pResultType,
		CValue* pResultValue
		)
	{
		*pResultValue = OpValue;
		return pResultValue->GetLlvmValue ();
	}

	static
	llvm::Value*
	LlvmOpFp (
		const CValue& OpValue,
		CType* pResultType,
		CValue* pResultValue
		)
	{
		*pResultValue = OpValue;
		return pResultValue->GetLlvmValue ();
	}
};

//.............................................................................

class CUnOp_Minus: public CUnOpT_Arithmetic <CUnOp_Minus>
{
public:
	CUnOp_Minus ()
	{
		m_OpKind = EUnOp_Minus;
	}

	static
	int32_t
	ConstOpInt32 (int32_t OpValue)
	{
		return -OpValue;
	}

	static
	int64_t
	ConstOpInt64 (int64_t OpValue)
	{
		return -OpValue;
	}

	static
	float
	ConstOpFp32 (float OpValue)
	{
		return -OpValue;
	}

	static
	double
	ConstOpFp64 (double OpValue)
	{
		return -OpValue;
	}

	llvm::Value*
	LlvmOpInt (
		const CValue& OpValue,
		CType* pResultType,
		CValue* pResultValue
		);

	llvm::Value*
	LlvmOpFp (
		const CValue& OpValue,
		CType* pResultType,
		CValue* pResultValue
		);
};

//.............................................................................

class CUnOp_BwNot: public CUnOpT_IntegerOnly <CUnOp_BwNot>
{
public:
	CUnOp_BwNot ()
	{
		m_OpKind = EUnOp_BwNot;
	};

	static
	int32_t
	ConstOpInt32 (int32_t OpValue)
	{
		return ~OpValue;
	}

	static
	int64_t
	ConstOpInt64 (int64_t OpValue)
	{
		return ~OpValue;
	}

	llvm::Value*
	LlvmOpInt (
		const CValue& OpValue,
		CType* pResultType,
		CValue* pResultValue
		);

};

//.............................................................................

} // namespace jnc {
