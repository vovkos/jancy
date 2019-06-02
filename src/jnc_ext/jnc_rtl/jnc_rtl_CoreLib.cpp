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
#include "jnc_rtl_DynamicLib.h"
#include "jnc_rtl_DynamicLayout.h"
#include "jnc_rtl_Multicast.h"
#include "jnc_rtl_Reactor.h"
#include "jnc_rtl_Regex.h"
#include "jnc_rtl_Promise.h"
#include "jnc_rt_Runtime.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_ArrayType.h"
#include "jnc_ct_MulticastClassType.h"
#include "jnc_CallSite.h"

#define JNC_MAP_STD_FUNCTION(stdFuncKind, proc) \
	if (module->m_functionMgr.isStdFunctionUsed(stdFuncKind)) \
	{ \
		function = module->m_functionMgr.getStdFunction(stdFuncKind); \
		ASSERT(function); \
		JNC_MAP_FUNCTION_IMPL(function, proc); \
	}

#define JNC_MAP_STD_TYPE(stdType, Type) \
	if (module->m_typeMgr.isStdTypeUsed(stdType)) \
	{ \
		JNC_MAP_TYPE(Type); \
	}

namespace jnc {
namespace rtl {

//..............................................................................

size_t
dynamicSizeOf(DataPtr ptr)
{
	if (!ptr.m_validator)
		return 0;

	char* p = (char*)ptr.m_p;
	char* end = (char*)ptr.m_validator->m_rangeEnd;
	return p < end ? end - p : 0;
}

size_t
dynamicCountOf(
	DataPtr ptr,
	Type* type
	)
{
	size_t maxSize = dynamicSizeOf(ptr);
	size_t typeSize = type->getSize();
	return maxSize / (typeSize ? typeSize : 1);
}

DataPtr
dynamicCastDataPtr(
	DataPtr ptr,
	Type* type
	)
{
	if (!ptr.m_validator)
		return g_nullDataPtr;

	Box* box = ptr.m_validator->m_targetBox;
	void* p =
		(box->m_type->getTypeKind() == TypeKind_Class) ? box + 1 :
		(box->m_flags & BoxFlag_Detached) ? ((DetachedDataBox*)box)->m_p :
		((DataBox*)box + 1);

	if (ptr.m_p < p)
		return g_nullDataPtr;

	Type* srcType = box->m_type;
	while (srcType->getTypeKind() == TypeKind_Array)
	{
		ArrayType* arrayType = (ArrayType*)srcType;
		srcType = arrayType->getElementType();

		size_t srcTypeSize = srcType->getSize();
		if (!srcTypeSize)
			srcTypeSize = 1;

		size_t offset = ((char*)ptr.m_p - (char*)p) % srcTypeSize;
		p = (char*)ptr.m_p - offset;
	}

	if (srcType->cmp(type) == 0)
	{
		ptr.m_p = p;
		return ptr;
	}

	AXL_TODO("find field pointed to by ptr and do cast accordingly")

	if (srcType->getTypeKind() != TypeKind_Struct)
		return g_nullDataPtr;

	size_t offset = ((StructType*)srcType)->findBaseTypeOffset(type);
	if (offset == -1)
		return g_nullDataPtr;

	ptr.m_p = (char*)p + offset;
	return ptr;
}

IfaceHdr*
dynamicCastClassPtr(
	IfaceHdr* iface,
	ClassType* type
	)
{
	if (!iface)
		return NULL;

	ASSERT(iface->m_box->m_type->getTypeKind() == TypeKind_Class);
	ClassType* classType = (ClassType*)iface->m_box->m_type;
	if (classType->cmp(type) == 0)
		return iface;

	size_t offset = classType->findBaseTypeOffset(type);
	if (offset == -1)
		return NULL;

	IfaceHdr* iface2 = (IfaceHdr*)((uchar_t*)(iface->m_box + 1) + offset);
	ASSERT(iface2->m_box == iface->m_box);
	return iface2;
}

bool
dynamicCastVariant(
	Variant variant,
	Type* type,
	void* buffer
	)
{
	return variant.cast(type, buffer);
}

IfaceHdr*
strengthenClassPtr(IfaceHdr* iface)
{
	return jnc_strengthenClassPtr(iface);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
primeStaticClass(
	Box* box,
	ClassType* type
	)
{
	primeClass(box, type);
	box->m_flags |= BoxFlag_Static;
}

IfaceHdr*
tryAllocateClass(ClassType* type)
{
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	return gcHeap->tryAllocateClass(type);
}

IfaceHdr*
allocateClass(ClassType* type)
{
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	return gcHeap->allocateClass(type);
}

DataPtr
tryAllocateData(Type* type)
{
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	return gcHeap->tryAllocateData(type);
}

DataPtr
allocateData(Type* type)
{
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	return gcHeap->allocateData(type);
}

DataPtr
tryAllocateArray(
	Type* type,
	size_t elementCount
	)
{
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	return gcHeap->tryAllocateArray(type, elementCount);
}

DataPtr
allocateArray(
	Type* type,
	size_t elementCount
	)
{
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	return gcHeap->allocateArray(type, elementCount);
}

DataPtrValidator*
createDataPtrValidator(
	Box* box,
	void* rangeBegin,
	size_t rangeLength
	)
{
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	return gcHeap->createDataPtrValidator(box, rangeBegin, rangeLength);
}

void
gcSafePoint()
{
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	gcHeap->safePoint();
}

void
setGcShadowStackFrameMap(
	GcShadowStackFrame* frame,
	GcShadowStackFrameMap* map,
	GcShadowStackFrameMapOp op
	)
{
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	gcHeap->setFrameMap(frame, map, op);
}

void
addStaticDestructor(StaticDestructFunc* destructFunc)
{
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	gcHeap->addStaticDestructor(destructFunc);
}

void
addStaticClassDestructor(
	DestructFunc* destructFunc,
	IfaceHdr* iface
	)
{
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	gcHeap->addStaticClassDestructor(destructFunc, iface);
}

void*
getTls()
{
	Tls* tls = getCurrentThreadTls();
	ASSERT(tls);

	return tls + 1;
}

void
dynamicThrow()
{
	jnc::dynamicThrow();
	ASSERT(false);
}

void
asyncRet(
	IfaceHdr* promise,
	Variant result
	)
{
	// jnc.Promisifier ONLY uses jnc.Promise fields to complete the promise
	// so it's OK to cast -- even though the actual class is NOT jnc.Promisifier

	((Promisifier*)promise)->complete_2(result, g_nullDataPtr);
}

void
asyncThrow(IfaceHdr* promise)
{
	err::Error error = err::getLastError();

	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	DataPtr errorPtr = gcHeap->tryAllocateBuffer(error->m_size);
	if (errorPtr.m_p)
		memcpy(errorPtr.m_p, error, error->m_size);

	((Promisifier*)promise)->complete_2(g_nullVariant, errorPtr);
}

jnc_Variant
variantUnaryOperator(
	int opKind,
	jnc_Variant variant
	)
{
	jnc_Variant result = jnc::g_nullVariant;
	variant.unaryOperator((jnc_UnOpKind)opKind, &result);
	return result;
}

jnc_Variant
variantBinaryOperator(
	int opKind,
	jnc_Variant variant1,
	jnc_Variant variant2
	)
{
	jnc_Variant result = jnc::g_nullVariant;
	variant1.binaryOperator(&variant2, (jnc_BinOpKind)opKind, &result);
	return result;
}

bool
variantRelationalOperator(
	int opKind,
	jnc_Variant variant1,
	jnc_Variant variant2
	)
{
	bool result = false;
	variant1.relationalOperator(&variant2, (jnc_BinOpKind)opKind, &result);
	return result;
}

Variant
variantMemberOperator(
	jnc_Variant variant,
	const char* name
	)
{
	jnc_Variant result = jnc::g_nullVariant;
	variant.getMember(name, &result);
	return result;
}

Variant
variantIndexOperator(
	jnc_Variant variant,
	size_t index
	)
{
	jnc_Variant result = jnc::g_nullVariant;
	variant.getElement(index, &result);
	return result;
}

Variant
variantMemberProperty_get(
	DataPtr variantPtr,
	const char* name
	)
{
	jnc_Variant result = jnc::g_nullVariant;
	Variant* variant = (Variant*)variantPtr.m_p;
	variant->getMember(name, &result);
	return result;
}

void
variantMemberProperty_set(
	DataPtr variantPtr,
	const char* name,
	Variant value
	)
{
	Variant* variant = (Variant*)variantPtr.m_p;
	variant->setMember(name, value);
}

Variant
variantIndexProperty_get(
	DataPtr variantPtr,
	size_t index
	)
{
	jnc_Variant result = jnc::g_nullVariant;
	Variant* variant = (Variant*)variantPtr.m_p;
	variant->getElement(index, &result);
	return result;
}

void
variantIndexProperty_set(
	DataPtr variantPtr,
	size_t index,
	Variant value
	)
{
	Variant* variant = (Variant*)variantPtr.m_p;
	variant->setElement(index, value);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
assertionFailure(
	const char* fileName,
	int line,
	const char* condition,
	const char* message
	)
{
	sl::String string;
	string.format("%s(%d): assertion (%s) failed", fileName, line + 1, condition);
	if (message)
		string.appendFormat("; %s", message);

	err::setError(string);
	dynamicThrow();
}

bool
tryCheckDataPtrRangeDirect(
	const void* p,
	const void* rangeBegin,
	size_t rangeLength
	)
{
	if (!p)
	{
		err::setError("null data pointer access");
		return false;
	}

	void* rangeEnd = (char*)rangeBegin + rangeLength;
	if (p < rangeBegin ||  p > rangeEnd)
	{
		err::setFormatStringError("data pointer %x out of range [%x:%x]", p, rangeBegin, rangeEnd);
		return false;
	}

	return true;
}

void
checkDataPtrRangeDirect(
	const void* p,
	const void* rangeBegin,
	size_t rangeLength
	)
{
	bool result = tryCheckDataPtrRangeDirect(p, rangeBegin, rangeLength);
	if (!result)
		dynamicThrow();
}

bool
tryCheckDataPtrRangeIndirect(
	const void* p,
	size_t size,
	DataPtrValidator* validator
	)
{
	if (!p || !validator)
	{
		err::setError("null data pointer access");
		return false;
	}

	void* end = (char*)p + size;
	if (p < validator->m_rangeBegin || end > validator->m_rangeEnd)
	{
		err::setFormatStringError("data pointer %x out of range [%x:%x]", p, validator->m_rangeBegin, validator->m_rangeEnd);
		return false;
	}

	return true;
}

void
checkDataPtrRangeIndirect(
	const void* p,
	size_t size,
	DataPtrValidator* validator
	)
{
	bool result = tryCheckDataPtrRangeIndirect(p, size, validator);
	if (!result)
		dynamicThrow();
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void*
tryLazyGetDynamicLibFunction(
	rtl::DynamicLib* lib,
	size_t index,
	const char* name
	)
{
	ASSERT(lib->m_box->m_type->getTypeKind() == TypeKind_Class);
	ClassType* type = (ClassType*)lib->m_box->m_type;

	if (!lib->m_handle)
	{
		err::setFormatStringError("dynamiclib '%s' is not loaded yet", type->getQualifiedName().sz());
		return NULL;
	}

	size_t librarySize = type->getIfaceStructType()->getSize();
	size_t functionCount = (librarySize - sizeof(DynamicLib)) / sizeof(void*);

	if (index >= functionCount)
	{
		err::setFormatStringError("index #%d out of range for dynamiclib '%s'", index, type->getQualifiedName().sz());
		return NULL;
	}

	void** functionTable = (void**) (lib + 1);
	if (functionTable[index])
		return functionTable[index];

	void* function = lib->getFunctionImpl(name);
	if (!function)
		return NULL;

	functionTable[index] = function;
	return function;
}

void*
lazyGetDynamicLibFunction(
	rtl::DynamicLib* lib,
	size_t index,
	const char* name
	)
{
	void* p = tryLazyGetDynamicLibFunction(lib, index, name);
	if (!p)
		dynamicThrow();

	return p;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

rtl::DynamicLayout*
getDynamicLayout(DataPtr ptr)
{
	if (!ptr.m_p || !ptr.m_validator)
	{
		err::setError("null data pointer access");
		dynamicThrow();
	}

	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	IfaceHdr* iface = gcHeap->getDynamicLayout(ptr.m_validator->m_targetBox);
	ASSERT(iface->m_box->m_type->getStdType() == StdType_DynamicLayout);

	return (rtl::DynamicLayout*)iface;
}

size_t
getDynamicFieldOffset(
	DataPtr ptr,
	DerivableType* type,
	StructField* field
	)
{
	ASSERT	(type->getFlags() & TypeFlag_Dynamic);

	if (type->getTypeKind() != TypeKind_Struct)
	{
		err::setError("only dynamic structs are currently supported");
		dynamicThrow();
	}

	StructType* structType = (StructType*)type;

	size_t offset;
	size_t prevIndex;

	if (field)
	{
		offset = field->getOffset();
		prevIndex = field->getPrevDynamicFieldIndex();
		if (prevIndex == -1)
			return offset;
	}
	else
	{
		field = structType->getMemberFieldArray().getBack();

		if (field->getType()->getFlags() & TypeFlag_Dynamic)
		{
			offset = 0;
			prevIndex = structType->getDynamicFieldArray().getCount() - 1;
		}
		else
		{
			offset = field->getOffset() + field->getType()->getSize();
			prevIndex = field->getPrevDynamicFieldIndex();
		}
	}

	rtl::DynamicLayout* dynamicLayout = getDynamicLayout(ptr);
	offset += dynamicLayout->getDynamicFieldEndOffset(ptr, structType, prevIndex);
	return offset;
}

void*
getDynamicField(
	DataPtr ptr,
	DerivableType* type,
	StructField* field
	)
{
	return (char*)ptr.m_p + getDynamicFieldOffset(ptr, type, field);
}

size_t
dynamicTypeSizeOf(
	DataPtr ptr,
	DerivableType* type
	)
{
	return getDynamicFieldOffset(ptr, type, NULL);
}

size_t
dynamicFieldSizeOf(
	DataPtr ptr,
	DerivableType* type,
	StructField* field
	)
{
	ASSERT(type->getFlags() & TypeFlag_Dynamic);
	ASSERT(field->getType()->getFlags() & TypeFlag_Dynamic);

	rtl::DynamicLayout* dynamicLayout = getDynamicLayout(ptr);
	size_t dynamicFieldIndex = field->getPrevDynamicFieldIndex() + 1;

	size_t beginOffset = getDynamicFieldOffset(ptr, type, field);
	size_t endOffset = dynamicLayout->getDynamicFieldEndOffset(ptr, type, dynamicFieldIndex);

	return endOffset - beginOffset;
}

size_t
dynamicFieldCountOf(
	DataPtr ptr,
	DerivableType* type,
	StructField* field
	)
{
	ASSERT(field->getType()->getTypeKind() == TypeKind_Array);
	ArrayType* arrayType = (ArrayType*)field->getType();

	size_t size = dynamicFieldSizeOf(ptr, type, field);
	return size / arrayType->getElementType()->getSize();
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

size_t
appendFmtLiteral_a(
	FmtLiteral* fmtLiteral,
	const char* p,
	size_t length
	)
{
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	size_t newLength = fmtLiteral->m_length + length;
	if (newLength < 64)
		newLength = 64;

	if (fmtLiteral->m_maxLength < newLength)
	{
		size_t newMaxLength = sl::getAllocSize(newLength);

		DataPtr ptr = gcHeap->tryAllocateBuffer(newMaxLength + 1);
		if (!ptr.m_p)
			return fmtLiteral->m_length;

		if (fmtLiteral->m_length)
			memcpy(ptr.m_p, fmtLiteral->m_ptr.m_p, fmtLiteral->m_length);

		fmtLiteral->m_ptr = ptr;
		fmtLiteral->m_maxLength = newMaxLength;
	}

	char* dst = (char*)fmtLiteral->m_ptr.m_p;
	memcpy(dst + fmtLiteral->m_length, p, length);
	fmtLiteral->m_length += length;
	dst[fmtLiteral->m_length] = 0;

	// adjust validator

	DataPtrValidator* validator = fmtLiteral->m_ptr.m_validator;
	validator->m_rangeEnd = (char*)validator->m_rangeBegin + fmtLiteral->m_length;
	return fmtLiteral->m_length;
}

static
void
prepareFormatString(
	sl::String* formatString,
	const char* fmtSpecifier,
	const char* defaultType
	)
{
	if (!fmtSpecifier)
	{
		formatString->copy('%');
		formatString->append(defaultType);
		return;
	}

	formatString->clear();

	if (fmtSpecifier[0] != '%')
		formatString->copy('%');

	formatString->append(fmtSpecifier);

	size_t length = formatString->getLength();
	if (!isalpha(formatString->sz() [length - 1]))
		formatString->append(defaultType);
}

inline
size_t
appendFmtLiteralDirect_va(
	FmtLiteral* fmtLiteral,
	const char* formatString,
	axl_va_list va
	)
{
	char buffer2[256];
	sl::String string(ref::BufKind_Stack, buffer2, sizeof(buffer2));
	string.format_va(formatString, va);

	return appendFmtLiteral_a(fmtLiteral, string, string.getLength());
}

inline
size_t
appendFmtLiteralDirect(
	FmtLiteral* fmtLiteral,
	const char* formatString,
	...
	)
{
	AXL_VA_DECL(va, formatString);
	return appendFmtLiteralDirect_va(fmtLiteral, formatString, va);
}

inline
size_t
appendFmtLiteralImpl(
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	const char* defaultType,
	...
	)
{
	AXL_VA_DECL(va, defaultType);

	char buffer1[256];
	sl::String formatString(ref::BufKind_Stack, buffer1, sizeof(buffer1));
	prepareFormatString(&formatString, fmtSpecifier, defaultType);

	return appendFmtLiteralDirect_va(fmtLiteral, formatString, va);
}

static
size_t
appendFmtLiteralStringImpl(
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	const char* p,
	size_t length
	)
{
	if (!fmtSpecifier)
		return appendFmtLiteral_a(fmtLiteral, p, length);

	char buffer[256];
	sl::String string(ref::BufKind_Stack, buffer, sizeof(buffer));

	if (p[length] != 0) // ensure zero-terminated
	{
		string.copy(p, length);
		p = string;
	}

	return appendFmtLiteralImpl(fmtLiteral, fmtSpecifier, "s", p);
}

static
size_t
appendFmtLiteral_p(
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	DataPtr ptr
	)
{
	return appendFmtLiteralStringImpl(fmtLiteral, fmtSpecifier, (const char*) ptr.m_p, strLen(ptr));
}

static
size_t
appendFmtLiteral_i32(
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	int32_t x
	)
{
	return appendFmtLiteralImpl(fmtLiteral, fmtSpecifier, "d", x);
}

static
size_t
appendFmtLiteral_ui32(
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	uint32_t x
	)
{
	return appendFmtLiteralImpl(fmtLiteral, fmtSpecifier, "u", x);
}

static
size_t
appendFmtLiteral_i64(
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	int64_t x
	)
{
	return appendFmtLiteralImpl(fmtLiteral, fmtSpecifier, "lld", x);
}

static
size_t
appendFmtLiteral_ui64(
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	uint64_t x
	)
{
	return appendFmtLiteralImpl(fmtLiteral, fmtSpecifier, "llu", x);
}

static
size_t
appendFmtLiteral_f(
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	double x
	)
{
	return appendFmtLiteralImpl(fmtLiteral, fmtSpecifier, "f", x);
}

size_t
appendFmtLiteral_v(
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	Variant variant
	)
{
	bool result;

	if (!variant.m_type)
		return fmtLiteral->m_length;

	TypeKind typeKind = variant.m_type->getTypeKind();
	uint_t typeKindFlags = variant.m_type->getTypeKindFlags();

	if (typeKindFlags & TypeKindFlag_Integer)
	{
		Module* module = variant.m_type->getModule();

		char buffer[sizeof(int64_t)];

		if (variant.m_type->getSize() > 4)
		{
			Type* targetType = module->m_typeMgr.getPrimitiveType(TypeKind_Int64);
			result = variant.cast(targetType, buffer);
			if (!result)
			{
				ASSERT(false);
				return fmtLiteral->m_length;
			}

			int64_t x = *(int64_t*)buffer;

			return (typeKindFlags & TypeKindFlag_Unsigned) ?
				appendFmtLiteral_ui64(fmtLiteral, fmtSpecifier, x) :
				appendFmtLiteral_i64(fmtLiteral, fmtSpecifier, x);
		}
		else
		{
			Type* targetType = module->m_typeMgr.getPrimitiveType(TypeKind_Int32);
			result = variant.cast(targetType, buffer);
			if (!result)
			{
				ASSERT(false);
				return fmtLiteral->m_length;
			}

			int32_t x = *(int32_t*)buffer;

			return (typeKindFlags & TypeKindFlag_Unsigned) ?
				appendFmtLiteral_ui32(fmtLiteral, fmtSpecifier, x) :
				appendFmtLiteral_i32(fmtLiteral, fmtSpecifier, x);
		}
	}
	else if (typeKindFlags & TypeKindFlag_Fp)
	{
		return typeKind == TypeKind_Float ?
			appendFmtLiteral_f(fmtLiteral, fmtSpecifier, *(float*) &variant) :
			appendFmtLiteral_f(fmtLiteral, fmtSpecifier, *(double*) &variant);
	}

	Type* type;
	const void* p;

	if (typeKind != TypeKind_DataRef)
	{
		type = variant.m_type;
		p = &variant;
	}
	else
	{
		type = ((DataPtrType*)variant.m_type)->getTargetType();
		p = variant.m_dataPtr.m_p;
	}

	if (isCharArrayType(type))
	{
		ArrayType* arrayType = (ArrayType*)type;
		size_t count = arrayType->getElementCount();
		const char* c = (char*)p;

		// trim zero-termination

		while (count && c[count - 1] == 0)
			count--;

		return appendFmtLiteralStringImpl(fmtLiteral, fmtSpecifier, c, count);
	}
	else if (type->getTypeKindFlags() & TypeKindFlag_Ptr)
	{
		if (isCharPtrType(type))
		{
			DataPtrType* ptrType = (DataPtrType*)type;
			DataPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind();

			if (ptrTypeKind == DataPtrTypeKind_Normal)
				return appendFmtLiteral_p(fmtLiteral, fmtSpecifier, *(DataPtr*) &variant);

			const char* c = *(char**) p;
			size_t length = strlen_s(c);

			return appendFmtLiteralStringImpl(fmtLiteral, fmtSpecifier, c, length);
		}
		else // generic pointer
		{
			return appendFmtLiteralDirect(fmtLiteral, "%p", variant.m_p);
		}
	}
	else // don't know how to format
	{
		return appendFmtLiteralDirect(fmtLiteral, "(variant:%s)", type->getTypeString().sz());
	}
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

Promise*
getCurrentPromise()
{
	return NULL;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
multicastDestruct(Multicast* multicast)
{
	((MulticastImpl*)multicast)->destruct();
}

void
multicastClear(Multicast* multicast)
{
	return ((MulticastImpl*)multicast)->clear();
}

handle_t
multicastSet(
	Multicast* multicast,
	FunctionPtr ptr
	)
{
	return ((MulticastImpl*)multicast)->setHandler(ptr);
}

handle_t
multicastSet_t(
	Multicast* multicast,
	void* p
	)
{
	return ((MulticastImpl*)multicast)->setHandler_t(p);
}

handle_t
multicastAdd(
	Multicast* multicast,
	FunctionPtr ptr
	)
{
	return ((MulticastImpl*)multicast)->addHandler(ptr);
}

handle_t
multicastAdd_t(
	Multicast* multicast,
	void* p
	)
{
	return ((MulticastImpl*)multicast)->addHandler_t(p);
}

FunctionPtr
multicastRemove(
	Multicast* multicast,
	handle_t handle
	)
{
	return ((MulticastImpl*)multicast)->removeHandler(handle);
}

void*
multicastRemove_t(
	Multicast* multicast,
	handle_t handle
	)
{
	return ((MulticastImpl*)multicast)->removeHandler_t(handle);
}

FunctionPtr
multicastGetSnapshot(Multicast* multicast)
{
	return ((MulticastImpl*)multicast)->getSnapshot();
}

static
void
mapMulticastMethods(
	Module* module,
	const MulticastClassType* multicastType
	)
{
	static void* multicastMethodTable[FunctionPtrTypeKind__Count][MulticastMethodKind__Count - 1] =
	{
		{
			(void*)multicastClear,
			(void*)multicastSet,
			(void*)multicastAdd,
			(void*)multicastRemove,
			(void*)multicastGetSnapshot,
		},

		{
			(void*)multicastClear,
			(void*)multicastSet,
			(void*)multicastAdd,
			(void*)multicastRemove,
			(void*)multicastGetSnapshot,
		},

		{
			(void*)multicastClear,
			(void*)multicastSet_t,
			(void*)multicastAdd_t,
			(void*)multicastRemove_t,
			(void*)multicastGetSnapshot,
		},
	};

	FunctionPtrTypeKind ptrTypeKind = multicastType->getTargetType()->getPtrTypeKind();
	ASSERT(ptrTypeKind < FunctionPtrTypeKind__Count);

	Function* function = multicastType->getDestructor();
	module->mapFunction(function, (void*)multicastDestruct);

	for (size_t i = 0; i < MulticastMethodKind__Count - 1; i++)
	{
		function = multicastType->getMethod((MulticastMethodKind)i);
		module->mapFunction(function, multicastMethodTable[ptrTypeKind][i]);
	}
}

bool
mapAllMulticastMethods(Module* module)
{
	const sl::Array<ct::MulticastClassType*> mcTypeArray = module->m_typeMgr.getMulticastClassTypeArray();
	size_t count = mcTypeArray.getCount();
	for (size_t i = 0; i < count; i++)
		mapMulticastMethods(module, mcTypeArray[i]);

	return true;
}

//..............................................................................

// {CFD9EA7A-35DE-4090-A83B-3D214B3FF358}
JNC_DEFINE_GUID(
	jnc_g_coreLibGuid,
	0xcfd9ea7a, 0x35de, 0x4090, 0xa8, 0x3b, 0x3d, 0x21, 0x4b, 0x3f, 0xf3, 0x58
	);

JNC_DEFINE_LIB(
	jnc_CoreLib,
	jnc_g_coreLibGuid,
	"CoreLib",
	"Jancy core extension library (RTL)"
	)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE(jnc_CoreLib)
JNC_END_LIB_SOURCE_FILE_TABLE()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE(jnc_CoreLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(RegexState)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(RegexDfa)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(DynamicLayout)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(ReactorImpl)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Promise)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE()

JNC_BEGIN_LIB_FUNCTION_MAP(jnc_CoreLib)
	// dynamic sizeof/countof/casts

	JNC_MAP_STD_FUNCTION(ct::StdFunc_DynamicSizeOf,       dynamicSizeOf)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_DynamicCountOf,      dynamicCountOf)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_DynamicTypeSizeOf,   dynamicTypeSizeOf)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_DynamicFieldSizeOf,  dynamicFieldSizeOf)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_DynamicFieldCountOf, dynamicFieldCountOf)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_DynamicCastDataPtr,  dynamicCastDataPtr)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_DynamicCastClassPtr, dynamicCastClassPtr)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_DynamicCastVariant,  dynamicCastVariant)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_StrengthenClassPtr,  strengthenClassPtr)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_GetDynamicField,     getDynamicField)

	// gc heap

	JNC_MAP_STD_FUNCTION(ct::StdFunc_PrimeStaticClass,         primeStaticClass)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_TryAllocateClass,         tryAllocateClass)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_AllocateClass,            allocateClass)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_TryAllocateData,          tryAllocateData)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_AllocateData,             allocateData)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_TryAllocateArray,         tryAllocateArray)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_AllocateArray,            allocateArray)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_CreateDataPtrValidator,   createDataPtrValidator)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_GcSafePoint,              gcSafePoint)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_SetGcShadowStackFrameMap, setGcShadowStackFrameMap)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_AddStaticDestructor,      addStaticDestructor)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_AddStaticClassDestructor, addStaticClassDestructor)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_GetTls,                   getTls)

	// variant operators

	JNC_MAP_STD_FUNCTION(ct::StdFunc_VariantUnaryOperator,      variantUnaryOperator)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_VariantBinaryOperator,     variantBinaryOperator)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_VariantRelationalOperator, variantRelationalOperator)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_VariantMemberOperator,     variantMemberOperator)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_VariantIndexOperator,      variantIndexOperator)

	JNC_MAP_STD_FUNCTION(ct::StdFunc_VariantMemberProperty_get, variantMemberProperty_get)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_VariantMemberProperty_set, variantMemberProperty_set)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_VariantIndexProperty_get,  variantIndexProperty_get)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_VariantIndexProperty_set,  variantIndexProperty_set)

	// exceptions/async

	JNC_MAP_STD_FUNCTION(ct::StdFunc_SetJmp,       ::setjmp)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_DynamicThrow, dynamicThrow)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_AsyncRet,     asyncRet)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_AsyncThrow,   asyncThrow)

	// runtime checks

	JNC_MAP_STD_FUNCTION(ct::StdFunc_AssertionFailure,             assertionFailure)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_TryCheckDataPtrRangeDirect,   tryCheckDataPtrRangeDirect)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_CheckDataPtrRangeDirect,      checkDataPtrRangeDirect)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_TryCheckDataPtrRangeIndirect, tryCheckDataPtrRangeIndirect)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_CheckDataPtrRangeIndirect,    checkDataPtrRangeIndirect)

	// dynamic libs

	JNC_MAP_STD_FUNCTION(ct::StdFunc_TryLazyGetDynamicLibFunction, tryLazyGetDynamicLibFunction)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_LazyGetDynamicLibFunction,    lazyGetDynamicLibFunction)

	// formating literals

	JNC_MAP_STD_FUNCTION(ct::StdFunc_AppendFmtLiteral_a,    appendFmtLiteral_a)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_AppendFmtLiteral_p,    appendFmtLiteral_p)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_AppendFmtLiteral_i32,  appendFmtLiteral_i32)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_AppendFmtLiteral_ui32, appendFmtLiteral_ui32)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_AppendFmtLiteral_i64,  appendFmtLiteral_i64)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_AppendFmtLiteral_ui64, appendFmtLiteral_ui64)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_AppendFmtLiteral_f,    appendFmtLiteral_f)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_AppendFmtLiteral_v,    appendFmtLiteral_v)

	// multicasts

	result = mapAllMulticastMethods(module);
	if (!result)
		return false;

	// std types

	JNC_MAP_STD_TYPE(StdType_RegexState,    RegexState)
	JNC_MAP_STD_TYPE(StdType_RegexDfa,      RegexDfa)
	JNC_MAP_STD_TYPE(StdType_DynamicLib,    DynamicLib)
	JNC_MAP_STD_TYPE(StdType_DynamicLayout, DynamicLayout)
	JNC_MAP_STD_TYPE(StdType_ReactorBase,   ReactorImpl)
	JNC_MAP_STD_TYPE(StdType_Promise,       Promise)
	JNC_MAP_STD_TYPE(StdType_Promisifier,   Promisifier)

JNC_END_LIB_FUNCTION_MAP()

//..............................................................................

} // namespace rtl
} // namespace jnc
