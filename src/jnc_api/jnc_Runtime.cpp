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

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_ClassType.h"
#	include "jnc_ct_ClosureClassType.h"
#	include "jnc_ct_StructType.h"
#endif

#include "jnc_Runtime.h"

//..............................................................................

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_strLen(jnc_DataPtr ptr) {
	if (!ptr.m_validator ||
		ptr.m_p < ptr.m_validator->m_rangeBegin ||
		(ptr.m_validator->m_targetBox->m_flags & jnc_BoxFlag_Invalid))
		return 0;

	char* p0 = (char*)ptr.m_p;
	char* end = (char*)ptr.m_validator->m_rangeEnd;
	char* p = (char*)memchr(p0, 0, end - p0);
	return p ? p - p0 : end - p0;
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DataPtr
jnc_strDup(
	const char* p,
	size_t length
) {
	using namespace jnc;

	if (length == -1)
		length = strlen_s(p);

	if (!length)
		return g_nullDataPtr;

	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	DataPtr resultPtr = gcHeap->allocateBuffer(length + 1);
	memcpy(resultPtr.m_p, p, length);
	return resultPtr;
}

template <typename T>
jnc::DataPtr
strDupConvert(
	const T* p,
	size_t length
) {
	using namespace jnc;

	typedef sl::StringDetailsBase<T> StringDetails;
	typedef enc::Convert<enc::Utf8, typename StringDetails::Encoding> Convert;

	if (length == -1)
		length = StringDetails::calcLength(p);

	if (!length)
		return g_nullDataPtr;

	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	size_t resultLength = Convert::calcRequiredLength(p, p + length);
	DataPtr resultPtr = gcHeap->allocateBuffer(resultLength + 1);
	Convert::convert_u((char*)resultPtr.m_p, p, p + length);
	return resultPtr;
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DataPtr
jnc_strDup_w(
	const wchar_t* p,
	size_t length
) {
	return strDupConvert(p, length);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DataPtr
jnc_strDup_utf16(
	const utf16_t* p,
	size_t length
) {
	return strDupConvert(p, length);
}

template <typename T>
jnc_String
allocateStringImpl(
	const T* p,
	size_t length
) {
	using namespace jnc;

	typedef sl::StringDetailsBase<T> StringDetails;

	if (length == -1)
		length = StringDetails::calcLength(p);

	if (!length)
		return g_nullString;

	String string;
	string.m_ptr = strDup(p, length);
	string.m_ptr_sz = string.m_ptr;
	string.m_length = length;
	return string;
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_String
jnc_allocateString(
	const char* p,
	size_t length
) {
	return allocateStringImpl(p, length);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_String
jnc_allocateString_w(
	const wchar_t* p,
	size_t length
) {
	return allocateStringImpl(p, length);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_String
jnc_allocateString_utf16(
	const utf16_t* p,
	size_t length
) {
	return allocateStringImpl(p, length);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DataPtr
jnc_memDup(
	const void* p,
	size_t size
) {
	using namespace jnc;

	if (!size)
		return g_nullDataPtr;

	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	DataPtr resultPtr = gcHeap->allocateBuffer(size);
	if (p)
		memcpy(resultPtr.m_p, p, size);

	return resultPtr;
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DataPtr
jnc_createForeignBufferPtr(
	const void* p,
	size_t size,
	bool_t isCallSiteLocal
) {
	using namespace jnc;

	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	return gcHeap->createForeignBufferPtr(p, size, isCallSiteLocal != 0);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_String
jnc_createForeignString(
	const char* p,
	size_t length,
	bool_t isCallSiteLocal
) {
	using namespace jnc;

	size_t ptrLength;
	if (length != -1)
		ptrLength = length;
	else if (!p)
		ptrLength = length = 0;
	else {
		length = strlen(p);
		ptrLength = length + 1;
	}

	if (!length)
		return g_nullString;

	DataPtr ptr = createForeignBufferPtr(p, ptrLength, isCallSiteLocal);
	String string;
	string.setPtr(ptr, length);
	return string;
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_String
jnc_createForeignString_sz(
	const char* p,
	size_t length,
	bool_t isCallSiteLocal
) {
	using namespace jnc;

	ASSERT(p);

	if (length == -1)
		length = strlen(p);

	if (!length)
		return g_nullString;

	ASSERT(!p[length]);
	DataPtr ptr = createForeignBufferPtr(p, length + 1, isCallSiteLocal);
	String string;
	string.m_ptr = ptr;
	string.m_ptr_sz = ptr;
	string.m_length = length;
	return string;
}

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_Runtime*
jnc_Runtime_create() {
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_createFunc();
}

JNC_EXTERN_C
void
jnc_Runtime_destroy(jnc_Runtime* runtime) {
	jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_destroyFunc(runtime);
}

JNC_EXTERN_C
jnc_Module*
jnc_Runtime_getModule(jnc_Runtime* runtime) {
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_getModuleFunc(runtime);
}

JNC_EXTERN_C
jnc_GcHeap*
jnc_Runtime_getGcHeap(jnc_Runtime* runtime) {
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_getGcHeapFunc(runtime);
}

JNC_EXTERN_C
bool_t
jnc_Runtime_isAborted(jnc_Runtime* runtime) {
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_isAbortedFunc(runtime);
}

JNC_EXTERN_C
bool_t
jnc_Runtime_startup(
	jnc_Runtime* runtime,
	jnc_Module* module
) {
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_startupFunc(runtime, module);
}

JNC_EXTERN_C
void
jnc_Runtime_shutdown(jnc_Runtime* runtime) {
	jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_shutdownFunc(runtime);
}

JNC_EXTERN_C
void
jnc_Runtime_abort(jnc_Runtime* runtime) {
	jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_abortFunc(runtime);
}

JNC_EXTERN_C
void
jnc_Runtime_initializeCallSite(
	jnc_Runtime* runtime,
	jnc_CallSite* callSite
) {
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_initializeCallSiteFunc(runtime, callSite);
}

JNC_EXTERN_C
void
jnc_Runtime_uninitializeCallSite(
	jnc_Runtime* runtime,
	jnc_CallSite* callSite
) {
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_uninitializeCallSiteFunc(runtime, callSite);
}

JNC_EXTERN_C
jnc_SjljFrame*
jnc_Runtime_setSjljFrame(
	jnc_Runtime* runtime,
	jnc_SjljFrame* frame
) {
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_setSjljFrameFunc(runtime, frame);
}

JNC_EXTERN_C
void*
jnc_Runtime_getUserData(jnc_Runtime* runtime) {
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_getUserDataFunc(runtime);
}

JNC_EXTERN_C
void*
jnc_Runtime_setUserData(
	jnc_Runtime* runtime,
	void* data
) {
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_setUserDataFunc(runtime, data);
}

JNC_EXTERN_C
jnc_Runtime*
jnc_getCurrentThreadRuntime() {
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_getCurrentThreadRuntimeFunc();
}

JNC_EXTERN_C
jnc_Tls*
jnc_getCurrentThreadTls() {
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_getCurrentThreadTlsFunc();
}

JNC_EXTERN_C
void
jnc_dynamicThrow() {
	jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_dynamicThrowFunc();
}

JNC_EXTERN_C
void
jnc_saveSignalInfo(jnc_SjljFrame* sjljFrame) {
	jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_saveSignalInfoFunc(sjljFrame);
}

JNC_EXTERN_C
void
jnc_primeClass(
	jnc_Box* box,
	jnc_Box* root,
	jnc_ClassType* type,
	const void* vtable
) {
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_primeClassFunc(box, root, type, vtable);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Runtime*
jnc_Runtime_create() {
	return new jnc_Runtime;
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Runtime_destroy(jnc_Runtime* runtime) {
	delete runtime;
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Module*
jnc_Runtime_getModule(jnc_Runtime* runtime) {
	return runtime->getModule();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_GcHeap*
jnc_Runtime_getGcHeap(jnc_Runtime* runtime) {
	return runtime->getGcHeap();
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Runtime_isAborted(jnc_Runtime* runtime) {
	return runtime->isAborted();
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Runtime_startup(
	jnc_Runtime* runtime,
	jnc_Module* module
) {
	return runtime->startup(module);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Runtime_shutdown(jnc_Runtime* runtime) {
	return runtime->shutdown();
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Runtime_abort(jnc_Runtime* runtime) {
	return runtime->abort();
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Runtime_initializeCallSite(
	jnc_Runtime* runtime,
	jnc_CallSite* callSite
) {
	runtime->initializeCallSite(callSite);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Runtime_uninitializeCallSite(
	jnc_Runtime* runtime,
	jnc_CallSite* callSite
) {
	runtime->uninitializeCallSite(callSite);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_SjljFrame*
jnc_Runtime_setSjljFrame(
	jnc_Runtime* runtime,
	jnc_SjljFrame* frame
) {
	return runtime->setSjljFrame(frame);
}

JNC_EXTERN_C
JNC_EXPORT_O
void*
jnc_Runtime_getUserData(jnc_Runtime* runtime) {
	return runtime->m_userData;
}

JNC_EXTERN_C
JNC_EXPORT_O
void*
jnc_Runtime_setUserData(
	jnc_Runtime* runtime,
	void* data
) {
	return (void*)sys::atomicXchg((size_t volatile*) &runtime->m_userData, (size_t)data);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Runtime*
jnc_getCurrentThreadRuntime() {
	return jnc::rt::getCurrentThreadRuntime();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Tls*
jnc_getCurrentThreadTls() {
	return jnc::rt::getCurrentThreadTls();
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_dynamicThrow() {
	jnc::rt::Runtime::dynamicThrow();
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_saveSignalInfo(jnc_SjljFrame* sjljFrame) {
#if (_JNC_OS_POSIX)
	jnc::rt::Runtime::saveSignalInfo(sjljFrame);
#endif
}

static
void
primeIface(
	jnc_Box* box,
	jnc_Box* root,
	jnc_IfaceHdr* iface,
	jnc_ClassType* type,
	const void* vtable
) {
	using namespace jnc;

	iface->m_vtable = vtable;
	iface->m_box = box;

	// primeClass all the base types

	sl::Array<ct::BaseTypeSlot*> classBaseTypeArray = type->getClassBaseTypeArray();
	size_t count = classBaseTypeArray.getCount();
	for (size_t i = 0; i < count; i++) {
		ct::BaseTypeSlot* slot = classBaseTypeArray[i];
		ASSERT(slot->getType()->getTypeKind() == TypeKind_Class);

		primeIface(
			box,
			root,
			(IfaceHdr*)((char*)iface + slot->getOffset()),
			(ct::ClassType*)slot->getType(),
			(void**) vtable + slot->getVtableIndex()
		);
	}

	// primeClass all the class fields

	const sl::Array<ct::Field*>& fieldPrimeArray = type->getClassFieldArray();
	count = fieldPrimeArray.getCount();
	for (size_t i = 0; i < count; i++) {
		ct::Field* field = fieldPrimeArray[i];
		ASSERT(field->getType()->getTypeKind() == TypeKind_Class);

		ct::ClassType* fieldType = (ct::ClassType*)field->getType();
		Box* fieldBox = (Box*)((char*)iface + field->getOffset());

		primeClass(
			fieldBox,
			root,
			fieldType
		);
	}
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_primeClass(
	jnc_Box* box,
	jnc_Box* root,
	jnc_ClassType* type,
	const void* vtable
) {
	ASSERT(root <= box);

	if (!vtable) {
		jnc_Variable* vtableVariable = type->getVtableVariable();
		if (vtableVariable)
			vtable = vtableVariable->getStaticData();
	}

	memset(box, 0, type->getSize());

	box->m_type = type;
	box->m_flags = jnc_BoxFlag_ClassMark | jnc_BoxFlag_DataMark | jnc_BoxFlag_WeakMark;
	box->m_rootOffset = (char*)box - (char*)root;

	primeIface(box, root, (jnc_IfaceHdr*)(box + 1), type, vtable);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_IfaceHdr*
jnc_strengthenClassPtr(jnc_IfaceHdr* iface) {
	using namespace jnc;

	if (!iface)
		return NULL;

	ASSERT(iface->m_box->m_type->getTypeKind() == TypeKind_Class);
	ClassType* classType = (ClassType*)iface->m_box->m_type;
	ClassTypeKind classTypeKind = classType->getClassTypeKind();

	return classTypeKind == ClassTypeKind_FunctionClosure || classTypeKind == ClassTypeKind_PropertyClosure ?
		((ct::ClosureClassType*)classType)->strengthen(iface) :
		(iface->m_box->m_flags & BoxFlag_ClassMark) && !(iface->m_box->m_flags & BoxFlag_Destructed) ? iface : NULL;
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................

#if (_JNC_OS_WIN)
#	if (_JNC_CPU_X86)

// based on: reactos/sdk/lib/crt/setjmp/i386/setjmp.s

#define JB_BP  0
#define JB_BX  1
#define JB_DI  2
#define JB_SI  3
#define JB_SP  4
#define JB_IP  5
#define JB_EH  6
#define PCOFF  0
#define JMPBUF 4

#ifdef xor
#	undef xor
#endif

EXTERN_C
int
__declspec(naked)
jnc_setJmp(jmp_buf jmpBuf) {
	__asm {
		xor eax, eax
		mov edx, JMPBUF[esp]

		/* Save registers.  */
		mov [edx + JB_BP*4], ebp /* Save caller's frame pointer.  */
		mov [edx + JB_BX*4], ebx
		mov [edx + JB_DI*4], edi
		mov [edx + JB_SI*4], esi
		lea ecx, JMPBUF[esp]    /* Save SP as it will be after we return.  */
		mov [edx + JB_SP*4], ecx
		mov ecx, PCOFF[esp]     /* Save PC we are returning to now.  */
		mov [edx + JB_IP*4], ecx
		mov ecx, FS:[0]         /* Save EXCEPTION_REGISTRATION */
		mov [edx + JB_EH*4], ecx
		ret
	}
}

// warning C4733: Inline asm assigning to 'FS:0': handler not registered as safe handler
#pragma warning(disable: 4733)

EXTERN_C
void
__declspec(naked)
jnc_longJmp(
	jmp_buf jmpBuf,
	int retVal
) {
	__asm {
		mov ecx, JMPBUF[esp]   /* User's jmp_buf in %ecx.  */
		mov eax, [esp + 8]      /* Second argument is return value.  */
		/* Restore EXCEPTION_REGISTRATION */
		mov edx, [ecx + JB_EH*4]
		mov FS:[0], edx
		/* Save the return address now.  */
		mov edx, [ecx + JB_IP*4]
		/* Restore registers.  */
		mov ebp, [ecx + JB_BP*4]
		mov ebx, [ecx + JB_BX*4]
		mov edi, [ecx + JB_DI*4]
		mov esi, [ecx + JB_SI*4]
		mov esp, [ecx + JB_SP*4]
		/* Jump to saved PC.  */
		jmp edx
	}
}

#	elif (_JNC_CPU_AMD64)
#		undef jnc_setJmp

/*
void
jnc_longJmp(
	jmp_buf jmpBuf,
	int retVal
) {
	_JUMP_BUFFER* buffer = (_JUMP_BUFFER*)jmpBuf;
	buffer->Frame = 0;
	longjmp(jmpBuf, retVal);
}
*/

#	endif
#endif

//..............................................................................
