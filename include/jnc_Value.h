// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Type.h"
#include "jnc_ObjHdr.h"

namespace jnc {

class Namespace;
class Scope;
class Variable;
class Function;
class FunctionTypeOverload;
class Property;
class StructField;
class ClassType;
class Closure;
class LeanDataPtrValidator;

//.............................................................................

enum ValueKind
{
	ValueKind_Void = 0,
	ValueKind_Null,
	ValueKind_Namespace,
	ValueKind_Type,
	ValueKind_Const,
	ValueKind_Variable,
	ValueKind_Function,
	ValueKind_FunctionTypeOverload,
	ValueKind_Property,
	ValueKind_Field,
	ValueKind_LlvmRegister,
	ValueKind_BoolNot,
	ValueKind_BoolAnd,
	ValueKind_BoolOr,
	ValueKind__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getValueKindString (ValueKind valueKind);

//.............................................................................

class Value
{
protected:
	struct BufHdr
	{
		size_t m_size;
	};

	class GetBufSize
	{
	public:
		size_t
		operator () (const BufHdr& hdr)
		{
			return sizeof (BufHdr) + hdr.m_size;
		}
	};

protected:
	ValueKind m_valueKind;
	Type* m_type;

	ref::Buf <BufHdr, GetBufSize> m_const;

	union
	{
		ModuleItem* m_item;
		Namespace* m_namespace;
		Variable* m_variable;
		Function* m_function;
		FunctionTypeOverload* m_functionTypeOverload;
		Property* m_property;
		StructField* m_field;
	};

	mutable llvm::Value* m_llvmValue;

	ref::Ptr <Closure> m_closure;
	ref::Ptr <LeanDataPtrValidator> m_leanDataPtrValidator;

public:
	Value ()
	{
		init ();
	}

	Value (
		const Value& value,
		Type* type
		)
	{
		init ();
		overrideType (value, type);
	}

	Value (
		const Value& value,
		TypeKind typeKind
		)
	{
		init ();
		overrideType (value, typeKind);
	}

	Value (
		int64_t value,
		TypeKind typeKind
		)
	{
		init ();
		createConst (&value, typeKind);
	}

	Value (
		int64_t value,
		Type* type
		)
	{
		init ();
		createConst (&value, type);
	}

	Value (
		const void* p,
		Type* type
		)
	{
		init ();
		createConst (p, type);
	}

	Value (Type* type)
	{
		init ();
		setType (type);
	}

	Value (Namespace* nspace)
	{
		init ();
		setNamespace (nspace);
	}

	Value (Variable* variable)
	{
		init ();
		setVariable (variable);
	}

	Value (Function* function)
	{
		init ();
		setFunction (function);
	}

	Value (FunctionTypeOverload* functionTypeOverload)
	{
		init ();
		setFunctionTypeOverload (functionTypeOverload);
	}

	Value (Property* prop)
	{
		init ();
		setProperty (prop);
	}

	Value (
		llvm::Value* llvmValue,
		Type* type = NULL,
		ValueKind valueKind = ValueKind_LlvmRegister
		)
	{
		init ();
		setLlvmValue (llvmValue, type, valueKind);
	}

	Value (
		llvm::Value* llvmValue,
		TypeKind typeKind,
		ValueKind valueKind = ValueKind_LlvmRegister
		)
	{
		init ();
		setLlvmValue (llvmValue, typeKind, valueKind);
	}

	operator bool () const
	{
		return !isEmpty ();
	}

	ValueKind
	getValueKind () const
	{
		return m_valueKind;
	}

	bool
	isEmpty () const
	{
		return m_valueKind == ValueKind_Void;
	}

	Type*
	getType () const
	{
		return m_type;
	}

	Namespace*
	getNamespace () const
	{
		ASSERT (m_valueKind == ValueKind_Namespace);
		return m_namespace;
	}

	Variable*
	getVariable () const
	{
		ASSERT (m_valueKind == ValueKind_Variable);
		return m_variable;
	}

	Function*
	getFunction () const
	{
		ASSERT (m_valueKind == ValueKind_Function);
		return m_function;
	}

