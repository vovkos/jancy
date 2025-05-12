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

#include "jnc_ct_Type.h"
#include "jnc_Function.h"

namespace jnc {
namespace ct {

class Namespace;
class Scope;
class Variable;
class Function;
class FunctionTypeOverload;
class Property;
class Field;
class ClassType;
class Closure;
class LeanDataPtrValidator;

//..............................................................................

enum ValueKind {
	ValueKind_Void = 0,
	ValueKind_Null,
	ValueKind_Namespace,
	ValueKind_Type,
	ValueKind_Const,
	ValueKind_Variable,
	ValueKind_Function,
	ValueKind_FunctionOverload,
	ValueKind_FunctionTypeOverload,
	ValueKind_Property,
	ValueKind_Field,
	ValueKind_LlvmRegister,
	ValueKind_BoolNot,
	ValueKind_BoolAnd,
	ValueKind_BoolOr,
	ValueKind__Count,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getValueKindString(ValueKind valueKind);

//..............................................................................

class Value {
	friend class Parser;

protected:
	ValueKind m_valueKind;
	Type* m_type;

	union {
		ModuleItem* m_item;
		Namespace* m_namespace;
		Variable* m_variable;
		Function* m_function;
		FunctionOverload* m_functionOverload;
		FunctionTypeOverload* m_functionTypeOverload;
		Property* m_property;
		Field* m_field;
	};

	sl::Array<char> m_constData;
	rc::Ptr<Closure> m_closure;

	// codegen-only

	mutable rc::Ptr<LeanDataPtrValidator> m_leanDataPtrValidator;
	mutable llvm::Value* m_llvmValue;

public:
	Value() {
		init();
	}

	Value(
		const Value& value,
		Type* type
	) {
		init();
		overrideType(value, type);
	}

	Value(
		int64_t value,
		Type* type
	) {
		init();
		createConst(&value, type);
	}

	Value(
		const void* p,
		Type* type
	) {
		init();
		createConst(p, type);
	}

	Value(Type* type) {
		init();
		setType(type);
	}

	Value(GlobalNamespace* nspace) {
		init();
		setNamespace(nspace);
	}

	Value(Variable* variable) {
		init();
		setVariable(variable);
	}

	Value(Function* function) {
		init();
		setFunction(function);
	}

	Value(FunctionOverload* functionOverload) {
		init();
		setFunctionOverload(functionOverload);
	}

	Value(OverloadableFunction function) {
		init();
		setOverloadableFunction(function);
	}

	Value(FunctionTypeOverload* functionTypeOverload) {
		init();
		setFunctionTypeOverload(functionTypeOverload);
	}

	Value(Property* prop) {
		init();
		setProperty(prop);
	}

	Value(EnumConst* enumConst) {
		init();
		setEnumConst(enumConst);
	}

	Value(
		llvm::Value* llvmValue,
		Type* type = NULL,
		ValueKind valueKind = ValueKind_LlvmRegister
	) {
		init();
		setLlvmValue(llvmValue, type, valueKind);
	}

	operator bool() const {
		return !isEmpty();
	}

	ValueKind
	getValueKind() const {
		return m_valueKind;
	}

	bool
	isEmpty() const {
		return m_valueKind == ValueKind_Void;
	}

	Type*
	getType() const {
		return m_type;
	}

	ModuleItem*
	getModuleItem() const {
		return m_item;
	}

	Namespace*
	getNamespace() const {
		ASSERT(m_valueKind == ValueKind_Namespace);
		return m_namespace;
	}

	Variable*
	getVariable() const {
		ASSERT(m_valueKind == ValueKind_Variable);
		return m_variable;
	}

	Function*
	getFunction() const {
		ASSERT(m_valueKind == ValueKind_Function);
		return m_function;
	}

	FunctionOverload*
	getFunctionOverload() const {
		ASSERT(m_valueKind == ValueKind_FunctionOverload);
		return m_functionOverload;
	}

	FunctionTypeOverload*
	getFunctionTypeOverload() const {
		ASSERT(m_valueKind == ValueKind_FunctionTypeOverload);
		return m_functionTypeOverload;
	}

	Property*
	getProperty() const {
		ASSERT(m_valueKind == ValueKind_Property);
		return m_property;
	}

	Field*
	getField() const {
		ASSERT(m_valueKind == ValueKind_Field);
		return m_field;
	}

