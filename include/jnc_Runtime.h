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

	Use functions from the :ref:`Runtime<cid-runtime>` group to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

enum jnc_RuntimeDef
{
#if (JNC_PTR_SIZE == 8)
	jnc_RuntimeDef_StackSizeLimit    = 1 * 1024 * 1024, // 1MB std stack limit
	jnc_RuntimeDef_MinStackSizeLimit = 32 * 1024,       // 32KB min stack
#else
	jnc_RuntimeDef_StackSizeLimit    = 512 * 1024,      // 512KB std stack
	jnc_RuntimeDef_MinStackSizeLimit = 16 * 1024,       // 16KB min stack
#endif
};

typedef enum jnc_RuntimeDef jnc_RuntimeDef;

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
size_t
jnc_Runtime_getStackSizeLimit(jnc_Runtime* runtime);

JNC_EXTERN_C
bool_t
jnc_Runtime_setStackSizeLimit(
	jnc_Runtime* runtime,
	size_t sizeLimit
	);

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

JNC_EXTERN_C
void
jnc_Runtime_checkStackOverflow(jnc_Runtime* runtime);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)
struct jnc_Runtime
{
	static
	jnc_Runtime*
	create()
	{
		return jnc_Runtime_create();
	}

	void
	destroy()
	{
		jnc_Runtime_destroy(this);
	}

	jnc_Module*
	getModule()
	{
		return jnc_Runtime_getModule(this);
	}

	jnc_GcHeap*
	getGcHeap()
	{
		return jnc_Runtime_getGcHeap(this);
	}

	bool
	isAborted()
	{
		return jnc_Runtime_isAborted(this) != 0;
	}

	size_t
	getStackSizeLimit()
	{
		return jnc_Runtime_getStackSizeLimit(this);
	}

	bool
	setStackSizeLimit(size_t sizeLimit)
	{
		return jnc_Runtime_setStackSizeLimit(this, sizeLimit) != 0;
	}

	bool
	startup(jnc_Module* module)
	{
		return jnc_Runtime_startup(this, module) != 0;
	}

	void
	shutdown()
	{
		jnc_Runtime_shutdown(this);
	}

	void
	abort()
	{
		jnc_Runtime_abort(this);
	}

	void
	initializeCallSite(jnc_CallSite* callSite)
	{
		jnc_Runtime_initializeCallSite(this, callSite);
	}

	void
	uninitializeCallSite(jnc_CallSite* callSite)
	{
		jnc_Runtime_uninitializeCallSite(this, callSite);
	}

	void*
	getUserData()
	{
		return jnc_Runtime_getUserData(this);
	}

	void*
	setUserData(void* data)
	{
		return jnc_Runtime_setUserData(this, data);
	}

	void
	checkStackOverflow()
	{
		jnc_Runtime_checkStackOverflow(this);
	}
};
#endif // _JNC_CORE

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
jnc_Runtime*
jnc_getCurrentThreadRuntime();