	FunctionTypeOverload*
	getFunctionTypeOverload () const
	{
		ASSERT (m_valueKind == ValueKind_FunctionTypeOverload);
		return m_functionTypeOverload;
	}

	Property*
	getProperty () const
	{
		ASSERT (m_valueKind == ValueKind_Property);
		return m_property;
	}

	StructField*
	getField () const
	{
		ASSERT (m_valueKind == ValueKind_Field);
		return m_field;
	}

	void*
	getConstData () const
	{
		ASSERT (m_valueKind == ValueKind_Const || m_valueKind == ValueKind_Field);
		return (void*) (m_const + 1);
	}

	int
	getInt () const
	{
		ASSERT (m_valueKind == ValueKind_Const && m_type->getSize () >= sizeof (int));
		return *(int*) getConstData ();
	}

	intptr_t
	getIntPtr () const
	{
		ASSERT (m_valueKind == ValueKind_Const && m_type->getSize () >= sizeof (intptr_t));
		return *(intptr_t*) getConstData ();
	}

	int32_t
	getInt32 () const
	{
		ASSERT (m_valueKind == ValueKind_Const && m_type->getSize () >= sizeof (int32_t));
		return *(int32_t*) getConstData ();
	}

	int64_t
	getInt64 () const
	{
		ASSERT (m_valueKind == ValueKind_Const && m_type->getSize () >= sizeof (int64_t));
		return *(int64_t*) getConstData ();
	}

	size_t
	getSizeT () const
	{
		ASSERT (m_valueKind == ValueKind_Const && m_type->getSize () >= sizeof (size_t));
		return *(size_t*) getConstData ();
	}

	size_t
	getFieldOffset () const
	{
		ASSERT (m_valueKind == ValueKind_Field && m_type->getSize () >= sizeof (size_t));
		return *(size_t*) getConstData ();
	}

	float
	getFloat () const
	{
		ASSERT (m_valueKind == ValueKind_Const && m_type->getSize () >= sizeof (float));
		return *(float*) getConstData ();
	}

	double
	getDouble () const
	{
		ASSERT (m_valueKind == ValueKind_Const && m_type->getSize () >= sizeof (double));
		return *(double*) getConstData ();
	}

	bool
	hasLlvmValue () const
	{
		return m_llvmValue != NULL || m_valueKind == ValueKind_Const;
	}

	llvm::Value*
	getLlvmValue () const;

	rtl::String
	getLlvmTypeString () const
	{
		llvm::Value* llvmValue = getLlvmValue ();
		return llvmValue ? jnc::getLlvmTypeString (llvmValue->getType ()) : rtl::String ();
	}

	static
	llvm::Constant*
	getLlvmConst (
		Type* type,
		const void* p
		);

	Closure*
	getClosure () const
	{
		return m_closure;
	}

	Closure*
	createClosure ();

	void
	setClosure (Closure* closure);

	void
	clearClosure ()
	{
		m_closure = ref::PtrKind_Null;
	}

	void
	insertToClosureHead (const Value& value);

	void
	insertToClosureTail (const Value& value);

	Type*
	getClosureAwareType () const;

	void
	overrideType (Type* type)
	{
		m_type = type;
	}

	void
	overrideType (TypeKind typeKind);

	void
	overrideType (
		const Value& value,
		Type* type
		)
	{
		*this = value;
		overrideType (type);
	}

	void
	overrideType (
		const Value& value,
		TypeKind typeKind
		)
	{
		*this = value;
		overrideType (typeKind);
	}

	void
	clear ();

	void
	setVoid ();

	void
	setNull ();

	void
	setNamespace (Namespace* nspace);

	void
	setType (TypeKind typeKind);

	void
	setType (Type* type);

	void
	setVariable (Variable* variable);

	void
	setFunction (Function* function);

	void
	setFunctionTypeOverload (FunctionTypeOverload* functionTypeOverload);

	void
	setProperty (Property* prop);

	void
	setField (
		StructField* field,
		Type* type,
		size_t baseOffset
		);

	void
	setField (
		StructField* field,
		size_t baseOffset
		);

	void
	setLlvmValue (
		llvm::Value* llvmValue,
		Type* type,
		ValueKind valueKind = ValueKind_LlvmRegister
		);