	sl::Array<char>
	getConstDataArray() const {
		return m_constData;
	}

	const void*
	getConstData() const {
		ASSERT(m_valueKind == ValueKind_Const || m_valueKind == ValueKind_Field);
		return m_constData.cp();
	}

	void*
	getConstData() {
		ASSERT(m_valueKind == ValueKind_Const);
		return m_constData.p();
	}

	bool
	getBool() const {
		ASSERT(m_valueKind == ValueKind_Const && m_type->getSize() >= sizeof(bool));
		return *(const bool*)m_constData.cp();
	}

	int
	getInt() const {
		ASSERT(m_valueKind == ValueKind_Const && m_type->getSize() >= sizeof(int));
		return *(const int*)m_constData.cp();
	}

	intptr_t
	getIntPtr() const {
		ASSERT(m_valueKind == ValueKind_Const && m_type->getSize() >= sizeof(intptr_t));
		return *(const intptr_t*)m_constData.cp();
	}

	int32_t
	getInt32() const {
		ASSERT(m_valueKind == ValueKind_Const && m_type->getSize() >= sizeof(int32_t));
		return *(const int32_t*)m_constData.cp();
	}

	int64_t
	getInt64() const {
		ASSERT(m_valueKind == ValueKind_Const && m_type->getSize() >= sizeof(int64_t));
		return *(const int64_t*)m_constData.cp();
	}

	size_t
	getSizeT() const {
		ASSERT(m_valueKind == ValueKind_Const && m_type->getSize() >= sizeof(size_t));
		return *(const size_t*)m_constData.cp();
	}

	float
	getFloat() const {
		ASSERT(m_valueKind == ValueKind_Const && m_type->getSize() >= sizeof(float));
		return *(const float*)m_constData.cp();
	}

	double
	getDouble() const {
		ASSERT(m_valueKind == ValueKind_Const && m_type->getSize() >= sizeof(double));
		return *(const double*)m_constData.cp();
	}

	size_t
	getFieldOffset() const {
		ASSERT(m_valueKind == ValueKind_Field && m_constData.getCount() >= sizeof(size_t));
		return *(const size_t*)m_constData.cp();
	}

	bool
	hasLlvmValue() const {
		return m_llvmValue != NULL || m_valueKind == ValueKind_Const;
	}

	llvm::Value*
	getLlvmValue() const;

	sl::String
	getLlvmTypeString() const {
		llvm::Value* llvmValue = getLlvmValue();
		return llvmValue ? ct::getLlvmTypeString(llvmValue->getType()) : sl::String();
	}

	static
	llvm::Constant*
	getLlvmConst(
		Type* type,
		const void* p
	);

	Closure*
	getClosure() const {
		return m_closure;
	}

	Closure*
	createClosure();

	void
	setClosure(Closure* closure);

	void
	clearClosure() {
		m_closure = rc::g_nullPtr;
	}

	Type*
	getClosureAwareType() const;

	void
	overrideType(Type* type) {
		m_type = type;
	}

	void
	overrideType(
		const Value& value,
		Type* type
	) {
		*this = value;
		overrideType(type);
	}

	void
	clear();

	bool
	isZero() const {
		return
			m_valueKind == ValueKind_Const &&
			m_type->getTypeKind() == TypeKind_Int8 &&
			*(char*)m_constData.cp() == 0;
	}

	void
	setVoid(Module* module);

	void
	setNull(Module* module);

	void
	setNamespace(GlobalNamespace* nspace);

	void
	setNamespace(NamedType* type);

	void
	setType(Type* type);

	void
	setVariable(Variable* variable);

	void
	setFunction(Function* function) {
		bool result = trySetFunction(function);
		ASSERT(result);
	}

	bool
	trySetFunction(Function* function);

	void
	setFunctionOverload(FunctionOverload* functionOverload);

	void
	setOverloadableFunction(OverloadableFunction function) {
		bool result = trySetOverloadableFunction(function);
		ASSERT(result);
	}

	bool
	trySetOverloadableFunction(OverloadableFunction function);

	void
	setFunctionTypeOverload(FunctionTypeOverload* functionTypeOverload);

	void
	setProperty(Property* prop);

	void
	setEnumConst(EnumConst* enumConst) {
		bool result = trySetEnumConst(enumConst);
		ASSERT(result);
	}

	bool
	trySetEnumConst(EnumConst* enumConst);