JNC_INLINE
jnc_GcHeap*
jnc_getCurrentThreadGcHeap()
{
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

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_RuntimeDef RuntimeDef;

const RuntimeDef
	RuntimeDef_StackSizeLimit    = jnc_RuntimeDef_StackSizeLimit,
	RuntimeDef_MinStackSizeLimit = jnc_RuntimeDef_MinStackSizeLimit;

//..............................................................................

inline
Runtime*
getCurrentThreadRuntime()
{
	return jnc_getCurrentThreadRuntime();
}

inline
GcHeap*
getCurrentThreadGcHeap()
{
	return jnc_getCurrentThreadGcHeap();
}

inline
Tls*
getCurrentThreadTls()
{
	return jnc_getCurrentThreadTls();
}

inline
void
dynamicThrow()
{
	return jnc_dynamicThrow();
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
void
primeClass(
	Box* box,
	Box* root,
	ClassType* type,
	const void* vtable = NULL // if null then vtable of class type will be used
	)
{
	jnc_primeClass(box, root, type, vtable);
}

inline
void
primeClass(
	Box* box,
	ClassType* type,
	const void* vtable = NULL // if null then vtable of class type will be used
	)
{
	jnc_primeClass(box, box, type, vtable);
}

template <typename T>
void
primeClass(
	Module* module,
	ClassBoxBase<T>* p,
	Box* root
	)
{
	jnc_primeClass(p, root, T::getType(module), T::getVTable());
}

template <typename T>
void
primeClass(
	Module* module,
	ClassBoxBase<T>* p
	)
{
	primeClass(p, p, T::getType(module), T::getVTable());
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
IfaceHdr*
strengthenClassPtr(IfaceHdr* iface)
{
	return jnc_strengthenClassPtr(iface);
}

template <typename T>
T*
createClass(Runtime* runtime)
{
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
	)
{
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
	)
{
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
	)
{
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
	)
{
	ClassType* type = T::getType(runtime->getModule());
	T* p = (T*)runtime->getGcHeap()->allocateClass(type);
	construct(p, arg1, arg2, arg3, arg4);

	return p;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename T>
DataPtr
createData(Runtime* runtime)
{
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
	)
{
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
	)
{
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
	)
{
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
	)
{
	Type* type = T::getType(runtime->getModule());
	DataPtr ptr = runtime->getGcHeap()->allocateData(type);
	construct((T*)ptr.m_p, arg1, arg2, arg3, arg4);

	return ptr;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
size_t
strLen(DataPtr ptr)
{
	return jnc_strLen(ptr);
}

inline
DataPtr
strDup(
	const char* p,
	size_t length = -1
	)
{
	return jnc_strDup(p, length);
}

#ifdef _AXL_SL_STRING_H

inline
DataPtr
strDup(const axl::sl::StringRef& string)
{
	return jnc_strDup(string.cp(), string.getLength());
}

#endif

inline
DataPtr
memDup(
	const void* p,
	size_t size
	)
{
	return jnc_memDup(p, size);
}

//..............................................................................

class AutoRuntime
{
protected:
	Runtime* m_runtime;

public:
	AutoRuntime()
	{
		m_runtime = jnc_Runtime_create();
	}

	~AutoRuntime()
	{
		if (m_runtime)
			jnc_Runtime_destroy(m_runtime);
	}

	operator Runtime* () const
	{
		return m_runtime;
	}

	Runtime*
	operator -> () const
	{
		return m_runtime;
	}

	Runtime*
	p() const
	{
		return m_runtime;
	}
};

//..............................................................................

class ScopedNoCollectRegion
{
protected:
	GcHeap* m_gcHeap;
	bool m_canCollectOnLeave;

public:
	ScopedNoCollectRegion(
		GcHeap* gcHeap,
		bool canCollectOnLeave
		)
	{
		init(gcHeap, canCollectOnLeave);
	}

	ScopedNoCollectRegion(
		Runtime* runtime,
		bool canCollectOnLeave
		)
	{
		init(jnc_Runtime_getGcHeap(runtime), canCollectOnLeave);
	}

	ScopedNoCollectRegion(bool canCollectOnLeave)
	{
		GcHeap* gcHeap = getCurrentThreadGcHeap();
		JNC_ASSERT(gcHeap);

		init(gcHeap, canCollectOnLeave);
	}

	~ScopedNoCollectRegion()
	{
		jnc_GcHeap_leaveNoCollectRegion(m_gcHeap, m_canCollectOnLeave);
	}

protected:
	void
	init(
		GcHeap* gcHeap,
		bool canCollectOnLeave
		)
	{
		m_gcHeap = gcHeap;
		m_canCollectOnLeave = canCollectOnLeave;
		jnc_GcHeap_enterNoCollectRegion(m_gcHeap);
	}
};

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
