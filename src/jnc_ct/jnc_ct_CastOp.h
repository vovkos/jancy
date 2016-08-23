// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_UnOp.h"

namespace jnc {
namespace ct {

class FunctionType;
class FunctionPtrType;

//.............................................................................

// ordered from the worst to the best

enum CastKind
{
	CastKind_None,
	CastKind_Dynamic,
	CastKind_Explicit,
	CastKind_ImplicitCrossFamily,
	CastKind_Implicit,
	CastKind_Identitiy,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

err::Error
setCastError (
	const Value& opValue,
	Type* type,
	CastKind castKind = CastKind_None
	);

err::Error
setUnsafeCastError (
	Type* srcType,
	Type* dstType
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// these are sometimes needed for inlining casts before jnc_Module.h is include

bool
castOperator (
	Module* module,
	const Value& opValue,
	Type* type,
	Value* resultValue
	);

JNC_INLINE
bool
castOperator (
	Module* module,
	Value* opValue,
	Type* type
	)
{
	return castOperator (module, *opValue, type, opValue);
}

//.............................................................................

class CastOperator
{	
	friend class OperatorMgr;

protected:
	Module* m_module;
	uint_t m_opFlags;

public:
	CastOperator ();

	Module*
	getModule ()
	{
		return m_module;
	}

	int 
	getOpFlags ()
	{
		return m_opFlags;
	}

	virtual
	CastKind
	getCastKind (
		const Value& opValue,
		Type* type
		) = 0;

	virtual
	bool
	constCast (
		const Value& opValue,
		Type* type,
		void* dst
		)
	{
		return false;
	}

	virtual
	bool
	llvmCast (
		const Value& opValue,
		Type* type,
		Value* resultValue
		) = 0;

	bool
	cast (
		const Value& opValue,
		Type* type,
		Value* resultValue
		);
};

//.............................................................................

// fail by default

class Cast_Default: public CastOperator
{
public:
	virtual
	CastKind
	getCastKind (
		const Value& opValue,
		Type* type
		)
	{
		return CastKind_None;
	}

	virtual
	bool
	llvmCast (
		const Value& opValue,
		Type* type,
		Value* resultValue
		)
	{
		setCastError (opValue, type);
		return false;
	}
};

//.............................................................................

// simple copy

class Cast_Copy: public CastOperator
{
public:
	virtual
	CastKind
	getCastKind (
		const Value& opValue,
		Type* type
		)
	{
		return opValue.getType ()->cmp (type) == 0 ? CastKind_Identitiy : CastKind_Implicit;
	}

	virtual
	bool
	constCast (
		const Value& opValue,
		Type* type,
		void* dst
		);

	virtual
	bool
	llvmCast (
		const Value& opValue,
		Type* type,
		Value* resultValue
		);
};

//.............................................................................

// master cast chooses particular implementation

class Cast_Master: public CastOperator
{
public:
	virtual
	CastKind
	getCastKind (
		const Value& opValue,
		Type* type
		);

	virtual
	bool
	constCast (
		const Value& opValue,
		Type* type,
		void* dst
		);

	virtual
	bool
	llvmCast (
		const Value& opValue,
		Type* type,
		Value* resultValue
		);

	virtual
	CastOperator*
	getCastOperator (
		const Value& opValue,
		Type* type
		) = 0;
};

//.............................................................................

// master cast capable of performing superposition of casts

class Cast_SuperMaster: public CastOperator
{
public:
	virtual
	CastKind
	getCastKind (
		const Value& opValue,
		Type* type
		);

	virtual
	bool
	constCast (
		const Value& opValue,
		Type* type,
		void* dst
		);

	virtual
	bool
	llvmCast (
		const Value& opValue,
		Type* type,
		Value* resultValue
		);

	virtual
	bool
	getCastOperators (
		const Value& opValue,
		Type* type,
		CastOperator** operator1,
		CastOperator** operator2,
		Type** intermediateType
		) = 0;
};

//.............................................................................

} // namespace ct
} // namespace jnc

