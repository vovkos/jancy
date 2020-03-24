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
#include "jnc_Variant.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_ct_Module.h"
#	include "jnc_ct_ArrayType.h"
#	include "jnc_rt_Runtime.h"
#endif

#include "jnc_Runtime.h"

//..............................................................................

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_create(
	jnc_Variant* variant,
	const void* p,
	jnc_Type* type
	)
{
	using namespace jnc;

	size_t size = type->getSize();
	if (size <= sizeof(jnc::DataPtr))
	{
		memcpy(variant, p, size);
		variant->m_type = type;
		return true;
	}

	GcHeap* gcHeap = getCurrentThreadGcHeap();
	if (!gcHeap)
		return err::fail(err::Error("not inside Jancy call-site"));

	DataPtr ptr = gcHeap->tryAllocateData(type);
	if (!ptr.m_p)
		return false;

	memcpy(ptr.m_p, p, size);
	variant->m_type = type->getDataPtrType(TypeKind_DataRef, DataPtrTypeKind_Normal, PtrTypeFlag_Const);
	variant->m_dataPtr = ptr;
	return true;
}

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_cast(
	const jnc_Variant* variant,
	jnc_Type* type,
	void* buffer
	)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_castFunc(variant, type, buffer);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_unaryOperator(
	const jnc_Variant* variant,
	jnc_UnOpKind opKind,
	jnc_Variant* result
	)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_unaryOperatorFunc(variant, opKind, result);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_binaryOperator(
	const jnc_Variant* variant,
	const jnc_Variant* variant2,
	jnc_BinOpKind opKind,
	jnc_Variant* result
	)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_binaryOperatorFunc(variant, variant2, opKind, result);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_relationalOperator(
	const jnc_Variant* variant,
	const jnc_Variant* variant2,
	jnc_BinOpKind opKind,
	bool_t* result
	)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_relationalOperatorFunc(variant, variant2, opKind, result);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_getMember(
	const jnc_Variant* variant,
	const char* name,
	jnc_Variant* result
	)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_getMemberFunc(variant, name, result);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_setMember(
	jnc_Variant* variant,
	const char* name,
	jnc_Variant value
	)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_setMemberFunc(variant, name, value);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_getElement(
	const jnc_Variant* variant,
	size_t index,
	jnc_Variant* result
	)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_getElementFunc(variant, index, result);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_setElement(
	jnc_Variant* variant,
	size_t index,
	jnc_Variant value
	)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_setElementFunc(variant, index, value);
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_Variant_hash(const jnc_Variant* variant)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_hashFunc(variant);
}

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_Variant_format_v(
	const jnc_Variant* variant,
	const char* fmtSpecifier
	)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_formatFunc(variant, fmtSpecifier);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_cast(
	const jnc_Variant* variant,
	jnc_Type* type,
	void* buffer
	)
{
	using namespace jnc;
	ct::Module* module = type->getModule();

	ct::Value opValue(variant, module->m_typeMgr.getPrimitiveType(TypeKind_Variant));
	ct::CastOperator* castOp = module->m_operatorMgr.getStdCastOperator(ct::StdCast_FromVariant);

	memset(buffer, 0, type->getSize());
	return castOp->constCast(opValue, type, buffer);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_unaryOperator(
	const jnc_Variant* variant,
	jnc_UnOpKind opKind,
	jnc_Variant* resultVariant
	)
{
	using namespace jnc;
	ct::Value opValue;

	if (variant->m_type)
	{
		opValue.createConst(variant, variant->m_type);
	}
	else
	{
		*resultVariant = *variant;
		return true;
	}

	ct::Module* module = variant->m_type->getModule();

	ct::Value resultValue;
	bool result = module->m_operatorMgr.unaryOperator(
		opKind,
		opValue,
		&resultValue
		) &&
		module->m_operatorMgr.castOperator(&resultValue, TypeKind_Variant);

	if (!result)
		return false;

	*resultVariant = *(Variant*)resultValue.getConstData();
	return true;
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_binaryOperator(
	const jnc_Variant* variant,
	const jnc_Variant* variant2,
	jnc_BinOpKind opKind,
	jnc_Variant* resultVariant
	)
{
	using namespace jnc;
	ct::Value opValue1;

	if (variant->m_type)
	{
		opValue1.createConst(variant, variant->m_type);
	}
	else if (variant2->m_type)
	{
		opValue1.createConst(NULL, variant2->m_type);
	}
	else
	{
		*resultVariant = *variant;
		return true;
	}

	ct::Value opValue2;

	if (variant2->m_type)
	{
		opValue2.createConst(variant2, variant2->m_type);
	}
	else
	{
		ASSERT(variant->m_type);
		opValue2.createConst(NULL, variant->m_type);
	}

	ct::Module* module = opValue1.getType()->getModule();

	ct::Value resultValue;
	bool result = module->m_operatorMgr.binaryOperator(
		opKind,
		opValue1,
		opValue2,
		&resultValue
		) &&
		module->m_operatorMgr.castOperator(&resultValue, TypeKind_Variant);

	if (!result)
		return false;

	*resultVariant = *(Variant*)resultValue.getConstData();
	return true;
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_relationalOperator(
	const jnc_Variant* variant,
	const jnc_Variant* variant2,
	jnc_BinOpKind opKind,
	bool_t* resultBool
	)
{
	using namespace jnc;
	ASSERT(opKind >= BinOpKind_Eq && opKind <= BinOpKind_Ge);

	ct::Value opValue1;

	if (variant->m_type)
	{
		opValue1.createConst(variant, variant->m_type);
	}
	else if (variant2->m_type)
	{
		opValue1.createConst(NULL, variant2->m_type);
	}
	else
	{
		*resultBool = opKind == jnc_BinOpKind_Eq;
		return true;
	}

	ct::Value opValue2;

	if (variant2->m_type)
	{
		opValue2.createConst(variant2, variant2->m_type);
	}
	else
	{
		ASSERT(variant->m_type);
		opValue2.createConst(NULL, variant->m_type);
	}

	ct::Module* module = opValue1.getType()->getModule();

	ct::Value resultValue;
	bool result = module->m_operatorMgr.binaryOperator(
		opKind,
		opValue1,
		opValue2,
		&resultValue
		) &&
		module->m_operatorMgr.castOperator(&resultValue, TypeKind_Bool);

	if (result)
	{
		*resultBool = *(bool*)resultValue.getConstData();
		return true;
	}

	// try memcmp fallback for equality and inequality

	if ((opKind != jnc_BinOpKind_Eq && opKind != jnc_BinOpKind_Ne) ||
		variant->m_type->cmp(variant2->m_type) != 0)
		return false;

	const void* p1;
	const void* p2;
	size_t size;

	Type* type = variant->m_type;
	if (type->getTypeKind() == TypeKind_DataRef)
	{
		p1 = variant->m_p;
		p2 = variant2->m_p;
		size = ((ct::DataPtrType*)type)->getTargetType()->getSize();
	}
	else
	{
		p1 = variant;
		p2 = variant2;
		size = type->getSize();
	}

	bool isEqual = memcmp(p1, p2, size) == 0;
	*resultBool = isEqual == (opKind == jnc_BinOpKind_Eq);
	return true;
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_getMember(
	const jnc_Variant* variant,
	const char* name,
	jnc_Variant* resultVariant
	)
{
	using namespace jnc;

	if (!variant->m_type)
	{
		err::setError("cannot apply member operator to 'null'");
		return false;
	}

	ct::Module* module = variant->m_type->getModule();

	ct::Value opValue(variant, variant->m_type);
	ct::Value memberValue;
	bool result =
		module->m_operatorMgr.memberOperator(opValue, name, &memberValue) &&
		module->m_operatorMgr.castOperator(&memberValue, TypeKind_Variant);

	if (!result)
		return false;

	*resultVariant = *(Variant*)memberValue.getConstData();
	return true;
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_setMember(
	jnc_Variant* variant,
	const char* name,
	jnc_Variant valueVariant
	)
{
	using namespace jnc;

	if (!variant->m_type)
	{
		err::setError("cannot apply member operator to 'null'");
		return true;
	}

	ct::Module* module = variant->m_type->getModule();

	ct::Value opValue;
	if (variant->m_type->getTypeKindFlags() & TypeKindFlag_Ptr)
	{
		opValue.createConst(variant, variant->m_type);
	}
	else
	{
		ASSERT(variant->m_type->getSize() <= sizeof(DataPtr));
		opValue.createConst(&variant, variant->m_type->getDataPtrType_c(TypeKind_DataRef));
	}

	ct::Value opValue2(&valueVariant, module->m_typeMgr.getPrimitiveType(TypeKind_Variant));
	ct::Value memberValue;

	return
		module->m_operatorMgr.memberOperator(opValue, name, &memberValue) &&
		module->m_operatorMgr.binaryOperator(BinOpKind_Assign, memberValue, opValue2);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_getElement(
	const jnc_Variant* variant,
	size_t index,
	jnc_Variant* resultVariant
	)
{
	using namespace jnc;

	if (!variant->m_type)
	{
		err::setError("cannot apply index operator to 'null'");
		return true;
	}

	ct::Module* module = variant->m_type->getModule();

	// turning it into ref is only necessary because of current implementation of OperatorMgr::memberOperator (size_t)

	ct::Value opValue;
	if (variant->m_type->getTypeKindFlags() & TypeKindFlag_Ptr)
	{
		opValue.createConst(variant, variant->m_type);
	}
	else
	{
		ASSERT(variant->m_type->getSize() <= sizeof(DataPtr));
		opValue.createConst(&variant, variant->m_type->getDataPtrType_c(TypeKind_DataRef));
	}

	ct::Value memberValue;
	bool result =
		module->m_operatorMgr.memberOperator(opValue, index, &memberValue) &&
		module->m_operatorMgr.castOperator(&memberValue, TypeKind_Variant);

	if (!result)
		return false;

	*resultVariant = *(Variant*)memberValue.getConstData();
	return true;
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_setElement(
	jnc_Variant* variant,
	size_t index,
	jnc_Variant valueVariant
	)
{
	using namespace jnc;

	if (!variant->m_type)
	{
		err::setError("cannot apply index operator to 'null'");
		return true;
	}

	ct::Module* module = variant->m_type->getModule();

	ct::Value opValue;
	if (variant->m_type->getTypeKindFlags() & TypeKindFlag_Ptr)
	{
		opValue.createConst(variant, variant->m_type);
	}
	else
	{
		ASSERT(variant->m_type->getSize() <= sizeof(DataPtr));
		opValue.createConst(&variant, variant->m_type->getDataPtrType_c(TypeKind_DataRef));
	}

	ct::Value opValue2(&valueVariant, module->m_typeMgr.getPrimitiveType(TypeKind_Variant));
	ct::Value memberValue;

	return
		module->m_operatorMgr.memberOperator(opValue, index, &memberValue) &&
		module->m_operatorMgr.binaryOperator(BinOpKind_Assign, memberValue, opValue2);
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_Variant_hash(const jnc_Variant* variant)
{
	using namespace jnc;
	if (!variant->m_type)
		return 0;

	size_t size = variant->m_type->getSize();

	if (variant->m_type->getTypeKindFlags() & TypeKindFlag_BigEndian)
	{
		uint64_t result = 0;
		axl::sl::swapByteOrder(&result, &variant->m_int64, size);
		return (uintptr_t)result;
	}

	if (size <= sizeof(uintptr_t) || variant->m_type->getTypeKind() == TypeKind_DataPtr)
		return variant->m_uintptr;

	const void* p = variant;
	if (variant->m_type->getTypeKind() == TypeKind_DataRef)
	{
		p = variant->m_p;
		size = ((ct::DataPtrType*)variant->m_type)->getTargetType()->getSize();
	}

	return sl::djb2(p, size);
}

// part of RTL-core (appendFmtLiteral_xxx)

namespace jnc {
namespace rtl {

void
prepareFormatString(
	sl::String* formatString,
	const char* fmtSpecifier,
	const char* defaultType
	);

} // namespace rtl
} // namespace jnc

inline
size_t
formatImpl(
	sl::String* string,
	const char* fmtSpecifier,
	const char* defaultType,
	...
	)
{
	AXL_VA_DECL(va, defaultType);

	char buffer[256];
	sl::String formatString(ref::BufKind_Stack, buffer, sizeof(buffer));
	jnc::rtl::prepareFormatString(&formatString, fmtSpecifier, defaultType);

	return string->format_va(formatString, va);
}

static
size_t
formatStringImpl(
	sl::String* string,
	const char* fmtSpecifier,
	const char* p,
	size_t length
	)
{
	if (!fmtSpecifier)
		return string->copy(p, length);

	if (!p[length]) // already zero-terminated
		return formatImpl(string, fmtSpecifier, "s", p);

	// make a zero-terminated copy

	char buffer[256];
	sl::String string2(ref::BufKind_Stack, buffer, sizeof(buffer));
	string2.copy(p, length);
	return formatImpl(string, fmtSpecifier, "s", string2.sz());
}

static
size_t
formatCharPtr(
	sl::String* string,
	const char* fmtSpecifier,
	const jnc::DataPtr& ptr
	)
{
	if (!ptr.m_p) // shortcut
		return string->getLength();

	size_t length = jnc::strLen(ptr);
	return formatStringImpl(string, fmtSpecifier, (const char*)ptr.m_p, length);
}

JNC_EXTERN_C
size_t
jnc_Variant_format(
	const jnc_Variant* variant,
	sl::String* string,
	const char* fmtSpecifier
	)
{
	bool result;

	string->clear();

	if (!variant->m_type)
		return 0;

	jnc::TypeKind typeKind = variant->m_type->getTypeKind();
	uint_t typeKindFlags = variant->m_type->getTypeKindFlags();

	if (typeKindFlags & jnc::TypeKindFlag_Integer)
	{
		jnc::Module* module = variant->m_type->getModule();

		char buffer[sizeof(int64_t)];

		if (variant->m_type->getSize() > 4)
		{
			jnc::Type* targetType = module->m_typeMgr.getPrimitiveType(jnc::TypeKind_Int64);
			result = variant->cast(targetType, buffer);
			if (!result)
			{
				ASSERT(false);
				return 0;
			}

			const char* defaultType = (typeKindFlags & jnc::TypeKindFlag_Unsigned) ? "llu" : "lld";
			return formatImpl(string, fmtSpecifier, defaultType, *(int64_t*)buffer);
		}
		else
		{
			jnc::Type* targetType = module->m_typeMgr.getPrimitiveType(jnc::TypeKind_Int32);
			result = variant->cast(targetType, buffer);
			if (!result)
			{
				ASSERT(false);
				return 0;
			}

			const char* defaultType = (typeKindFlags & jnc::TypeKindFlag_Unsigned) ? "llu" : "lld";
			return formatImpl(string, fmtSpecifier,	defaultType, *(int32_t*)buffer);
		}
	}
	else if (typeKindFlags & jnc::TypeKindFlag_Fp)
	{
		double x = jnc::TypeKind_Float ? *(float*)&variant : *(double*)&variant;
		return formatImpl(string, fmtSpecifier, "f", x);
	}

	jnc::Type* type;
	const void* p;

	if (typeKind != jnc::TypeKind_DataRef)
	{
		type = variant->m_type;
		p = variant;
	}
	else
	{
		type = ((jnc::DataPtrType*)variant->m_type)->getTargetType();
		p = variant->m_dataPtr.m_p;
	}

	if (jnc::isCharArrayType(type))
	{
		jnc::ArrayType* arrayType = (jnc::ArrayType*)type;
		size_t count = arrayType->getElementCount();
		const char* c = (char*)p;

		// trim zero-termination

		while (count && c[count - 1] == 0)
			count--;

		return string->copy(c, count);
	}
	else if (type->getTypeKindFlags() & jnc::TypeKindFlag_Ptr)
	{
		if (jnc::isCharPtrType(type))
		{
			jnc::DataPtrType* ptrType = (jnc::DataPtrType*)type;
			jnc::DataPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind();

			if (ptrTypeKind == jnc::DataPtrTypeKind_Normal)
				return formatCharPtr(string, fmtSpecifier, variant->m_dataPtr);

			const char* c = *(char**)p;
			size_t length = strlen_s(c);
			return formatStringImpl(string, fmtSpecifier, c, length);
		}
		else // generic pointer
		{
			return string->format("%p", variant->m_p);
		}
	}
	else // don't know how to format
	{
		return string->format("(variant:%s)", type->getTypeString().sz());
	}

	return string->getLength();
}

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_Variant_format_v(
	const jnc_Variant* variant,
	const char* fmtSpecifier
	)
{
	sl::String* string = jnc::getTlsStringBuffer();
	jnc_Variant_format(variant, string, fmtSpecifier);
	return *string;
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................

bool
jnc::Variant::relationalOperator(
	const jnc_Variant* variant2,
	jnc_BinOpKind opKind,
	bool* result
	) const
{
	bool_t intResult;
	if (!jnc_Variant_relationalOperator(this, variant2, opKind, &intResult))
		return false;

	*result = intResult != 0;
	return true;
}

//..............................................................................
