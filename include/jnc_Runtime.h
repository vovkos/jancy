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
jnc_memDup(
	const void* p,
	size_t size
);

JNC_EXTERN_C
jnc_DataPtr
jnc_createForeignBufferPtr(
	const void* p,
	size_t size,
	bool_t isCallSiteLocal
);

JNC_EXTERN_C
jnc_DataPtr
jnc_createForeignStringPtr(
	const char* p,
	bool_t isCallSiteLocal
);

// we want to make sure there is no unwinding during SJLJ -- alas,
// unwinding doesn't work too well with the LLVM-generated JIT-code

#if (_JNC_OS_WIN)
#	if (_JNC_CPU_X86)
JNC_EXTERN_C
int
jnc_setJmp(jmp_buf jmpBuf);
#	else
#		define jnc_setJmp setjmp
#	endif

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
	construct(p);
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
	construct(p, arg);
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
	construct(p, arg1, arg2);
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
	construct(p, arg1, arg2, arg3);
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
	construct(p, arg1, arg2, arg3, arg4);
	return p;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename T>
DataPtr
createData(Runtime* runtime) {
	Type* type = T::getType(runtime->getModule());
	DataPtr ptr = runtime->getGcHeap()->allocateData(type);
	construct((T*)ptr.m_p);

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
	construct((T*)ptr.m_p, arg);

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
	construct((T*)ptr.m_p, arg1, arg2);

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
	construct((T*)ptr.m_p, arg1, arg2, arg3);

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
	construct((T*)ptr.m_p, arg1, arg2, arg3, arg4);

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
memDup(
	const void* p,
	size_t size
) {
	return jnc_memDup(p, size);
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
DataPtr
createForeignStringPtr(
	const char* p,
	bool isCallSiteLocal = true
) {
	return jnc_createForeignStringPtr(p, isCallSiteLocal);
}

#ifdef _AXL_SL_STRING_H

inline
DataPtr
strDup(const axl::sl::StringRef& string) {
	return jnc_strDup(string.cp(), string.getLength());
}

inline
DataPtr
createForeignStringPtr(
	const axl::sl::StringRef& string,
	bool isCallSiteLocal = true
) {
	const char* p = string.cp();
	return jnc_createForeignBufferPtr(p, p ? string.getLength() + 1 : 0, isCallSiteLocal);
}

struct DualString {
	axl::sl::StringRef m_string;
	DataPtr m_ptr;

	DualString() {
		m_ptr = g_nullDataPtr;
	}

	DualString(
		const axl::sl::StringRef& string,
		DataPtr ptr = g_nullDataPtr
	):
		m_string(string) {
		m_ptr = ptr;
	}

#if (_AXL_CPP_HAS_RVALUE_REF)
	DualString(
		axl::sl::StringRef&& string,
		DataPtr ptr = g_nullDataPtr
	):
		m_string(string) {
		m_ptr = ptr;
	}
#endif

	DualString(
		const char* string,
		DataPtr ptr = g_nullDataPtr
	):
		m_string(string) {
		m_ptr = ptr;
	}

	operator const axl::sl::StringRef& () const {
		return m_string;
	}

	operator DataPtr () const {
		return m_ptr;
	}

	DualString&
	operator = (const axl::sl::StringRef& string) {
		m_string = string;
		m_ptr = g_nullDataPtr;
		return *this;
	}

#if (_AXL_CPP_HAS_RVALUE_REF)
	DualString&
	operator = (axl::sl::StringRef&& string) {
		m_string = string;
		m_ptr = g_nullDataPtr;
		return *this;
	}
#endif

	DualString&
	operator = (const char* string) {
		m_string = string;
		m_ptr = g_nullDataPtr;
		return *this;
	}

	bool
	isEmpty() const {
		return m_string.isEmpty();
	}

	const char*
	sz() const {
		return m_string.sz();
	}

	DataPtr
	getPtr() {
		return m_ptr.m_p || m_string.isEmpty() ? m_ptr : (m_ptr = strDup(m_string));
	}

	void
	clear() {
		m_string.clear();
		m_ptr = g_nullDataPtr;
	}

	void
	markGcRoots(jnc::GcHeap* gcHeap) {
		gcHeap->markDataPtr(m_ptr);
	}
};

#endif

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

class NoCollectRegion {
protected:
	GcHeap* m_gcHeap;
	bool m_canCollectOnLeave;

public:
	NoCollectRegion(
		GcHeap* gcHeap,
		bool canCollectOnLeave
	) {
		init(gcHeap, canCollectOnLeave);
	}

	NoCollectRegion(
		Runtime* runtime,
		bool canCollectOnLeave
	) {
		init(runtime->getGcHeap(), canCollectOnLeave);
	}

	NoCollectRegion(bool canCollectOnLeave) {
		GcHeap* gcHeap = getCurrentThreadGcHeap();
		JNC_ASSERT(gcHeap);

		init(gcHeap, canCollectOnLeave);
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
