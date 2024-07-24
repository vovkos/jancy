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
			llvm::ArrayRef<llvm::Constant*>(llvmMemberArray, llvmMemberArray.getCount())
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
		m_item = variable;
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
		m_item = function;
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

bool
Value::trySetEnumConst(EnumConst* enumConst) {
	EnumType* enumType = enumConst->getParentEnumType();
	if (!(enumConst->getFlags() & EnumConstFlag_ValueReady)) {
		bool result = enumType->ensureLayout();
		if (!result)
			return false;
	}

	uint64_t enumValue = enumConst->getValue();

	bool result = createConst(
		&enumValue,
		(enumType->getFlags() & ModuleItemFlag_LayoutReady) ?
			enumType :
			enumType->getBaseType()
		);

	if (!result)
		return false;

	m_item = enumConst;
	return true;
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
		memcpy(m_constData.p(), p, size);
	else
		memset(m_constData.p(), 0, size);

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
