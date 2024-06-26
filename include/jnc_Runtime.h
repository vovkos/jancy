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

#define _JNC_RUNTIME_H

#include "jnc_RuntimeStructs.h"
#include "jnc_Variant.h"
#include "jnc_GcHeap.h"
#include "jnc_Construct.h"

/**

\defgroup runtime-subsystem Runtime Subsystem

\defgroup runtime Runtime
	\ingroup runtime-subsystem
	\import{jnc_Runtime.h}

\addtogroup runtime
@{

\struct jnc_Runtime
	\verbatim

	Opaque structure used as a handle to Jancy runtime.

	Use functions from the `Runtime` group to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

JNC_EXTERN_C
jnc_Runtime*
jnc_Runtime_create();

JNC_EXTERN_C
void
jnc_Runtime_destroy(jnc_Runtime* runtime);

JNC_EXTERN_C
jnc_Module*
jnc_Runtime_getModule(jnc_Runtime* runtime);

JNC_EXTERN_C
jnc_GcHeap*
jnc_Runtime_getGcHeap(jnc_Runtime* runtime);

JNC_EXTERN_C
bool_t
jnc_Runtime_isAborted(jnc_Runtime* runtime);

JNC_EXTERN_C
bool_t
jnc_Runtime_startup(
	jnc_Runtime* runtime,
	jnc_Module* module
);

JNC_EXTERN_C
void
jnc_Runtime_shutdown(jnc_Runtime* runtime);

JNC_EXTERN_C
void
jnc_Runtime_abort(jnc_Runtime* runtime);

JNC_EXTERN_C
void
jnc_Runtime_initializeCallSite(
	jnc_Runtime* runtime,
	jnc_CallSite* callSite
);

JNC_EXTERN_C
void
jnc_Runtime_uninitializeCallSite(
	jnc_Runtime* runtime,
	jnc_CallSite* callSite
);

JNC_EXTERN_C
jnc_SjljFrame*
jnc_Runtime_setSjljFrame(
	jnc_Runtime* runtime,
	jnc_SjljFrame* frame
);

JNC_EXTERN_C
void*
jnc_Runtime_getUserData(jnc_Runtime* runtime);

JNC_EXTERN_C
void*
jnc_Runtime_setUserData(
	jnc_Runtime* runtime,
	void* data
);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)
struct jnc_Runtime {
	static
	jnc_Runtime*
	create() {
		return jnc_Runtime_create();
	}

	void
	destroy() {
		jnc_Runtime_destroy(this);
	}

	jnc_Module*
	getModule() {
		return jnc_Runtime_getModule(this);
	}

	jnc_GcHeap*
	getGcHeap() {
		return jnc_Runtime_getGcHeap(this);
	}

	bool
	isAborted() {
		return jnc_Runtime_isAborted(this) != 0;
	}

	bool
	startup(jnc_Module* module) {
		return jnc_Runtime_startup(this, module) != 0;
	}

	void
	shutdown() {
		jnc_Runtime_shutdown(this);
	}

	void
	abort() {
		jnc_Runtime_abort(this);
	}

	void
	initializeCallSite(jnc_CallSite* callSite) {
		jnc_Runtime_initializeCallSite(this, callSite);
	}

	void
	uninitializeCallSite(jnc_CallSite* callSite) {
		jnc_Runtime_uninitializeCallSite(this, callSite);
	}

	void*
	getUserData() {
		return jnc_Runtime_getUserData(this);
	}

	void*
	setUserData(void* data) {
		return jnc_Runtime_setUserData(this, data);
	}
};
#endif // _JNC_CORE

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
jnc_Runtime*
jnc_getCurrentThreadRuntime();

JNC_INLINE
jnc_GcHeap*
jnc_getCurrentThreadGcHeap() {
	jnc_Runtime* runtime = jnc_getCurrentThreadRuntime();
	return runtime ? jnc_Runtime_getGcHeap(runtime) : NULL;
}

JNC_EXTERN_C
jnc_Tls*
jnc_getCurrentThreadTls();

JNC_EXTERN_C
void
jnc_dynamicThrow();

JNC_EXTERN_C
void
jnc_saveSignalInfo(jnc_SjljFrame* sjljFrame);

JNC_EXTERN_C
void
jnc_primeClass(
	jnc_Box* box,
	jnc_Box* root,
	jnc_ClassType* type,
	const void* vtable // if null then vtable of class type will be used
);

JNC_EXTERN_C
jnc_IfaceHdr*
jnc_strengthenClassPtr(jnc_IfaceHdr* iface);

JNC_EXTERN_C
size_t
jnc_strLen(jnc_DataPtr ptr);

JNC_EXTERN_C
jnc_DataPtr
jnc_strDup(
	const char* p,
	size_t length
);

JNC_EXTERN_C
jnc_DataPtr
jnc_strDup_w(
	const wchar_t* p,
	size_t length
);

JNC_EXTERN_C
jnc_DataPtr
jnc_strDup_utf16(
	const utf16_t* p,
	size_t length
);

JNC_EXTERN_C
jnc_String
jnc_allocateString(
	const char* p,
	size_t length
);

JNC_EXTERN_C
jnc_String
jnc_allocateString_w(
	const wchar_t* p,
	size_t length
);

JNC_EXTERN_C
jnc_String
jnc_allocateString_utf16(
	const utf16_t* p,
	size_t length
);

JNC_EXTERN_C
jnc_DataPtr
jnc_memDup(
	const void* p,
	size_t size
);

JNC_EXTERN_C
JNC_INLINE
intptr_t
jnc_getDataPtrLeftRadius(jnc_DataPtr ptr) {
	return ptr.m_validator ? (char*)ptr.m_p - (char*)ptr.m_validator->m_rangeBegin : 0;
}

JNC_EXTERN_C
JNC_INLINE
intptr_t
jnc_getDataPtrRightRadius(jnc_DataPtr ptr) {
	return ptr.m_validator ? (char*)ptr.m_validator->m_rangeEnd - (char*)ptr.m_p : 0;
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DataPtr
jnc_limitDataPtr(
	jnc_DataPtr ptr,
	size_t length
);

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DataPtrValidator*
jnc_createDataPtrValidator(
	jnc_Box* box,
	const void* rangeBegin,
	size_t rangeLength
);

JNC_EXTERN_C
jnc_DataPtr
jnc_createForeignBufferPtr(
	const void* p,
	size_t size,
	bool_t isCallSiteLocal
);

JNC_EXTERN_C
jnc_String
jnc_createForeignString(
	const char* p,
	size_t length,
	bool_t isCallSiteLocal
);

JNC_EXTERN_C
jnc_String
jnc_createForeignString_sz(
	const char* p,
	size_t length,
	bool_t isCallSiteLocal
);

// we want to make sure there is no unwinding during SJLJ -- alas,
// unwinding doesn't work too well with the LLVM-generated JIT-code

#if (_JNC_OS_WIN)

JNC_EXTERN_C
int
jnc_setJmp(jmp_buf jmpBuf);

JNC_EXTERN_C
void
jnc_longJmp(
	jmp_buf jmpBuf,
	int retVal
);

#else
#	define jnc_setJmp  setjmp
#	define jnc_longJmp longjmp
#endif

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

inline
Runtime*
getCurrentThreadRuntime() {
	return jnc_getCurrentThreadRuntime();
}

inline
GcHeap*
getCurrentThreadGcHeap() {
	return jnc_getCurrentThreadGcHeap();
}

inline
Tls*
getCurrentThreadTls() {
	return jnc_getCurrentThreadTls();
}

inline
void
dynamicThrow() {
	return jnc_dynamicThrow();
}

inline
void
saveSignalInfo(SjljFrame* sjljFrame) {
	jnc_saveSignalInfo(sjljFrame);
}


// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
void
primeClass(
	Box* box,
	Box* root,
	ClassType* type,
	const void* vtable = NULL // if null then vtable of class type will be used
) {
	jnc_primeClass(box, root, type, vtable);
}

inline
void
primeClass(
	Box* box,
	ClassType* type,
	const void* vtable = NULL // if null then vtable of class type will be used
) {
	jnc_primeClass(box, box, type, vtable);
}

template <typename T>
void
primeClass(
	Module* module,
	ClassBoxBase<T>* p,
	Box* root
) {
	jnc_primeClass(p, root, T::getType(module), T::getVtable());
}

template <typename T>
void
primeClass(
	Module* module,
	ClassBoxBase<T>* p
) {
	primeClass(p, p, T::getType(module), T::getVtable());
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
IfaceHdr*
strengthenClassPtr(IfaceHdr* iface) {
	return jnc_strengthenClassPtr(iface);
}

template <typename T>
T*
createClass(Runtime* runtime) {
	ClassType* type = T::getType(runtime->getModule());
	T* p = (T*)runtime->getGcHeap()->allocateClass(type);
	jnc::construct(p);
	return p;
}

template <
	typename T,
	typename Arg
>
T*
createClass(
	Runtime* runtime,
	Arg arg
) {
	ClassType* type = T::getType(runtime->getModule());
	T* p = (T*)runtime->getGcHeap()->allocateClass(type);
	jnc::construct(p, arg);
	return p;
}

template <
	typename T,
	typename Arg1,
	typename Arg2
>
T*
createClass(
	Runtime* runtime,
	Arg1 arg1,
	Arg2 arg2
) {
	ClassType* type = T::getType(runtime->getModule());
	T* p = (T*)runtime->getGcHeap()->allocateClass(type);
	jnc::construct(p, arg1, arg2);
	return p;
}

template <
	typename T,
	typename Arg1,
	typename Arg2,
	typename Arg3
>
T*
createClass(
	Runtime* runtime,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
) {
	ClassType* type = T::getType(runtime->getModule());
	T* p = (T*)runtime->getGcHeap()->allocateClass(type);
	jnc::construct(p, arg1, arg2, arg3);
	return p;
}

template <
	typename T,
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4
>
T*
createClass(
	Runtime* runtime,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
) {
	ClassType* type = T::getType(runtime->getModule());
	T* p = (T*)runtime->getGcHeap()->allocateClass(type);
	jnc::construct(p, arg1, arg2, arg3, arg4);
	return p;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename T>
DataPtr
createData(Runtime* runtime) {
	Type* type = T::getType(runtime->getModule());
	DataPtr ptr = runtime->getGcHeap()->allocateData(type);
	jnc::construct((T*)ptr.m_p);
	return ptr;
}

template <
	typename T,
	typename Arg
>
DataPtr
createData(
	Runtime* runtime,
	Arg arg
) {
	Type* type = T::getType(runtime->getModule());
	DataPtr ptr = runtime->getGcHeap()->allocateData(type);
	jnc::construct((T*)ptr.m_p, arg);
	return ptr;
}

template <
	typename T,
	typename Arg1,
	typename Arg2
>
DataPtr
createData(
	Runtime* runtime,
	Arg1 arg1,
	Arg2 arg2
) {
	Type* type = T::getType(runtime->getModule());
	DataPtr ptr = runtime->getGcHeap()->allocateData(type);
	jnc::construct((T*)ptr.m_p, arg1, arg2);
	return ptr;
}

template <
	typename T,
	typename Arg1,
	typename Arg2,
	typename Arg3
>
DataPtr
createData(
	Runtime* runtime,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
) {
	Type* type = T::getType(runtime->getModule());
	DataPtr ptr = runtime->getGcHeap()->allocateData(type);
	jnc::construct((T*)ptr.m_p, arg1, arg2, arg3);
	return ptr;
}

template <
	typename T,
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4
>
DataPtr
createData(
	Runtime* runtime,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
) {
	Type* type = T::getType(runtime->getModule());
	DataPtr ptr = runtime->getGcHeap()->allocateData(type);
	jnc::construct((T*)ptr.m_p, arg1, arg2, arg3, arg4);
	return ptr;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
size_t
strLen(DataPtr ptr) {
	return jnc_strLen(ptr);
}

inline
DataPtr
strDup(
	const char* p,
	size_t length = -1
) {
	return jnc_strDup(p, length);
}

inline
DataPtr
strDup(
	const wchar_t* p,
	size_t length = -1
) {
	return jnc_strDup_w(p, length);
}

inline
String
allocateString(
	const char* p,
	size_t length = -1
) {
	return jnc_allocateString(p, length);
}

inline
String
allocateString(
	const wchar_t* p,
	size_t length = -1
) {
	return jnc_allocateString_w(p, length);
}

#if (WCHAR_MAX > 0xffff)

inline
DataPtr
strDup(
	const utf16_t* p,
	size_t length = -1
) {
	return jnc_strDup_utf16(p, length);
}

inline
String
allocateString(
	const utf16_t* p,
	size_t length = -1
) {
	return jnc_allocateString_utf16(p, length);
}

#endif

inline
DataPtr
memDup(
	const void* p,
	size_t size
) {
	return jnc_memDup(p, size);
}

inline
intptr_t
getDataPtrLeftRadius(jnc_DataPtr ptr) {
	return jnc_getDataPtrLeftRadius(ptr);
}

inline
intptr_t
getDataPtrRightRadius(jnc_DataPtr ptr) {
	return jnc_getDataPtrRightRadius(ptr);
}

inline
jnc_DataPtr
limitDataPtr(
	jnc_DataPtr ptr,
	size_t length
) {
	return jnc_limitDataPtr(ptr, length);
}

inline
jnc_DataPtrValidator*
createDataPtrValidator(
	jnc_Box* box,
	const void* rangeBegin,
	size_t rangeLength
) {
	return jnc_createDataPtrValidator(box, rangeBegin, rangeLength);
}

inline
DataPtr
createForeignBufferPtr(
	const void* p,
	size_t size,
	bool isCallSiteLocal = true
) {
	return jnc_createForeignBufferPtr(p, size, isCallSiteLocal);
}

inline
String
createForeignString(
	const char* p,
	size_t length,
	bool isCallSiteLocal = true
) {
	return jnc_createForeignString(p, length, isCallSiteLocal);
}

#ifdef _AXL_SL_STRING_H

inline
DataPtr
strDup(const axl::sl::StringRef& string) {
	return jnc_strDup(string.cp(), string.getLength());
}

inline
DataPtr
strDup(const axl::sl::StringRef_utf16& string) {
	return jnc_strDup_utf16(string.cp(), string.getLength());
}

inline
String
allocateString(const axl::sl::StringRef& string) {
	return jnc_allocateString(string.cp(), string.getLength());
}

inline
String
createForeignString(
	const axl::sl::StringRef& string,
	bool isCallSiteLocal = true
) {
	return string.isNullTerminated() ?
		jnc_createForeignString_sz(string.cp(), string.getLength(), isCallSiteLocal) :
		jnc_createForeignString(string.cp(), string.getLength(), isCallSiteLocal);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct DualString {
	axl::sl::StringRef m_string_axl;
	mutable String m_string_jnc;

	DualString() {
		m_string_jnc = g_nullString;
	}

	DualString(
		const axl::sl::StringRef& string_axl,
		String string_jnc = g_nullString
	):
		m_string_axl(string_axl),
		m_string_jnc(string_jnc) {}

#if (_AXL_CPP_HAS_RVALUE_REF)
	DualString(
		axl::sl::StringRef&& string_axl,
		String string_jnc = g_nullString
	):
		m_string_axl(::std::move(string_axl)),
		m_string_jnc(string_jnc) {}
#endif

	operator const axl::sl::StringRef& () const {
		return m_string_axl;
	}

	operator String () const {
		return getString();
	}

	DualString&
	operator = (const axl::sl::StringRef& string) {
		m_string_axl = string;
		m_string_jnc = g_nullString;
		return *this;
	}

#if (_AXL_CPP_HAS_RVALUE_REF)
	DualString&
	operator = (axl::sl::StringRef&& string) {
		m_string_axl = ::std::move(string);
		m_string_jnc = g_nullString;
		return *this;
	}
#endif

	bool
	isEmpty() const {
		return m_string_axl.isEmpty();
	}

	const char*
	sz() const {
		return m_string_axl.sz();
	}

	String
	getString() const {
		return m_string_jnc.m_length || m_string_axl.isEmpty() ?
			m_string_jnc :
			(m_string_jnc = allocateString(m_string_axl));
	}

	void
	clear() {
		m_string_axl.clear();
		m_string_jnc = g_nullString;
	}

	void
	setup(
		const axl::sl::StringRef& string_axl,
		String string_jnc = g_nullString
	) {
		m_string_axl = string_axl;
		m_string_jnc = string_jnc;
	}

#if (_AXL_CPP_HAS_RVALUE_REF)
	void
	setup(
		axl::sl::StringRef&& string_axl,
		String string_jnc = g_nullString
	) {
		m_string_axl = ::std::move(string_axl);
		m_string_jnc = string_jnc;
	}
#endif

	void
	markGcRoots(jnc::GcHeap* gcHeap) {
		gcHeap->markString(m_string_jnc);
	}
};

#endif // _AXL_SL_STRING_H

//..............................................................................

class AutoRuntime {
protected:
	Runtime* m_runtime;

public:
	AutoRuntime() {
		m_runtime = jnc_Runtime_create();
	}

	~AutoRuntime() {
		if (m_runtime)
			jnc_Runtime_destroy(m_runtime);
	}

	operator Runtime* () const {
		return m_runtime;
	}

	Runtime*
	operator -> () const {
		return m_runtime;
	}

	Runtime*
	p() const {
		return m_runtime;
	}
};

//..............................................................................

// canCollectOnLeave = false as default is more suitable for situations where
// no-collect-regions are generally required (e.g., we are building a complex
// data structure somewhere from a C++ code, then return a pointer to it)

class NoCollectRegion {
protected:
	GcHeap* m_gcHeap;
	bool m_canCollectOnLeave;

public:
	NoCollectRegion(
		GcHeap* gcHeap,
		bool canCollectOnLeave = false
	) {
		init(gcHeap, canCollectOnLeave);
	}

	NoCollectRegion(
		Runtime* runtime,
		bool canCollectOnLeave = false
	) {
		init(runtime->getGcHeap(), canCollectOnLeave);
	}

	~NoCollectRegion() {
		m_gcHeap->leaveNoCollectRegion(m_canCollectOnLeave);
	}

protected:
	void
	init(
		GcHeap* gcHeap,
		bool canCollectOnLeave
	) {
		JNC_ASSERT(gcHeap);
		m_gcHeap = gcHeap;
		m_canCollectOnLeave = canCollectOnLeave;
		gcHeap->enterNoCollectRegion();
	}
};

//..............................................................................

class WaitRegion {
protected:
	GcHeap* m_gcHeap;

public:
	WaitRegion(GcHeap* gcHeap) {
		init(gcHeap);
	}

	WaitRegion(Runtime* runtime) {
		init(runtime->getGcHeap());
	}

	WaitRegion() {
		init(getCurrentThreadGcHeap());
	}

	~WaitRegion() {
		m_gcHeap->leaveWaitRegion();
	}

protected:
	void
	init(GcHeap* gcHeap) {
		JNC_ASSERT(gcHeap);
		m_gcHeap = gcHeap;
		gcHeap->enterWaitRegion();
	}
};

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
