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

#include "pch.h"
#include "jnc_ct_Value.h"
#include "jnc_ct_Closure.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_ArrayType.h"
#include "jnc_ct_FunctionOverload.h"
#include "jnc_ct_LeanDataPtrValidator.h"

namespace jnc {
namespace ct {

//..............................................................................

class LlvmPodArray: public llvm::ConstantDataSequential {
public:
	static
	llvm::Constant*
	get(
		ArrayType* type,
		const void* p
	) {
		llvm::Type* llvmType = type->getLlvmType();
		return getImpl(llvm::StringRef((char*)p, type->getSize()), llvmType);
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class LlvmPodStruct {
public:
	static
	llvm::Constant*
	get(
		StructType* type,
		const void* p
	) {
		llvm::Type* llvmType = type->getLlvmType();

		char buffer[256];
		sl::Array<llvm::Constant*> llvmMemberArray(rc::BufKind_Stack, buffer, sizeof(buffer));

		const sl::Array<Field*>& fieldArray = type->getFieldArray();
		size_t count = fieldArray.getCount();

		for (size_t i = 0; i < count; i++) {
			Field* field = fieldArray[i];
			jnc::ct::Value memberConst((char*)p + field->getOffset(), field->getType());
			llvmMemberArray.append((llvm::Constant*)memberConst.getLlvmValue());
		}

		return llvm::ConstantStruct::get(
			(llvm::StructType*)llvmType,
			llvm::ArrayRef<llvm::Constant*> (llvmMemberArray, llvmMemberArray.getCount())
		);
	}
};

//..............................................................................

const char*
getValueKindString(ValueKind valueKind) {
	static const char* stringTable[ValueKind__Count] = {
		"void",                   // ValueKind_Void = 0,
		"null",                   // ValueKind_Null,
		"namespace",              // ValueKind_Namespace,
		"type",                   // ValueKind_Type,
		"const",                  // ValueKind_Const,
		"variable",               // ValueKind_Variable,
		"function",               // ValueKind_Function,
		"function-overload",      // ValueKind_FunctionOverload,
		"function-type-overload", // ValueKind_FunctionTypeOverload,
		"property",               // ValueKind_Property,
		"llvm-register",          // ValueKind_LlvmRegister,
		"bool-not",               // ValueKind_BoolNot,
		"bool-and",               // ValueKind_BoolAnd,
		"bool-or",                // ValueKind_BoolOr,
	};

	return (size_t)valueKind < ValueKind__Count ?
		stringTable[valueKind] :
		"undefined-value-kind";
}

//..............................................................................

void
Value::init() {
	m_valueKind = ValueKind_Void;
	m_type = NULL;
	m_variable = NULL;
	m_llvmValue = NULL;
}

void
Value::clear() {
	m_valueKind = ValueKind_Void;
	m_type = NULL;
	m_item = NULL;
	m_llvmValue = NULL;
	m_closure = rc::g_nullPtr;
	m_leanDataPtrValidator = rc::g_nullPtr;
}

llvm::Constant*
getLlvmPtrConst(
	Type* type,
	const void* p
) {
	int64_t integer = *(int64_t*)p;

	llvm::Constant* llvmConst = llvm::ConstantInt::get(
		type->getModule()->m_typeMgr.getPrimitiveType(TypeKind_IntPtr_u)->getLlvmType(),
		llvm::APInt(sizeof(void*)* 8, integer, false)
	);

	return llvm::ConstantExpr::getIntToPtr(llvmConst, type->getLlvmType());
}

llvm::Constant*
Value::getLlvmConst(
	Type* type,
	const void* p
) {
	int64_t integer;
	double doubleValue;
	llvm::Constant* llvmConst = NULL;

	if (type->getTypeKind() == TypeKind_Enum)
		type = ((EnumType*)type)->getRootType();

	Module* module = type->getModule();

	TypeKind typeKind = type->getTypeKind();
	switch (typeKind) {
	case TypeKind_Bool:
		integer = *(int8_t*)p != 0;
		llvmConst = llvm::ConstantInt::get(
			type->getLlvmType(),
			llvm::APInt(1, integer)
		);
		break;

	case TypeKind_Int8:
	case TypeKind_Int8_u:
		integer = *(int8_t*)p;
		llvmConst = llvm::ConstantInt::get(
			type->getLlvmType(),
			llvm::APInt(8, integer, !(type->getTypeKindFlags() & TypeKindFlag_Unsigned))
		);
		break;

	case TypeKind_Int16:
	case TypeKind_Int16_u:
		integer = *(int16_t*)p;
		llvmConst = llvm::ConstantInt::get(
			type->getLlvmType(),
			llvm::APInt(16, integer, !(type->getTypeKindFlags() & TypeKindFlag_Unsigned))
		);
		break;

	case TypeKind_Int32:
	case TypeKind_Int32_u:
		integer = *(int32_t*)p;
		llvmConst = llvm::ConstantInt::get(
			type->getLlvmType(),
			llvm::APInt(32, integer, !(type->getTypeKindFlags() & TypeKindFlag_Unsigned))
		);
		break;

	case TypeKind_Int64:
	case TypeKind_Int64_u:
		integer = *(int64_t*)p;
		llvmConst = llvm::ConstantInt::get(
			type->getLlvmType(),
			llvm::APInt(64, integer, !(type->getTypeKindFlags() & TypeKindFlag_Unsigned))
		);
		break;

	case TypeKind_Float:
		doubleValue = *(float*)p;
		llvmConst = llvm::ConstantFP::get(type->getLlvmType(), doubleValue);
		break;

	case TypeKind_Double:
		doubleValue = *(double*)p;
		llvmConst = llvm::ConstantFP::get(type->getLlvmType(), doubleValue);
		break;

	case TypeKind_Variant:
		llvmConst = LlvmPodStruct::get((StructType*)module->m_typeMgr.getStdType(StdType_VariantStruct), p);
		break;

	case TypeKind_String:
		llvmConst = LlvmPodStruct::get((StructType*)module->m_typeMgr.getStdType(StdType_StringStruct), p);
		break;

	case TypeKind_Array:
		llvmConst = LlvmPodArray::get((ArrayType*)type, p);
		break;

	case TypeKind_Struct:
		llvmConst = LlvmPodStruct::get((StructType*)type, p);
		break;

	case TypeKind_DataPtr:
	case TypeKind_DataRef:
		if (((DataPtrType*)type)->getPtrTypeKind() == DataPtrTypeKind_Normal) {
			llvmConst = LlvmPodStruct::get((StructType*)module->m_typeMgr.getStdType(StdType_DataPtrStruct), p);
		} else { // thin or unsafe
			llvmConst = getLlvmPtrConst(type, p);
		}
		break;

	case TypeKind_ClassPtr:
		llvmConst = getLlvmPtrConst(type, p);
		break;

	default:
		ASSERT(false);
	}

	return llvmConst;
}

Closure*
Value::createClosure() {
	m_closure = AXL_RC_NEW(Closure);
	return m_closure;
}

void
Value::setClosure(Closure* closure) {
	m_closure = closure;
}

Type*
Value::getClosureAwareType() const {
	return m_closure ? m_closure->getClosureType(m_type) : m_type;
}

void
Value::setVoid(Module* module) {
	clear();

	m_valueKind = ValueKind_Void;
	m_type = module->m_typeMgr.getPrimitiveType(TypeKind_Void);
}

void
Value::setNull(Module* module) {
	clear();

	m_valueKind = ValueKind_Null;
	m_type = module->m_typeMgr.getPrimitiveType(TypeKind_Void);
}

void
Value::setType(Type* type) {
	clear();

	m_valueKind = type->getTypeKind() != TypeKind_Void ? ValueKind_Type : ValueKind_Void;
	m_type = type;
}

void
Value::setNamespace(GlobalNamespace* nspace) {
	clear();

	Module* module = nspace->getModule();

	m_valueKind = ValueKind_Namespace;
	m_namespace = nspace;
	m_type = module->m_typeMgr.getPrimitiveType(TypeKind_Void);
}

void
Value::setNamespace(NamedType* type) {
	clear();

	Module* module = type->getModule();

	m_valueKind = ValueKind_Namespace;
	m_namespace = type;
	m_type = module->m_typeMgr.getPrimitiveType(TypeKind_Void);
}

void
Value::setVariable(Variable* variable) {
	clear();

	Module* module = variable->getModule();

	if (!module->hasCodeGen()) {
		bool result = variable->getType()->ensureLayout();
		if (!result) {
			setVoid(module);
			return;
		}

		Type* resultType = getDirectRefType(
			variable->getType(),
			variable->getPtrTypeFlags() | PtrTypeFlag_Safe
		);

		setType(resultType);
		return;
	}

	Type* resultType = getDirectRefType(
		variable->getType(),
		variable->getPtrTypeFlags() | PtrTypeFlag_Safe
	);

	m_valueKind = ValueKind_Variable;
	m_variable = variable;
	m_type = resultType;
	m_llvmValue = variable->getLlvmValue();
}

bool
Value::trySetFunction(Function* function) {
	bool result = function->getType()->ensureLayout();
	if (!result)
		return false;

	FunctionPtrType* resultType = function->getType()->getFunctionPtrType(
		TypeKind_FunctionRef,
		FunctionPtrTypeKind_Thin,
		PtrTypeFlag_Safe
	);

	if (!function->getModule()->hasCodeGen()) {
		setType(resultType);
		return true;
	}

	clear();

	m_valueKind = ValueKind_Function;
	m_function = function;
	m_type = resultType;

	if (!function->isVirtual())
		m_llvmValue = function->getLlvmFunction();

	return true;
}

void
Value::setFunctionOverload(FunctionOverload* functionOverload) {
	clear();

	m_valueKind = ValueKind_FunctionOverload;
	m_functionOverload = functionOverload;
	m_type = functionOverload->getModule()->m_typeMgr.getPrimitiveType(TypeKind_Void);
}

bool
Value::trySetOverloadableFunction(OverloadableFunction function) {
	if (function->getItemKind() == ModuleItemKind_Function)
		return trySetFunction(function.getFunction());

	setFunctionOverload(function.getFunctionOverload());
	return true;
}

void
Value::setFunctionTypeOverload(FunctionTypeOverload* functionTypeOverload) {
	clear();

	m_valueKind = ValueKind_FunctionTypeOverload;
	m_functionTypeOverload = functionTypeOverload;
	m_type = functionTypeOverload->getModule()->m_typeMgr.getPrimitiveType(TypeKind_Void);
}

void
Value::setProperty(Property* prop) {
	clear();

	m_valueKind = ValueKind_Property;
	m_property = prop;
	m_type = prop->getType()->getPropertyPtrType(
		TypeKind_PropertyRef,
		PropertyPtrTypeKind_Thin,
		PtrTypeFlag_Safe
	);

	// don't assign LlvmValue (property LlvmValue is only needed for pointers)
}

bool
Value::trySetEnumConst(EnumConst* enumConst) {
	EnumType* enumType = enumConst->getParentEnumType();
	if (!(enumConst->getFlags() & EnumConstFlag_ValueReady)) {
		bool result = enumType->ensureLayout();
		if (!result)
			return false;
	}

	uint64_t enumValue = enumConst->getValue();

	return createConst(
		&enumValue,
		(enumType->getFlags() & ModuleItemFlag_LayoutReady) ?
			enumType :
			enumType->getBaseType()
		);
}

void
Value::setField(
	Field* field,
	size_t baseOffset
) {
	clear();

	m_valueKind = ValueKind_Field;
	m_field = field;
	m_type = field->getModule()->m_typeMgr.getPrimitiveType(TypeKind_Void);
	m_constData.setCount(sizeof(size_t));
	*(size_t*)m_constData.p() = baseOffset + field->getOffset();
}

void
Value::setLlvmValue(
	llvm::Value* llvmValue,
	Type* type,
	ValueKind valueKind
) {
	clear();

	m_valueKind = valueKind;
	m_type = type;
	m_llvmValue = llvmValue;
}

LeanDataPtrValidator*
Value::getLeanDataPtrValidator() const {
	if (m_leanDataPtrValidator)
		return m_leanDataPtrValidator;

	ASSERT(m_valueKind == ValueKind_Variable);
	m_leanDataPtrValidator = m_variable->getLeanDataPtrValidator();
	return m_leanDataPtrValidator;
}

void
Value::setLeanDataPtrValidator(LeanDataPtrValidator* validator) {
	ASSERT(isDataPtrType(m_type, DataPtrTypeKind_Lean));
	m_leanDataPtrValidator = validator;
}

void
Value::setLeanDataPtrValidator(const Value& originValue) {
	ASSERT(isDataPtrType(m_type, DataPtrTypeKind_Lean));

	if (originValue.m_leanDataPtrValidator) {
		m_leanDataPtrValidator = originValue.m_leanDataPtrValidator;
	} else if (originValue.m_valueKind == ValueKind_Variable) {
		m_leanDataPtrValidator = originValue.m_variable->getLeanDataPtrValidator();
	} else {
		m_leanDataPtrValidator = AXL_RC_NEW(LeanDataPtrValidator);
		m_leanDataPtrValidator->m_originValue = originValue;
	}
}

void
Value::setLeanDataPtrValidator(
	const Value& originValue,
	const Value& rangeBeginValue,
	size_t rangeLength
) {
	ASSERT(isDataPtrType(m_type, DataPtrTypeKind_Lean));

	rc::Ptr<LeanDataPtrValidator> validator = AXL_RC_NEW(LeanDataPtrValidator);
	validator->m_originValue = originValue;
	validator->m_rangeBeginValue = rangeBeginValue;
	validator->m_rangeLength = rangeLength;
	m_leanDataPtrValidator = validator;
}

bool
Value::createConst(
	const void* p,
	Type* type
) {
	bool result;

	clear();

	result = type->ensureLayout();
	if (!result)
		return false;

	size_t size = type->getSize();
	size_t allocSize = sl::align<sizeof(int64_t)>(size); // ensure int64 for getLlvmConst ()

	result = m_constData.setCount(allocSize);
	if (!result)
		return false;

	m_valueKind = ValueKind_Const;
	m_type = type;

	if (p)
		memcpy(m_constData, p, size);
	else
		memset(m_constData, 0, size);

	return true;
}

void
Value::setCharArray(
	const void* p,
	size_t size,
	Module* module
) {
	if (!size)
		size = 1;

	Type* type = module->m_typeMgr.getArrayType(
		module->m_typeMgr.getPrimitiveType(TypeKind_Char),
		size
	);

	createConst(p, type);
}

//..............................................................................

} // namespace ct
} // namespace jnc