	void
	setLlvmValue (
		llvm::Value* llvmValue,
		TypeKind typeKind,
		ValueKind valueKind = ValueKind_LlvmRegister
		);

	LeanDataPtrValidator*
	getLeanDataPtrValidator () const
	{
		return m_leanDataPtrValidator;
	}

	void
	setLeanDataPtrValidator (LeanDataPtrValidator* validator);

	void
	setLeanDataPtrValidator (const Value& validatorValue);

	void
	setLeanDataPtrValidator (
		const Value& scopeValidatorValue,
		const Value& rangeBeginValue,
		const Value& sizeValue
		);

	void
	setLeanDataPtrValidator (
		const Value& scopeValidatorValue,
		const Value& rangeBeginValue,
		size_t size
		)
	{
		setLeanDataPtrValidator (scopeValidatorValue, rangeBeginValue, Value (size, TypeKind_SizeT));
	}

	void
	setLeanDataPtr (
		llvm::Value* llvmValue,
		DataPtrType* type,
		LeanDataPtrValidator* validator
		)
	{
		setLlvmValue (llvmValue, (Type*) type);
		setLeanDataPtrValidator (validator);
	}

	void
	setLeanDataPtr (
		llvm::Value* llvmValue,
		DataPtrType* type,
		const Value& validatorValue
		)
	{
		setLlvmValue (llvmValue, (Type*) type);
		setLeanDataPtrValidator (validatorValue);
	}

	void
	setLeanDataPtr (
		llvm::Value* llvmValue,
		DataPtrType* type,
		const Value& scopeValidatorValue,
		const Value& rangeBeginValue,
		const Value& sizeValue
		)
	{
		setLlvmValue (llvmValue, (Type*) type);
		setLeanDataPtrValidator (scopeValidatorValue, rangeBeginValue, sizeValue);
	}

	void
	setLeanDataPtr (
		llvm::Value* llvmValue,
		DataPtrType* type,
		const Value& scopeValidatorValue,
		const Value& rangeBeginValue,
		size_t size
		)
	{
		setLeanDataPtr (llvmValue, type, scopeValidatorValue, rangeBeginValue, Value (size, TypeKind_SizeT));
	}

	bool
	createConst (
		const void* p,
		Type* type
		);

	bool
	createConst (
		const void* p,
		TypeKind type
		);

	void
	setConstBool (bool value)
	{
		createConst (&value, TypeKind_Bool);
	}

	void
	setConstInt32 (
		int32_t value,
		Type* type
		)
	{
		createConst (&value, type);
	}

	void
	setConstInt32 (
		int32_t value,
		TypeKind typeKind
		)
	{
		createConst (&value, typeKind);
	}

	void
	setConstInt32 (int32_t value)
	{
		setConstInt32 (value, getInt32TypeKind (value));
	}

	void
	setConstInt32_u (uint32_t value)
	{
		setConstInt32 (value, getInt32TypeKind_u (value));
	}

	void
	setConstInt64 (
		int64_t value,
		Type* type
		)
	{
		createConst (&value, type);
	}

	void
	setConstInt64 (
		int64_t value,
		TypeKind typeKind
		)
	{
		createConst (&value, typeKind);
	}

	void
	setConstInt64 (int64_t value)
	{
		setConstInt64 (value, getInt64TypeKind (value));
	}

	void
	setConstInt64_u (uint64_t value)
	{
		setConstInt64 (value, getInt64TypeKind_u (value));
	}

	void
	setConstSizeT (
		size_t value,
		TypeKind typeKind = TypeKind_SizeT
		)
	{
		createConst (&value, typeKind);
	}

	void
	setConstFloat (float value)
	{
		createConst (&value, TypeKind_Float);
	}

	void
	setConstDouble (double value)
	{
		createConst (&value, TypeKind_Double);
	}

	void
	setCharArray (const char* p)
	{
		setCharArray (p, strlen (p) + 1);
	}

	void
	setCharArray (
		const void* p,
		size_t count
		);

protected:
	void
	init ();
};

//.............................................................................

} // namespace jnc {
