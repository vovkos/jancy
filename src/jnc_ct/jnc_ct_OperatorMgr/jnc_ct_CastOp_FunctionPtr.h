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

#include "jnc_ct_CastOp.h"
#include "jnc_ct_FunctionPtrType.h"

namespace jnc {
namespace ct {

//..............................................................................

class Cast_FunctionPtr_FromMulticast: public CastOperator
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
	llvmCast (
		const Value& opValue,
		Type* type,
		Value* resultValue
		);
};

//..............................................................................

class Cast_FunctionPtr_FromDataPtr: public CastOperator
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
	llvmCast (
		const Value& opValue,
		Type* type,
		Value* resultValue
		);
};

//..............................................................................

class Cast_FunctionPtr_Base: public CastOperator
{
public:
	virtual
	CastKind
	getCastKind (
		const Value& opValue,
		Type* type
		);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_FunctionPtr_FromFat: public Cast_FunctionPtr_Base
{
public:
	virtual
	bool
	llvmCast (
		const Value& opValue,
		Type* type,
		Value* resultValue
		);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_FunctionPtr_Weak2Normal: public Cast_FunctionPtr_Base
{
public:
	virtual
	bool
	llvmCast (
		const Value& opValue,
		Type* type,
		Value* resultValue
		);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_FunctionPtr_Thin2Fat: public Cast_FunctionPtr_Base
{
public:
	virtual
	bool
	llvmCast (
		const Value& opValue,
		Type* type,
		Value* resultValue
		);

protected:
	bool
	llvmCast_NoThunkSimpleClosure (
		const Value& opValue,
		const Value& simpleClosureObjValue,
		FunctionType* srcFunctionType,
		FunctionPtrType* dstPtrType,
		Value* resultValue
		);

	bool
	llvmCast_DirectThunkNoClosure (
		Function* function,
		FunctionPtrType* dstPtrType,
		Value* resultValue
		);

	bool
	llvmCast_DirectThunkSimpleClosure (
		Function* function,
		const Value& simpleClosureObjValue,
		FunctionPtrType* dstPtrType,
		Value* resultValue
		);

	bool
	llvmCast_FullClosure (
		const Value& opValue,
		FunctionType* srcFunctionType,
		FunctionPtrType* dstPtrType,
		Value* resultValue
		);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_FunctionPtr_Thin2Thin: public Cast_FunctionPtr_Base
{
public:
	virtual
	bool
	llvmCast (
		const Value& opValue,
		Type* type,
		Value* resultValue
		);
};

//..............................................................................

class Cast_FunctionPtr: public Cast_Master
{
protected:
	Cast_FunctionPtr_FromMulticast m_fromMulticast;
	Cast_FunctionPtr_FromDataPtr m_fromDataPtr;
	Cast_FunctionPtr_FromFat m_fromFat;
	Cast_FunctionPtr_Weak2Normal m_weak2Normal;
	Cast_FunctionPtr_Thin2Fat m_thin2Fat;
	Cast_FunctionPtr_Thin2Thin m_thin2Thin;

	CastOperator* m_operatorTable [FunctionPtrTypeKind__Count] [FunctionPtrTypeKind__Count];

public:
	Cast_FunctionPtr ();

	virtual
	CastOperator*
	getCastOperator (
		const Value& opValue,
		Type* type
		);
};

//..............................................................................

// function ref (UnOpKind_Indir => function ptr cast => UnOpKind_Addr)

class Cast_FunctionRef: public CastOperator
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
	llvmCast (
		const Value& opValue,
		Type* type,
		Value* resultValue
		);
};

//..............................................................................

} // namespace ct
} // namespace jnc
