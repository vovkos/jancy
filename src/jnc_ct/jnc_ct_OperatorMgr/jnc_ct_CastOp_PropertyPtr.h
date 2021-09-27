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
#include "jnc_ct_PropertyPtrType.h"

namespace jnc {
namespace ct {

//..............................................................................

class Cast_PropertyPtr_FromDataPtr: public CastOperator {
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

protected:
	bool
	llvmCast_DirectThunk(
		Variable* variable,
		PropertyPtrType* dstPtrType,
		Value* resultValue
	);

	bool
	llvmCast_FullClosure(
		const Value& opValue,
		PropertyPtrType* dstPtrType,
		Value* resultValue
	);
};

//..............................................................................

class Cast_PropertyPtr_Base: public CastOperator {
public:
	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_PropertyPtr_FromFat: public Cast_PropertyPtr_Base {
public:
	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_PropertyPtr_Thin2Fat: public Cast_PropertyPtr_Base {
public:
	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);

protected:
	bool
	llvmCast_NoThunkSimpleClosure(
		const Value& opValue,
		const Value& simpleClosureObjValue,
		PropertyType* srcPropertyType,
		PropertyPtrType* dstPtrType,
		Value* resultValue
	);

	bool
	llvmCast_DirectThunkNoClosure(
		Property* prop,
		PropertyPtrType* dstPtrType,
		Value* resultValue
	);

	bool
	llvmCast_DirectThunkSimpleClosure(
		Property* prop,
		const Value& simpleClosureObjValue,
		PropertyPtrType* dstPtrType,
		Value* resultValue
	);

	bool
	llvmCast_FullClosure(
		const Value& opValue,
		PropertyType* srcPropertyType,
		PropertyPtrType* dstPtrType,
		Value* resultValue
	);

	bool
	createClosurePropertyPtr(
		Property* prop,
		const Value& closureValue,
		PropertyPtrType* ptrType,
		Value* resultValue
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_PropertyPtr_Weak2Normal: public Cast_PropertyPtr_Base {
public:
	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_PropertyPtr_Thin2Thin: public Cast_PropertyPtr_Base {
public:
	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);
};

//..............................................................................

// safe / unsafe fn pointer -> safe fn pointer

class Cast_PropertyPtr: public Cast_Master {
protected:
	Cast_PropertyPtr_FromDataPtr m_fromDataPtr;
	Cast_PropertyPtr_FromFat m_fromFat;
	Cast_PropertyPtr_Weak2Normal m_weak2Normal;
	Cast_PropertyPtr_Thin2Fat m_thin2Fat;
	Cast_PropertyPtr_Thin2Thin m_thin2Thin;

	CastOperator* m_operatorTable[PropertyPtrTypeKind__Count][PropertyPtrTypeKind__Count];

public:
	Cast_PropertyPtr();

	virtual
	bool
	constCast(
		const Value& opValue,
		Type* type,
		void* dst
	);

	virtual
	CastOperator*
	getCastOperator(
		const Value& opValue,
		Type* type
	);
};

//..............................................................................

// data ref (UnOpKind_Indir => data ptr cast => UnOpKind_Addr)

class Cast_PropertyRef: public CastOperator {
public:
	Cast_PropertyRef() {
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
