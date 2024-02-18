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
#include "jnc_ct_Jit.h"
#include "jnc_ct_ArrayType.h"
#include "jnc_ct_MulticastClassType.h"
#include "jnc_CallSite.h"

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// jancy sources

;static char g_jnc_gcSrc[] =
#include "jnc_gc.jnc.cpp"
;static char g_jnc_dataPtrSrc[] =
#include "jnc_DataPtr.jnc.cpp"
;static char g_jnc_dynamicLibSrc[] =
#include "jnc_DynamicLib.jnc.cpp"
;static char g_jnc_promiseSrc[] =
#include "jnc_Promise.jnc.cpp"
;static char g_jnc_regexSrc[] =
#include "jnc_Regex.jnc.cpp"
;static char g_jnc_schedulerSrc[] =
#include "jnc_Scheduler.jnc.cpp"
;

namespace jnc {
namespace rtl {

//..............................................................................

size_t
dynamicSizeOf(DataPtr ptr) {
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
) {
	size_t maxSize = dynamicSizeOf(ptr);
	size_t typeSize = type->getSize();
	return maxSize / (typeSize ? typeSize : 1);
}

DataPtr
dynamicCastDataPtr(
	DataPtr ptr,
	Type* type
) {
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
	while (srcType->getTypeKind() == TypeKind_Array) {
		ArrayType* arrayType = (ArrayType*)srcType;
		srcType = arrayType->getElementType();

		size_t srcTypeSize = srcType->getSize();
		if (!srcTypeSize)
			srcTypeSize = 1;

		size_t offset = ((char*)ptr.m_p - (char*)p) % srcTypeSize;
		p = (char*)ptr.m_p - offset;
	}

	if (srcType->cmp(type) == 0) {
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
) {
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
) {
	return variant.cast(type, buffer);
}

IfaceHdr*
strengthenClassPtr(IfaceHdr* iface) {
	return jnc_strengthenClassPtr(iface);
}

void
resetDynamicLayout(DataPtr ptr) {
	if (!ptr.m_validator)
		return;

	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	gcHeap->resetDynamicLayout(ptr.m_validator->m_targetBox);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
primeStaticClass(
	Box* box,
	ClassType* type
) {
	primeClass(box, type);
	box->m_flags |= BoxFlag_Static;
}

IfaceHdr*
tryAllocateClass(ClassType* type) {
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	return gcHeap->tryAllocateClass(type);
}

IfaceHdr*
allocateClass(ClassType* type) {
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	return gcHeap->allocateClass(type);
}

DataPtr
tryAllocateData(Type* type) {
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	return gcHeap->tryAllocateData(type);
}

DataPtr
allocateData(Type* type) {
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	return gcHeap->allocateData(type);
}

DataPtr
tryAllocateArray(
	Type* type,
	size_t elementCount
) {
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	return gcHeap->tryAllocateArray(type, elementCount);
}

DataPtr
allocateArray(
	Type* type,
	size_t elementCount
) {
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	return gcHeap->allocateArray(type, elementCount);
}

DataPtrValidator*
createDataPtrValidator(
	Box* box,
	const void* rangeBegin,
	size_t rangeLength
) {
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	return gcHeap->createDataPtrValidator(box, rangeBegin, rangeLength);
}

void
gcSafePoint() {
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	gcHeap->safePoint();
}

void
setGcShadowStackFrameMap(
	GcShadowStackFrame* frame,
	GcShadowStackFrameMap* map,
	GcShadowStackFrameMapOp op
) {
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	gcHeap->setFrameMap(frame, map, op);
}

void
addStaticDestructor(StaticDestructFunc* destructFunc) {
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	gcHeap->addStaticDestructor(destructFunc);
}

void
addStaticClassDestructor(
	DestructFunc* destructFunc,
	IfaceHdr* iface
) {
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	gcHeap->addStaticClassDestructor(destructFunc, iface);
}

void*
getTls() {
	Tls* tls = getCurrentThreadTls();
	ASSERT(tls);

	return tls + 1;
}

void
dynamicThrow() {
	jnc::dynamicThrow();
	ASSERT(false);
}

void
asyncRet(
	IfaceHdr* promise,
	Variant result
) {
	// jnc.Promisifier ONLY uses jnc.Promise fields to complete the promise
	// so it's OK to cast -- even though the actual class is NOT jnc.Promisifier

	((PromiseImpl*)promise)->complete_2(result, g_nullDataPtr);
}

void
asyncThrow(IfaceHdr* promise) {
	err::Error error = err::getLastError();

	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	DataPtr errorPtr = gcHeap->tryAllocateBuffer(error->m_size);
	if (errorPtr.m_p)
		memcpy(errorPtr.m_p, error, error->m_size);

	((PromiseImpl*)promise)->complete_2(g_nullVariant, errorPtr);
}

jnc_Variant
variantUnaryOperator(
	int opKind,
	jnc_Variant variant
) {
	jnc_Variant result = jnc::g_nullVariant;
	variant.unaryOperator((jnc_UnOpKind)opKind, &result);
	return result;
}

jnc_Variant
variantBinaryOperator(
	int opKind,
	jnc_Variant variant1,
	jnc_Variant variant2
) {
	jnc_Variant result = jnc::g_nullVariant;
	variant1.binaryOperator(&variant2, (jnc_BinOpKind)opKind, &result);
	return result;
}

bool
variantRelationalOperator(
	int opKind,
	jnc_Variant variant1,
	jnc_Variant variant2
) {
	bool result = false;
	variant1.relationalOperator(&variant2, (jnc_BinOpKind)opKind, &result);
	return result;
}

Variant
variantMemberOperator(
	jnc_Variant variant,
	const char* name
) {
	jnc_Variant result = jnc::g_nullVariant;
	variant.getMember(name, &result);
	return result;
}

Variant
variantIndexOperator(
	jnc_Variant variant,
	size_t index
) {
	jnc_Variant result = jnc::g_nullVariant;
	variant.getElement(index, &result);
	return result;
}

Variant
variantMemberProperty_get(
	DataPtr variantPtr,
	const char* name
) {
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
) {
	Variant* variant = (Variant*)variantPtr.m_p;
	variant->setMember(name, value);
}

Variant
variantIndexProperty_get(
	DataPtr variantPtr,
	size_t index
) {
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
) {
	Variant* variant = (Variant*)variantPtr.m_p;
	variant->setElement(index, value);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
stringConstruct(
	String* string,
	DataPtr ptr,
	size_t length
) {
	string->setPtr(ptr, length);
}

String
stringCreate(
	DataPtr ptr,
	size_t length
) {
	String string;
	string.setPtr(ptr, length);
	return string;
}

String
stringIncrement(
	String string,
	size_t delta
) {
	if (delta >= string.m_length)
		delta = string.m_length;

	if (delta) {
		string.m_ptr.m_p = (char*)string.m_ptr.m_p + delta;
		if (string.m_ptr_sz.m_p)
			string.m_ptr_sz.m_p = (char*)string.m_ptr_sz.m_p + delta;
		string.m_length -= delta;
	}

	return string;
}

static
inline
bool
isNullTerminated(
	DataPtr ptr,
	size_t length
) {
	if (!ptr.m_validator)
		return false;

	char* p = (char*)ptr.m_p;
	char* p0 = (char*)ptr.m_validator->m_rangeBegin;
	char* end = (char*)ptr.m_validator->m_rangeEnd;
	return p >= p0 && p + length < end && !p[length];
}

DataPtr
stringSz(String string) {
	return
		string.m_ptr_sz.m_p ? string.m_ptr_sz :
		isNullTerminated(string.m_ptr, string.m_length) ? string.m_ptr :
		strDup((char*)string.m_ptr.m_p, string.m_length);
}

DataPtr
stringRefSz(String* string) {
	return
		string->m_ptr_sz.m_p ? string->m_ptr_sz :
		isNullTerminated(string->m_ptr, string->m_length) ? string->m_ptr_sz = string->m_ptr :
		string->m_ptr_sz = strDup((char*)string->m_ptr.m_p, string->m_length);
}

bool
stringEq(
	String string1,
	String string2
) {
	return string1.isEqual(&string2);
}

int
stringCmp(
	String string1,
	String string2
) {
	return string1.cmp(&string2);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
assertionFailure(
	const char* fileName,
	int line,
	const char* condition,
	const char* message
) {
	sl::String string;
	string.format("%s(%d): assertion (%s) failed", fileName, line + 1, condition);
	if (message)
		string.appendFormat("; %s", message);

	err::setError(string);
	string.release(); // release before throwing (sl::String::~String will not be called)
	dynamicThrow();
}

bool
tryCheckDataPtrRangeDirect(
	const void* p,
	const void* rangeBegin,
	size_t rangeLength
) {
	if (!p) {
		err::setError("null data pointer access");
		return false;
	}

	void* rangeEnd = (char*)rangeBegin + rangeLength;
	if (p < rangeBegin ||  p > rangeEnd) {
		err::setFormatStringError("data pointer %p out of range [%p:%p]", p, rangeBegin, rangeEnd);
		return false;
	}

	return true;
}

void
checkDataPtrRangeDirect(
	const void* p,
	const void* rangeBegin,
	size_t rangeLength
) {
	bool result = tryCheckDataPtrRangeDirect(p, rangeBegin, rangeLength);
	if (!result)
		dynamicThrow();
}

bool
tryCheckDataPtrRangeIndirect(
	const void* p,
	size_t size,
	DataPtrValidator* validator
) {
	if (!p || !validator) {
		err::setError("null data pointer access");
		return false;
	}

	if (validator->m_targetBox->m_flags & BoxFlag_Invalid) {
		err::setError("invalidated pointer access");
		return false;
	}

	void* end = (char*)p + size;
	if (p < validator->m_rangeBegin || end > validator->m_rangeEnd) {
		err::setFormatStringError("data pointer %p out of range [%p:%p]", p, validator->m_rangeBegin, validator->m_rangeEnd);
		return false;
	}

	return true;
}

void
checkDataPtrRangeIndirect(
	const void* p,
	size_t size,
	DataPtrValidator* validator
) {
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
) {
	ASSERT(lib->m_box->m_type->getTypeKind() == TypeKind_Class);
	ClassType* type = (ClassType*)lib->m_box->m_type;

	if (!lib->m_handle) {
		err::setFormatStringError("dynamiclib '%s' is not loaded yet", type->getQualifiedName().sz());
		return NULL;
	}

	size_t librarySize = type->getIfaceStructType()->getSize();
	size_t functionCount = (librarySize - sizeof(DynamicLib)) / sizeof(void*);

	if (index >= functionCount) {
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
) {
	void* p = tryLazyGetDynamicLibFunction(lib, index, name);
	if (!p)
		dynamicThrow();

	return p;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

rtl::DynamicLayout*
getDynamicLayout(DataPtr ptr) {
	if (!ptr.m_p || !ptr.m_validator) {
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
	Field* field
) {
	ASSERT(type->getFlags() & TypeFlag_Dynamic);

	err::setError("dynamic structs are under redesign");
	dynamicThrow();
	return -1;
}

void*
getDynamicField(
	DataPtr ptr,
	DerivableType* type,
	Field* field
) {
	return (char*)ptr.m_p + getDynamicFieldOffset(ptr, type, field);
}

size_t
dynamicTypeSizeOf(
	DataPtr ptr,
	DerivableType* type
) {
	return getDynamicFieldOffset(ptr, type, NULL);
}

size_t
dynamicFieldSizeOf(
	DataPtr ptr,
	DerivableType* type,
	Field* field
) {
	ASSERT(type->getFlags() & TypeFlag_Dynamic);
	ASSERT(field->getType()->getFlags() & TypeFlag_Dynamic);

	rtl::DynamicLayout* dynamicLayout = getDynamicLayout(ptr);
	err::setError("dynamic structs are under redesign");
	dynamicThrow();
	return -1;
}

size_t
dynamicFieldCountOf(
	DataPtr ptr,
	DerivableType* type,
	Field* field
) {
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
) {
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	size_t newLength = fmtLiteral->m_length + length;
	if (newLength < 64)
		newLength = 64;

	if (fmtLiteral->m_maxLength < newLength) {
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
	validator->m_rangeEnd = (char*)validator->m_rangeBegin + fmtLiteral->m_length + 1; // including zero-terminator
	return fmtLiteral->m_length;
}

void
prepareFormatString(
	sl::String* formatString,
	const char* fmtSpecifier,
	const char* defaultType
) {
	if (!fmtSpecifier) {
		formatString->copy('%');
		formatString->append(defaultType);
		return;
	}

	formatString->clear();

	if (fmtSpecifier[0] != '%')
		formatString->copy('%');

	formatString->append(fmtSpecifier);

	size_t length = formatString->getLength();
	if (!isalpha(formatString->sz()[length - 1]))
		formatString->append(defaultType);
}

inline
size_t
appendFmtLiteralDirect_va(
	FmtLiteral* fmtLiteral,
	const char* formatString,
	axl_va_list va
) {
	char buffer2[256];
	sl::String string(rc::BufKind_Stack, buffer2, sizeof(buffer2));
	string.format_va(formatString, va);

	return appendFmtLiteral_a(fmtLiteral, string, string.getLength());
}

inline
size_t
appendFmtLiteralDirect(
	FmtLiteral* fmtLiteral,
	const char* formatString,
	...
) {
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
) {
	AXL_VA_DECL(va, defaultType);

	char buffer1[256];
	sl::String formatString(rc::BufKind_Stack, buffer1, sizeof(buffer1));
	prepareFormatString(&formatString, fmtSpecifier, defaultType);

	return appendFmtLiteralDirect_va(fmtLiteral, formatString, va);
}

static
size_t
appendFmtLiteralStringImpl(
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	const char* p,
	size_t length,
	DataPtrValidator* validator
) {
	if (!fmtSpecifier)
		return appendFmtLiteral_a(fmtLiteral, p, length);

	if (validator && p + length < (char*)validator->m_rangeEnd && !p[length])
		return appendFmtLiteralImpl(fmtLiteral, fmtSpecifier, "s", p);

	char buffer[256];
	sl::String string(rc::BufKind_Stack, buffer, sizeof(buffer));
	string.copy(p, length);
	return appendFmtLiteralImpl(fmtLiteral, fmtSpecifier, "s", string.sz());
}

static
size_t
appendFmtLiteral_p(
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	DataPtr ptr
) {
	if (!ptr.m_p) // shortcut
		return fmtLiteral->m_length;

	size_t length = strLen(ptr);
	if (!length)
		return fmtLiteral->m_length;

	checkDataPtrRangeIndirect(ptr.m_p, length, ptr.m_validator);
	return appendFmtLiteralStringImpl(
		fmtLiteral,
		fmtSpecifier,
		(const char*)ptr.m_p,
		length,
		ptr.m_validator
	);
}

static
size_t
appendFmtLiteral_i32(
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	int32_t x
) {
	return appendFmtLiteralImpl(fmtLiteral, fmtSpecifier, "d", x);
}

static
size_t
appendFmtLiteral_ui32(
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	uint32_t x
) {
	return appendFmtLiteralImpl(fmtLiteral, fmtSpecifier, "u", x);
}

static
size_t
appendFmtLiteral_i64(
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	int64_t x
) {
	return appendFmtLiteralImpl(fmtLiteral, fmtSpecifier, "lld", x);
}

static
size_t
appendFmtLiteral_ui64(
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	uint64_t x
) {
	return appendFmtLiteralImpl(fmtLiteral, fmtSpecifier, "llu", x);
}

static
size_t
appendFmtLiteral_f(
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	double x
) {
	return appendFmtLiteralImpl(fmtLiteral, fmtSpecifier, "f", x);
}

size_t
appendFmtLiteral_v(
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	Variant variant
) {
	char buffer[256];
	sl::String string(rc::BufKind_Stack, buffer, sizeof(buffer));
	variant.format(&string, fmtSpecifier);
	return appendFmtLiteral_a(fmtLiteral, string, string.getLength());
}

size_t
appendFmtLiteral_s(
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	String string
) {
	DataPtr ptr = string.m_ptr_sz.m_p ? string.m_ptr_sz : string.m_ptr;
	return appendFmtLiteralStringImpl(
		fmtLiteral,
		fmtSpecifier,
		(char*)ptr.m_p,
		string.m_length,
		ptr.m_validator
	);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
collectGarbage() {
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	gcHeap->collect();
}

GcStats
getGcStats() {
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	GcStats stats;
	gcHeap->getStats(&stats);
	return stats;
}

GcSizeTriggers
gcTriggers_get() {
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	GcSizeTriggers triggers;
	gcHeap->getSizeTriggers(&triggers);
	return triggers;
}

void
gcTriggers_set(GcSizeTriggers triggers) {
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	gcHeap->setSizeTriggers(triggers);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataPtr
createDataPtr(
	void* p,
	size_t length
) {
	if (length == -1)
		length = p ? strlen((char*)p) + 1 : 0;

	return jnc::createForeignBufferPtr(p, length, false);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
multicastDestruct(Multicast* multicast) {
	((MulticastImpl*)multicast)->destruct();
}

void
multicastClear(Multicast* multicast) {
	return ((MulticastImpl*)multicast)->clear();
}

handle_t
multicastSet(
	Multicast* multicast,
	FunctionPtr ptr
) {
	return ((MulticastImpl*)multicast)->setHandler(ptr);
}

handle_t
multicastSet_t(
	Multicast* multicast,
	void* p
) {
	return ((MulticastImpl*)multicast)->setHandler_t(p);
}

handle_t
multicastAdd(
	Multicast* multicast,
	FunctionPtr ptr
) {
	return ((MulticastImpl*)multicast)->addHandler(ptr);
}

handle_t
multicastAdd_t(
	Multicast* multicast,
	void* p
) {
	return ((MulticastImpl*)multicast)->addHandler_t(p);
}

FunctionPtr
multicastRemove(
	Multicast* multicast,
	handle_t handle
) {
	return ((MulticastImpl*)multicast)->removeHandler(handle);
}

void*
multicastRemove_t(
	Multicast* multicast,
	handle_t handle
) {
	return ((MulticastImpl*)multicast)->removeHandler_t(handle);
}

FunctionPtr
multicastGetSnapshot(Multicast* multicast) {
	return ((MulticastImpl*)multicast)->getSnapshot();
}

static
void
mapMulticastMethods(
	Module* module,
	const MulticastClassType* multicastType
) {
	static void* multicastMethodTable[FunctionPtrTypeKind__Count][MulticastMethodKind__Count - 1] = {
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
	module->m_jit->mapFunction(function, (void*)multicastDestruct);

	for (size_t i = 0; i < MulticastMethodKind__Count - 1; i++) {
		function = multicastType->getMethod((MulticastMethodKind)i);
		module->m_jit->mapFunction(function, multicastMethodTable[ptrTypeKind][i]);
	}
}

bool
mapAllMulticastMethods(Module* module) {
	const sl::Array<ct::MulticastClassType*>& mcTypeArray = module->m_typeMgr.getMulticastClassTypeArray();
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
	"Jancy core RTL extension library"
)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE(jnc_CoreLib)
	JNC_LIB_SOURCE_FILE("jnc_gc.jnc",         g_jnc_gcSrc)
	JNC_LIB_SOURCE_FILE("jnc_DataPtr.jnc",    g_jnc_dataPtrSrc)
	JNC_LIB_SOURCE_FILE("jnc_DynamicLib.jnc", g_jnc_dynamicLibSrc)
	JNC_LIB_SOURCE_FILE("jnc_Promise.jnc",    g_jnc_promiseSrc)
	JNC_LIB_SOURCE_FILE("jnc_Regex.jnc",      g_jnc_regexSrc)
	JNC_LIB_SOURCE_FILE("jnc_Scheduler.jnc",  g_jnc_schedulerSrc)
JNC_END_LIB_SOURCE_FILE_TABLE()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE(jnc_CoreLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(RegexCapture)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(RegexMatch)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(RegexState)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Regex)
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

	// strings

	JNC_MAP_STD_FUNCTION(ct::StdFunc_StringConstruct, stringConstruct)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_StringCreate,    stringCreate)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_StringIncrement, stringIncrement)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_StringSz,        stringSz)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_StringRefSz,     stringRefSz)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_StringEq,        stringEq)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_StringCmp,       stringCmp)

	// exceptions/async

	JNC_MAP_STD_FUNCTION(ct::StdFunc_SetJmp,         jnc_setJmp)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_SaveSignalInfo, jnc_saveSignalInfo)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_DynamicThrow,   dynamicThrow)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_AsyncRet,       asyncRet)
	JNC_MAP_STD_FUNCTION(ct::StdFunc_AsyncThrow,     asyncThrow)

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
	JNC_MAP_STD_FUNCTION(ct::StdFunc_AppendFmtLiteral_s,    appendFmtLiteral_s)

	// gc heap

	JNC_MAP_FUNCTION_Q("jnc.collectGarbage", collectGarbage)
	JNC_MAP_FUNCTION_Q("jnc.getGcStats",     getGcStats)
	JNC_MAP_PROPERTY_Q("jnc.g_gcTriggers",   gcTriggers_get, gcTriggers_set)

	// thin -> safe data pointers

	JNC_MAP_FUNCTION_Q("jnc.createDataPtr",      createDataPtr)
	JNC_MAP_OVERLOAD(createDataPtr)
	JNC_MAP_FUNCTION_Q("jnc.resetDynamicLayout", resetDynamicLayout)

	// multicasts

	result = mapAllMulticastMethods(module);
	if (!result)
		return false;

	// std types

	JNC_MAP_TYPE(RegexCapture)
	JNC_MAP_TYPE(RegexMatch)
	JNC_MAP_TYPE(RegexState)
	JNC_MAP_TYPE(Regex)
	JNC_MAP_TYPE(DynamicLib)
	JNC_MAP_TYPE(Promise)
	JNC_MAP_TYPE(Promisifier)

	JNC_MAP_STD_TYPE(StdType_DynamicLayout, DynamicLayout)
	JNC_MAP_STD_TYPE(StdType_ReactorBase,   ReactorImpl)

JNC_END_LIB_FUNCTION_MAP()

//..............................................................................

} // namespace rtl
} // namespace jnc
