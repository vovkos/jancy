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
#include "jnc_ct_Value.h"
#include "jnc_ct_Closure.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_LeanDataPtrValidator.h"

namespace jnc {
namespace ct {

//..............................................................................

class LlvmPodArray: public llvm::ConstantDataSequential
{
public:
	static
	llvm::Constant*
	get (
		ArrayType* type,
		const void* p
		)
	{
		llvm::Type* llvmType = type->getLlvmType ();
		return getImpl (llvm::StringRef ((char*) p, type->getSize ()), llvmType);
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class LlvmPodStruct
{
public:
	static
	llvm::Constant*
	get (
		StructType* type,
		const void* p
		)
	{
		llvm::Type* llvmType = type->getLlvmType ();

		char buffer [256];
		sl::Array <llvm::Constant*> llvmMemberArray (ref::BufKind_Stack, buffer, sizeof (buffer));

		sl::Array <StructField*> fieldArray = type->getMemberFieldArray ();
		size_t count = fieldArray.getCount ();

		for (size_t i = 0; i < count; i++)
		{
			StructField* field = fieldArray [i];
			jnc::ct::Value memberConst ((char*) p + field->getOffset (), field->getType ());
			llvmMemberArray.append ((llvm::Constant*) memberConst.getLlvmValue ());
		}

		return llvm::ConstantStruct::get (
			(llvm::StructType*) llvmType,
			llvm::ArrayRef <llvm::Constant*> (llvmMemberArray, llvmMemberArray.getCount ())
			);
	}
};

//..............................................................................

const char*
getValueKindString (ValueKind valueKind)
{
	static const char* stringTable [ValueKind__Count] =
	{
		"void",                   // EValue_Void = 0,
		"null",                   // EValue_Null,
		"namespace",              // EValue_Namespace,
		"type",                   // EValue_Type,
		"const",                  // EValue_Const,
		"variable",               // EValue_Variable,
		"function",               // EValue_Function,
		"function-type-overload", // EValue_FunctionTypeOverload,
		"property",               // EValue_Property,
		"llvm-register",          // EValue_LlvmRegister,
		"bool-not",               // EValue_BoolNot,
		"bool-and",               // EValue_BoolAnd,
		"bool-or",                // EValue_BoolOr,
	};

	return (size_t) valueKind < ValueKind__Count ?
		stringTable [valueKind] :
		"undefined-value-kind";
}

//..............................................................................

void
Value::init ()
{
	m_valueKind = ValueKind_Void;
	m_type = NULL;
	m_variable = NULL;
	m_llvmValue = NULL;
}

void
Value::clear ()
{
	m_valueKind = ValueKind_Void;
	m_type = NULL;
	m_item = NULL;
	m_llvmValue = NULL;
	m_closure = ref::g_nullPtr;
	m_leanDataPtrValidator = ref::g_nullPtr;
}

llvm::Value*
Value::getLlvmValue () const
{
	if (m_llvmValue)
		return m_llvmValue;

	ASSERT (m_valueKind == ValueKind_Const);

	m_llvmValue = getLlvmConst (m_type, getConstData ());
	return m_llvmValue;
}

llvm::Constant*
getLlvmPtrConst (
	Type* type,
	const void* p
	)
{
	int64_t integer = *(int64_t*) p;

	llvm::Constant* llvmConst = llvm::ConstantInt::get (
		type->getModule ()->m_typeMgr.getPrimitiveType (TypeKind_IntPtr_u)->getLlvmType (),
		llvm::APInt (sizeof (void*) * 8, integer, false)
		);

	return llvm::ConstantExpr::getIntToPtr (llvmConst, type->getLlvmType ());
}

llvm::Constant*
Value::getLlvmConst (
	Type* type,
	const void* p
	)
{
	int64_t integer;
	double doubleValue;
	llvm::Constant* llvmConst = NULL;

	if (type->getTypeKind () == TypeKind_Enum)
		type = ((EnumType*) type)->getBaseType ();

	Module* module = type->getModule ();

	TypeKind typeKind = type->getTypeKind ();
	switch (typeKind)
	{
	case TypeKind_Bool:
		integer = *(int8_t*) p != 0;
		llvmConst = llvm::ConstantInt::get (
			type->getLlvmType (),
			llvm::APInt (1, integer)
			);
		break;

	case TypeKind_Int8:
	case TypeKind_Int8_u:
		integer = *(int8_t*) p;
		llvmConst = llvm::ConstantInt::get (
			type->getLlvmType (),
			llvm::APInt (8, integer, !(type->getTypeKindFlags () & TypeKindFlag_Unsigned))
			);
		break;

	case TypeKind_Int16:
	case TypeKind_Int16_u:
	case TypeKind_Int16_be:
	case TypeKind_Int16_beu:
		integer = *(int16_t*) p;
		llvmConst = llvm::ConstantInt::get (
			type->getLlvmType (),
			llvm::APInt (16, integer, !(type->getTypeKindFlags () & TypeKindFlag_Unsigned))
			);
		break;

	case TypeKind_Int32:
	case TypeKind_Int32_u:
	case TypeKind_Int32_be:
	case TypeKind_Int32_beu:
		integer = *(int32_t*) p;
		llvmConst = llvm::ConstantInt::get (
			type->getLlvmType (),
			llvm::APInt (32, integer, !(type->getTypeKindFlags () & TypeKindFlag_Unsigned))
			);
		break;

	case TypeKind_Int64:
	case TypeKind_Int64_u:
	case TypeKind_Int64_be:
	case TypeKind_Int64_beu:
		integer = *(int64_t*) p;
		llvmConst = llvm::ConstantInt::get (
			type->getLlvmType (),
			llvm::APInt (64, integer, !(type->getTypeKindFlags () & TypeKindFlag_Unsigned))
			);
		break;

	case TypeKind_Float:
		doubleValue = *(float*) p;
		llvmConst = llvm::ConstantFP::get (type->getLlvmType (), doubleValue);
		break;

	case TypeKind_Double:
		doubleValue = *(double*) p;
		llvmConst = llvm::ConstantFP::get (type->getLlvmType (), doubleValue);
		break;

	case TypeKind_Variant:
		llvmConst = LlvmPodStruct::get ((StructType*) module->m_typeMgr.getStdType (StdType_VariantStruct), p);
		break;

	case TypeKind_Array:
		llvmConst = LlvmPodArray::get ((ArrayType*) type, p);
		break;

	case TypeKind_Struct:
		llvmConst = LlvmPodStruct::get ((StructType*) type, p);
		break;

	case TypeKind_DataPtr:
	case TypeKind_DataRef:
		if (((DataPtrType*) type)->getPtrTypeKind () == DataPtrTypeKind_Normal)
		{
			llvmConst = LlvmPodStruct::get ((StructType*) module->m_typeMgr.getStdType (StdType_DataPtrStruct), p);
		}
		else // thin or unsafe
		{
			llvmConst = getLlvmPtrConst (type, p);
		}
		break;

	case TypeKind_ClassPtr:
		llvmConst = getLlvmPtrConst (type, p);
		break;

	default:
		ASSERT (false);
	}

	return llvmConst;
}

Closure*
Value::createClosure ()
{
	m_closure = AXL_REF_NEW (Closure);
	return m_closure;
}

void
Value::setClosure (Closure* closure)
{
	m_closure = closure;
}

Type*
Value::getClosureAwareType () const
{
	return m_closure ? m_closure->getClosureType (m_type) : m_type;
}

void
Value::setDynamicFieldInfo (
	const Value& parentValue,
	DerivableType* parentType,
	StructField* field
	)
{
	m_dynamicFieldInfo = AXL_REF_NEW (DynamicFieldValueInfo);
	m_dynamicFieldInfo->m_parentValue = parentValue;
	m_dynamicFieldInfo->m_parentType = parentType;
	m_dynamicFieldInfo->m_field = field;
}

void
Value::setVoid (Module* module)
{
	clear ();

	m_valueKind = ValueKind_Void;
	m_type = module->m_typeMgr.getPrimitiveType (TypeKind_Void);
}

void
Value::setNull (Module* module)
{
	clear ();

	m_valueKind = ValueKind_Null;
	m_type = module->m_typeMgr.getPrimitiveType (TypeKind_Void);
}

void
Value::setType (Type* type)
{
	clear ();

	m_valueKind = ValueKind_Type;
	m_type = type;
}

void
Value::setNamespace (GlobalNamespace* nspace)
{
	clear ();

	Module* module = nspace->getModule ();

	m_valueKind = ValueKind_Namespace;
	m_namespace = nspace;
	m_type = module->m_typeMgr.getPrimitiveType (TypeKind_Void);
}

void
Value::setNamespace (NamedType* type)
{
	clear ();

	Module* module = type->getModule ();

	m_valueKind = ValueKind_Namespace;
	m_namespace = type;
	m_type = module->m_typeMgr.getPrimitiveType (TypeKind_Void);
}

void
Value::setVariable (Variable* variable)
{
	clear ();

	m_valueKind = ValueKind_Variable;
	m_llvmValue = variable->getLlvmValue ();
	m_variable = variable;
	m_type = getDirectRefType (
		variable->getType (),
		variable->getPtrTypeFlags () | PtrTypeFlag_Safe
		);
}

void
Value::setFunction (Function* function)
{
	clear ();

	m_valueKind = ValueKind_Function;
	m_function = function;
	m_type = function->getType ()->getFunctionPtrType (
		TypeKind_FunctionRef,
		FunctionPtrTypeKind_Thin,
		PtrTypeFlag_Safe
		);

	if (!function->isVirtual ())
		m_llvmValue = function->getLlvmFunction ();
}

void
Value::setFunctionTypeOverload (FunctionTypeOverload* functionTypeOverload)
{
	clear ();

	m_valueKind = functionTypeOverload->isOverloaded () ? ValueKind_FunctionTypeOverload : ValueKind_Type;
	m_functionTypeOverload = functionTypeOverload;
	m_type = functionTypeOverload->getOverload (0);
}

void
Value::setProperty (Property* prop)
{
	clear ();

	m_valueKind = ValueKind_Property;
	m_property = prop;
	m_type = prop->getType ()->getPropertyPtrType (
		TypeKind_PropertyRef,
		PropertyPtrTypeKind_Thin,
		PtrTypeFlag_Safe
		);

	// don't assign LlvmValue yet cause property LlvmValue is only needed for pointers
}

void
Value::setEnumConst (EnumConst* enumConst)
{
	EnumType* enumType;
	uint64_t enumValue;

	enumType = enumConst->getParentEnumType ();
	enumValue = enumConst->getValue ();

	if (enumType->getBaseType ()->getTypeKindFlags () & TypeKindFlag_BigEndian)
	{
		enumValue = sl::swapByteOrder64 (enumValue);
			
		size_t size = enumType->getSize ();
		if (size < 8)
			enumValue >>= (8 - size) * 8; 
	}

	createConst (&enumValue, enumType);
}

void
Value::setField (
	StructField* field,
	Type* type,
	size_t baseOffset
	)
{
	clear ();

	m_valueKind = ValueKind_Field;
	m_field = field;
	m_type = type;
	m_constData.setCount (sizeof (size_t));
	*(size_t*) m_constData.p () = baseOffset + field->getOffset ();
}

void
Value::setField (
	StructField* field,
	size_t baseOffset
	)
{
	setField (field, field->getType (), baseOffset);
}

void
Value::setLlvmValue (
	llvm::Value* llvmValue,
	Type* type,
	ValueKind valueKind
	)
{
	clear ();

	m_valueKind = valueKind;
	m_type = type;
	m_llvmValue = llvmValue;
}

LeanDataPtrValidator*
Value::getLeanDataPtrValidator () const
{
	if (m_leanDataPtrValidator)
		return m_leanDataPtrValidator;

	ASSERT (m_valueKind == ValueKind_Variable);
	m_leanDataPtrValidator = m_variable->getLeanDataPtrValidator ();
	return m_leanDataPtrValidator;
}

void
Value::setLeanDataPtrValidator (LeanDataPtrValidator* validator)
{
	ASSERT (isDataPtrType (m_type, DataPtrTypeKind_Lean));
	m_leanDataPtrValidator = validator;
}

void
Value::setLeanDataPtrValidator (const Value& originValue)
{
	ASSERT (isDataPtrType (m_type, DataPtrTypeKind_Lean));

	if (originValue.m_leanDataPtrValidator)
	{
		m_leanDataPtrValidator = originValue.m_leanDataPtrValidator;
	}
	else if (originValue.m_valueKind == ValueKind_Variable)
	{
		m_leanDataPtrValidator = originValue.m_variable->getLeanDataPtrValidator ();
	}
	else
	{
		m_leanDataPtrValidator = AXL_REF_NEW (LeanDataPtrValidator);
		m_leanDataPtrValidator->m_originValue = originValue;
	}
}

void
Value::setLeanDataPtrValidator (
	const Value& originValue,
	const Value& rangeBeginValue,
	size_t rangeLength
	)
{
	ASSERT (isDataPtrType (m_type, DataPtrTypeKind_Lean));

	ref::Ptr <LeanDataPtrValidator> validator = AXL_REF_NEW (LeanDataPtrValidator);
	validator->m_originValue = originValue;
	validator->m_rangeBeginValue = rangeBeginValue;
	validator->m_rangeLength = rangeLength;
	m_leanDataPtrValidator = validator;
}

bool
Value::createConst (
	const void* p,
	Type* type
	)
{
	clear ();

	size_t size = type->getSize ();
	size_t allocSize = AXL_MAX (size, sizeof (int64_t)); // ensure int64 for GetLlvmConst ()

	bool result = m_constData.setCount (allocSize);
	if (!result)
		return false;

	m_valueKind = ValueKind_Const;
	m_type = type;

	if (p)
		memcpy (m_constData, p, size);
	else
		memset (m_constData, 0, size);

	return true;
}

void
Value::setCharArray (
	const void* p,
	size_t size,
	Module* module
	)
{
	if (!size)
		size = 1;

	Type* type = module->m_typeMgr.getArrayType (
		module->m_typeMgr.getPrimitiveType (TypeKind_Char),
		size
		);

	createConst (p, type);
}

//..............................................................................

} // namespace ct
} // namespace jnc