	void
	setField(
		Field* field,
		size_t baseOffset
	);

	void
	setLlvmValue(
		llvm::Value* llvmValue,
		Type* type,
		ValueKind valueKind = ValueKind_LlvmRegister
	);

	LeanDataPtrValidator*
	getLeanDataPtrValidator() const;

	void
	setLeanDataPtrValidator(LeanDataPtrValidator* validator);

	void
	setLeanDataPtrValidator(const Value& originValue);

	void
	setLeanDataPtrValidator(
		const Value& originValue,
		const Value& rangeBeginValue,
		size_t rangeLength
	);

	void
	setLeanDataPtr(
		llvm::Value* llvmValue,
		DataPtrType* type,
		LeanDataPtrValidator* validator
	) {
		setLlvmValue(llvmValue, (Type*)type);
		setLeanDataPtrValidator(validator);
	}

	void
	setLeanDataPtr(
		llvm::Value* llvmValue,
		DataPtrType* type,
		const Value& originValue
	) {
		setLlvmValue(llvmValue, (Type*)type);
		setLeanDataPtrValidator(originValue);
	}

	void
	setLeanDataPtr(
		llvm::Value* llvmValue,
		DataPtrType* type,
		const Value& originValue,
		const Value& rangeBeginValue,
		size_t rangeLength
	) {
		setLlvmValue(llvmValue, (Type*)type);
		setLeanDataPtrValidator(originValue, rangeBeginValue, rangeLength);
	}

	bool
	createConst(
		const void* p,
		Type* type
	);

	void
	setConstBool(
		bool value,
		Module* module
	) {
		createConst(&value, getSimpleType(TypeKind_Bool, module));
	}

	void
	setConstInt32(
		int32_t value,
		Type* type
	) {
		createConst(&value, type);
	}

	void
	setConstInt32(
		int32_t value,
		Module* module
	) {
		Type* type = getSimpleType(getInt32TypeKind(value), module);
		setConstInt32(value, type);
	}

	void
	setConstInt32_u(
		uint32_t value,
		Module* module
	) {
		Type* type = getSimpleType(getInt32TypeKind_u(value), module);
		setConstInt32(value, type);
	}

	void
	setConstInt64(
		int64_t value,
		Type* type
	) {
		createConst(&value, type);
	}

	void
	setConstInt64(
		int64_t value,
		Module* module
	) {
		Type* type = getSimpleType(getInt64TypeKind(value), module);
		setConstInt64(value, type);
	}

	void
	setConstInt64_u(
		uint64_t value,
		Module* module
	) {
		Type* type = getSimpleType(getInt64TypeKind_u(value), module);
		setConstInt64(value, type);
	}

	void
	setConstSizeT(
		size_t value,
		Type* type
	) {
		createConst(&value, type);
	}

	void
	setConstSizeT(
		size_t value,
		Module* module
	) {
		createConst(&value, getSimpleType(TypeKind_SizeT, module));
	}

	void
	setConstFloat(
		float value,
		Module* module
	) {
		createConst(&value, getSimpleType(TypeKind_Float, module));
	}

	void
	setConstDouble(
		double value,
		Module* module
	) {
		createConst(&value, getSimpleType(TypeKind_Double, module));
	}

	void
	setEmptyCharArray(Module* module) {
		setCharArray("", 1, module);
	}

	void
	setCharArray(
		const sl::StringRef& string,
		Module* module
	) {
		if (string.isEmpty())
			setEmptyCharArray(module);
		else {
			setCharArray(NULL, string.getLength() + 1, module);
			memcpy(m_constData.p(), string.cp(), string.getLength());
		}
	}

	void
	setCharArray(
		const void* p,
		size_t count,
		Module* module
	);

protected:
	void
	init();

	void
	prepareLlvmValue() const {
		ASSERT(!m_llvmValue && m_valueKind == ValueKind_Const);
		m_llvmValue = getLlvmConst(m_type, getConstData()); // mutable
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
llvm::Value*
Value::getLlvmValue() const {
	if (!m_llvmValue)
		prepareLlvmValue();

	return m_llvmValue;
}

//..............................................................................

struct DynamicFieldValueInfo: rc::RefCount {
	Value m_parentValue;
	DerivableType* m_parentType;
	Field* m_field;
};

//..............................................................................

} // namespace ct
} // namespace jnc
