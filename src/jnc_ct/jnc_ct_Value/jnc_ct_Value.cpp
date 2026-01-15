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
#include "jnc_ct_TypeMgr/jnc_ct_UnionType.h"

namespace jnc {
namespace ct {

//..............................................................................

class LlvmPodArray: public llvm::ConstantDataSequential {
public:
	static
	llvm::Constant*
	get(
		llvm::Type* llvmType,
		const void* p,
		size_t size
	) {
		return getImpl(llvm::StringRef((char*)p, size), llvmType);
	}

	static
	llvm::Constant*
	get(
		ArrayType* type,
		const void* p
	) {
		return get(type->getLlvmType(), p, type->getSize());
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class LlvmPodStruct {
protected:
	char _m_buffer[256];
	sl::Array<llvm::Constant*> m_llvmMemberArray;
	llvm::StructType* m_llvmType;
	size_t m_llvmIdx;
	size_t m_offset;
	char* m_p;

	LlvmPodStruct(
		StructType* type,
		const void* p0
	):
		m_llvmMemberArray(rc::BufKind_Field, _m_buffer, sizeof(_m_buffer)) {
		ASSERT(type->getFlags() & TypeFlag_LayoutReady);
		ASSERT(llvm::isa<llvm::StructType>(*type->getLlvmType()));
		m_llvmType = (llvm::StructType*)type->getLlvmType();
		m_llvmIdx = 0;
		m_offset = 0;
		m_p = (char*)p0;
	}

	void
	addPaddingIf(size_t offset) {
		ASSERT(offset >= m_offset);
		if (offset == m_offset)
			return;

		llvm::Type* llvmType = m_llvmType->getElementType(m_llvmIdx++);
		ASSERT(
			llvmType->getArrayElementType()->getTypeID() == llvm::Type::IntegerTyID &&
			llvmType->getArrayNumElements() == offset - m_offset
		);

		m_llvmMemberArray.append(LlvmPodArray::get(llvmType, m_p + m_offset, offset - m_offset));
		m_offset = offset;
	}

	void
	add(
		size_t offset,
		Type* type
	) {
		addPaddingIf(offset);

		llvm::Type* llvmType = m_llvmType->getElementType(m_llvmIdx++);
		ASSERT(llvmType == type->getLlvmType());

		m_llvmMemberArray.append((llvm::Constant*)jnc::ct::Value(m_p + offset, type).getLlvmValue());
		m_offset += type->getSize();
	}

	llvm::Constant*
	finalize() {
		return llvm::ConstantStruct::get(
			(llvm::StructType*)m_llvmType,
			llvm::ArrayRef<llvm::Constant*>(m_llvmMemberArray, m_llvmMemberArray.getCount())
		);
	}

public:
	static
	llvm::Constant*
	get(
		StructType* type,
		const void* p
	) {
		LlvmPodStruct layout(type, p);

		const sl::Array<BaseTypeSlot*>& baseTypeArray = type->getBaseTypeArray();
		size_t baseTypeCount = baseTypeArray.getCount();
		for (size_t i = 0; i < baseTypeCount; i++) {
			BaseTypeSlot* slot = baseTypeArray[i];
			layout.add(slot->getOffset(), slot->getType());
		}

		const sl::Array<Field*>& fieldArray = type->getFieldArray();
		size_t fieldCount = fieldArray.getCount();
		for (size_t i = 0; i < fieldCount; i++) {
			Field* field = fieldArray[i];
			layout.add(field->getOffset(), field->getType());
		}

		layout.addPaddingIf(type->getSize());
		return layout.finalize();
	}
};

//..............................................................................

const char*
getValueKindString(ValueKind valueKind) {
	static const char* stringTable[ValueKind__Count] = {
		"void",                   // ValueKind_Void = 0,
		"null",                   // ValueKind_Null,
		"namespace",              // ValueKind_Namespace,
		"template",               // ValueKind_Template,
		"type",                   // ValueKind_Type,
		"const",                  // ValueKind_Const,
		"variable",               // ValueKind_Variable,
		"function",               // ValueKind_Function,
		"function-overload",      // ValueKind_FunctionOverload,
		"property",               // ValueKind_Property,
		"field",                  // ValueKind_Field,
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
getLlvmConstantFunc_variant(
	Type* type,
	const void* p
) {
	type = type->getModule()->m_typeMgr.getStdType(StdType_VariantStruct);
	return LlvmPodStruct::get((StructType*)type, p);
}

llvm::Constant*
getLlvmConstantFunc_string(
	Type* type,
	const void* p
) {
	type = type->getModule()->m_typeMgr.getStdType(StdType_StringStruct);
	return LlvmPodStruct::get((StructType*)type, p);
}

llvm::Constant*
getLlvmConstantFunc_bool(
	Type* type,
	const void* p
) {
	uint64_t value = *(bool*)p != 0;
	return llvm::ConstantInt::get(
		type->getLlvmType(),
		llvm::APInt(1, value, false)
	);
}

template <
	typename T,
	bool isSigned
>
llvm::Constant*
getLlvmConstantFunc_int(
	Type* type,
	const void* p
) {
	uint64_t value = *(const T*)p;
	return llvm::ConstantInt::get(
		type->getLlvmType(),
		llvm::APInt(sizeof(T) * 8, value, isSigned)
	);
}

template <typename T>
llvm::Constant*
getLlvmConstantFunc_fp(
	Type* type,
	const void* p
) {
	return llvm::ConstantFP::get(type->getLlvmType(), *(T*)p);
}

llvm::Constant*
getLlvmConstantFunc_enum(
	Type* type,
	const void* p
) {
	type = ((EnumType*)type)->getRootType();
	return Value::getLlvmConst(type, p);
}

llvm::Constant*
getLlvmConstantFunc_array(
	Type* type,
	const void* p
) {
	return LlvmPodArray::get((ArrayType*)type, p);
}

llvm::Constant*
getLlvmConstantFunc_struct(
	Type* type,
	const void* p
) {
	return LlvmPodStruct::get((StructType*)type, p);
}

llvm::Constant*
getLlvmConstantFunc_union(
	Type* type,
	const void* p
) {
	return LlvmPodStruct::get(((UnionType*)type)->getStructType(), p);
}

llvm::Constant*
getLlvmConstantFunc_ptr(
	Type* type,
	const void* p
) {
	uint64_t integer = *(const size_t*)p;

	llvm::Constant* llvmConst = llvm::ConstantInt::get(
		type->getModule()->m_typeMgr.getPrimitiveType(TypeKind_IntPtr_u)->getLlvmType(),
		llvm::APInt(JNC_PTR_BITS, integer, false)
	);

	return llvm::ConstantExpr::getIntToPtr(llvmConst, type->getLlvmType());
}

llvm::Constant*
getLlvmConstantFunc_dataPtr(
	Type* type,
	const void* p
) {
	return ((DataPtrType*)type)->getPtrTypeKind() == DataPtrTypeKind_Normal ?
		LlvmPodStruct::get((StructType*)type->getModule()->m_typeMgr.getStdType(StdType_DataPtrStruct), p) :
		getLlvmConstantFunc_ptr(type, p);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

llvm::Constant*
Value::getLlvmConst(
	Type* type,
	const void* p
) {
	if (mem::isZeroMemory(p, type->getSize()))
		return llvm::Constant::getNullValue(type->getLlvmType());

	typedef
	llvm::Constant*
	GetLlvmConstantFunc(
		Type* type,
		const void* p
	);

	GetLlvmConstantFunc* getLlvmConstantFuncTable[TypeKind_PropertyRef + 1] = {
		NULL,                                      // TypeKind_Void (should never happen)
		getLlvmConstantFunc_variant,               // TypeKind_Variant
		getLlvmConstantFunc_string,                // TypeKind_String
		getLlvmConstantFunc_bool,                  // TypeKind_Bool
		getLlvmConstantFunc_int<int8_t, true>,     // TypeKind_Int8
		getLlvmConstantFunc_int<uint8_t, false>,   // TypeKind_Int8_u
		getLlvmConstantFunc_int<int16_t, true>,    // TypeKind_Int16
		getLlvmConstantFunc_int<uint16_t, false>,  // TypeKind_Int16_u
		getLlvmConstantFunc_int<int32_t, true>,    // TypeKind_Int32
		getLlvmConstantFunc_int<uint32_t, false>,  // TypeKind_Int32_u
		getLlvmConstantFunc_int<int64_t, true>,    // TypeKind_Int64
		getLlvmConstantFunc_int<uint64_t, false>,  // TypeKind_Int64_u
		getLlvmConstantFunc_fp<float>,             // TypeKind_Float
		getLlvmConstantFunc_fp<double>,            // TypeKind_Double
		getLlvmConstantFunc_array,                 // TypeKind_Array
		getLlvmConstantFunc_enum,                  // TypeKind_Enum
		getLlvmConstantFunc_struct,                // TypeKind_Struct
		getLlvmConstantFunc_union,                 // TypeKind_Union
		NULL,                                      // TypeKind_Class (should never happen)
		NULL,                                      // TypeKind_Function (should never happen)
		NULL,                                      // TypeKind_Property (should never happen)
		getLlvmConstantFunc_dataPtr,               // TypeKind_DataPtr
		getLlvmConstantFunc_dataPtr,               // TypeKind_DataRef
		getLlvmConstantFunc_ptr,                   // TypeKind_ClassPtr
		getLlvmConstantFunc_ptr,                   // TypeKind_ClassRef
	};

	TypeKind typeKind = type->getTypeKind();
	ASSERT(typeKind < countof(getLlvmConstantFuncTable) && getLlvmConstantFuncTable[typeKind] != NULL);
	return getLlvmConstantFuncTable[typeKind](type, p);
}

bool
Value::trySetVariable(Variable* variable) {
	bool result = variable->getType()->ensureLayout();
	if (!result)
		return false;

	Type* resultType = getDirectRefType(
		variable->getType(),
		variable->getPtrTypeFlags() | PtrTypeFlag_Safe
	);

	if (!variable->getModule()->hasCodeGen()) {
		setType(resultType);
		m_item = variable;
		return true;
	}

	clear();

	m_valueKind = ValueKind_Variable;
	m_variable = variable;
	m_type = resultType;
	m_llvmValue = variable->getLlvmValue();
	return true;
}

void
Value::setFunction(
	Function* function,
	FunctionType* functionType
) {
	ASSERT(functionType->getFlags() & TypeFlag_LayoutReady);

	FunctionPtrType* resultType = functionType->getFunctionPtrType(
		TypeKind_FunctionRef,
		FunctionPtrTypeKind_Thin,
		PtrTypeFlag_Safe
	);

	if (!function->getModule()->hasCodeGen()) {
		setType(resultType);
		m_item = function;
		return;
	}

	clear();

	m_valueKind = ValueKind_Function;
	m_function = function;
	m_type = resultType;

	if (!function->isVirtual())
		m_llvmValue = function->getLlvmFunction();
}

bool
Value::trySetFunctionOverload(FunctionOverload* functionOverload) {
	bool result = functionOverload->getTypeOverload().ensureLayout();
	if (!result)
		return false;

	clear();

	m_valueKind = ValueKind_FunctionOverload;
	m_functionOverload = functionOverload;

	AXL_TODO("encode function type overload in Value");
	m_type = functionOverload->getModule()->m_typeMgr.getPrimitiveType(TypeKind_Void);
	return true;
}

bool
Value::trySetProperty(Property* prop) {
	bool result = prop->getType()->ensureLayout();
	if (!result)
		return false;

	clear();

	m_valueKind = ValueKind_Property;
	m_property = prop;
	m_type = prop->getType()->getPropertyPtrType(
		TypeKind_PropertyRef,
		PropertyPtrTypeKind_Thin,
		PtrTypeFlag_Safe
	);

	// don't assign LlvmValue (property LlvmValue is only needed for pointers)
	return true;
}

bool
Value::trySetEnumConst(EnumConst* enumConst) {
	EnumType* enumType = enumConst->getType();
	if (!(enumConst->getFlags() & EnumConstFlag_ValueReady)) {
		bool result = enumType->ensureLayout();
		if (!result)
			return false;
	}

	uint64_t enumValue = enumConst->getValue();

	bool result = createConst(
		&enumValue,
		(enumType->getFlags() & TypeFlag_LayoutReady) ?
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
