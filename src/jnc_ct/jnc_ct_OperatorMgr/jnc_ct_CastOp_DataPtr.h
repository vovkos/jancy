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

namespace jnc {
namespace ct {

class BaseTypeCoord;

//..............................................................................

// array -> data ptr

class Cast_DataPtr_FromArray: public CastOperator {
public:
	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
	);

	virtual
	bool
	cast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);

	virtual
	bool
	constCast(
		const Value& opValue,
		Type* type,
		void* dst
	);

	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);
};

//..............................................................................

// string -> data ptr

class Cast_DataPtr_FromString: public CastOperator {
public:
	Cast_DataPtr_FromString() {
		m_opFlags = OpFlag_KeepStringRef;
	}

	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
	);

	virtual
	bool
	constCast(
		const Value& opValue,
		Type* type,
		void* dst
	);

	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);
};

//..............................................................................

// rvalue -> const data ptr

class Cast_DataPtr_FromRvalue: public CastOperator {
public:
	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
	) {
		return CastKind_ImplicitCrossFamily; // to avoid ambiguity with "proper" ptr casts
	}

	virtual
	bool
	constCast(
		const Value& opValue,
		Type* type,
		void* dst
	);

	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);
};

//..............................................................................

// class ptr -> data ptr

class Cast_DataPtr_FromClassPtr: public CastOperator {
public:
	Cast_DataPtr_FromClassPtr() {
		m_opFlags = OpFlag_EnsurePtrTargetLayout;
	}

	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
	);

	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);
};

//..............................................................................

// function ptr -> data ptr

class Cast_DataPtr_FromFunctionPtr: public CastOperator {
public:
	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
	);

	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);
};

//..............................................................................

// property ptr -> data ptr

class Cast_DataPtr_FromPropertyPtr: public CastOperator {
public:
	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
	);

	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);
};

//..............................................................................

// data ptr -> data ptr

class Cast_DataPtr_Base: public CastOperator {
public:
	Cast_DataPtr_Base() {
		m_opFlags = OpFlag_KeepDerivableRef | OpFlag_EnsurePtrTargetLayout;
	}

	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
	);

protected:
	size_t
	getOffset(
		DataPtrType* srcType,
		DataPtrType* dstType,
		BaseTypeCoord* coord
	);

	bool
	getOffsetUnsafePtrValue(
		const Value& ptrValue,
		DataPtrType* srcType,
		DataPtrType* dstType,
		bool isFat,
		Value* resultValue
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_DataPtr_Normal2Normal: public Cast_DataPtr_Base {
public:
	virtual
	bool
	constCast(
		const Value& opValue,
		Type* type,
		void* dst
	);

	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_DataPtr_Lean2Normal: public Cast_DataPtr_Base {
public:
	virtual
	bool
	constCast(
		const Value& opValue,
		Type* type,
		void* dst
	);

	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_DataPtr_Normal2Thin: public Cast_DataPtr_Base {
public:
	virtual
	bool
	constCast(
		const Value& opValue,
		Type* type,
		void* dst
	);

	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_DataPtr_Lean2Thin: public Cast_DataPtr_Base {
public:
	virtual
	bool
	constCast(
		const Value& opValue,
		Type* type,
		void* dst
	) {
		ASSERT(false); // there are no lean pointer constants
		return true;
	}

	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_DataPtr_Thin2Thin: public Cast_DataPtr_Lean2Thin {
public:
	virtual
	bool
	constCast(
		const Value& opValue,
		Type* type,
		void* dst
	);

	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);
};

//..............................................................................

// data ptr master cast

class Cast_DataPtr: public Cast_Master {
protected:
	Cast_DataPtr_FromArray m_fromArray;
	Cast_DataPtr_FromString m_fromString;
	Cast_DataPtr_FromRvalue m_fromRvalue;
	Cast_DataPtr_FromClassPtr m_fromClassPtr;
	Cast_DataPtr_FromFunctionPtr m_fromFunctionPtr;
	Cast_DataPtr_FromPropertyPtr m_fromPropertyPtr;
	Cast_DataPtr_Normal2Normal m_normal2Normal;
	Cast_DataPtr_Lean2Normal m_lean2Normal;
	Cast_DataPtr_Normal2Thin m_normal2Thin;
	Cast_DataPtr_Lean2Thin m_lean2Thin;
	Cast_DataPtr_Thin2Thin m_thin2Thin;

	CastOperator* m_operatorTable[DataPtrTypeKind__Count][DataPtrTypeKind__Count];

public:
	Cast_DataPtr();

	virtual
	CastOperator*
	getCastOperator(
		const Value& opValue,
		Type* type
	);
};

//..............................................................................

// data ref (UnOpKind_Addr => data ptr cast => UnOpKind_Indir)

class Cast_DataRef: public CastOperator {
public:
	Cast_DataRef() {
		m_opFlags = OpFlag_KeepRef;
	}

	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
	);

	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);
};

//..............................................................................

} // namespace ct
} // namespace jnc
