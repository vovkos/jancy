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
#elif defined (_JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_rt_ExceptionMgr.h"
#	include "jnc_ct_Module.h"
#endif

#include "jnc_Runtime.h"

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Runtime*
jnc_Runtime_create ()
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_createFunc ();
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Runtime_destroy (jnc_Runtime* runtime)
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_destroyFunc (runtime);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Module*
jnc_Runtime_getModule (jnc_Runtime* runtime)
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_getModuleFunc (runtime);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_GcHeap*
jnc_Runtime_getGcHeap (jnc_Runtime* runtime)
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_getGcHeapFunc (runtime);
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_Runtime_getStackSizeLimit (jnc_Runtime* runtime)
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_getStackSizeLimitFunc (runtime);
}

JNC_EXTERN_C
JNC_EXPORT_O
int
jnc_Runtime_setStackSizeLimit (
	jnc_Runtime* runtime,
	size_t sizeLimit
	)
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_setStackSizeLimitFunc (runtime, sizeLimit);
}

JNC_EXTERN_C
JNC_EXPORT_O
int
jnc_Runtime_startup (
	jnc_Runtime* runtime,
	jnc_Module* module
	)
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_startupFunc (runtime, module);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Runtime_shutdown (jnc_Runtime* runtime)
{
	jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_shutdownFunc (runtime);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Runtime_initializeCallSite (
	jnc_Runtime* runtime,
	jnc_CallSite* callSite
	)
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_initializeCallSiteFunc (runtime, callSite);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Runtime_uninitializeCallSite (
	jnc_Runtime* runtime,
	jnc_CallSite* callSite
	)
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_uninitializeCallSiteFunc (runtime, callSite);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_SjljFrame*
jnc_Runtime_setSjljFrame (
	jnc_Runtime* runtime,
	jnc_SjljFrame* frame
	)
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_setSjljFrameFunc (runtime, frame);
}

JNC_EXTERN_C
JNC_EXPORT_O
void*
jnc_Runtime_getUserData (jnc_Runtime* runtime)
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_getUserDataFunc (runtime);
}

JNC_EXTERN_C
JNC_EXPORT_O
void*
jnc_Runtime_setUserData (
	jnc_Runtime* runtime,
	void* data
	)
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_setUserDataFunc (runtime, data);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Runtime_checkStackOverflow (jnc_Runtime* runtime)
{
	jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_checkStackOverflowFunc (runtime);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Runtime*
jnc_getCurrentThreadRuntime ()
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_getCurrentThreadRuntimeFunc ();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Tls*
jnc_getCurrentThreadTls ()
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_getCurrentThreadTlsFunc ();
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_dynamicThrow ()
{
	jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_dynamicThrowFunc ();
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_primeClass (
	jnc_Box* box,
	jnc_Box* root,
	jnc_ClassType* type,
	const void* vtable
	)
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_primeClassFunc (box, root, type, vtable);
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_strLen (jnc_DataPtr ptr)
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_strLenFunc (ptr);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DataPtr
jnc_strDup (
	const char* p,
	size_t length
	)
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_strDupFunc (p, length);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DataPtr
jnc_memDup (
	const void* p,
	size_t size
	)
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_memDupFunc (p, size);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Runtime*
jnc_Runtime_create ()
{
	return AXL_MEM_NEW (jnc_Runtime);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Runtime_destroy (jnc_Runtime* runtime)
{
	return AXL_MEM_DELETE (runtime);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Module*
jnc_Runtime_getModule (jnc_Runtime* runtime)
{
	return runtime->getModule ();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_GcHeap*
jnc_Runtime_getGcHeap (jnc_Runtime* runtime)
{
	return runtime->getGcHeap ();
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_Runtime_getStackSizeLimit (jnc_Runtime* runtime)
{
	return runtime->getStackSizeLimit ();
}

JNC_EXTERN_C
JNC_EXPORT_O
int
jnc_Runtime_setStackSizeLimit (
	jnc_Runtime* runtime,
	size_t sizeLimit
	)
{
	return runtime->setStackSizeLimit (sizeLimit);
}

JNC_EXTERN_C
JNC_EXPORT_O
int
jnc_Runtime_startup (
	jnc_Runtime* runtime,
	jnc_Module* module
	)
{
	return runtime->startup (module);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Runtime_shutdown (jnc_Runtime* runtime)
{
	return runtime->shutdown ();
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Runtime_initializeCallSite (
	jnc_Runtime* runtime,
	jnc_CallSite* callSite
	)
{
	runtime->initializeCallSite (callSite);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Runtime_uninitializeCallSite (
	jnc_Runtime* runtime,
	jnc_CallSite* callSite
	)
{
	runtime->uninitializeCallSite (callSite);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_SjljFrame*
jnc_Runtime_setSjljFrame (
	jnc_Runtime* runtime,
	jnc_SjljFrame* frame
	)
{
	return runtime->setSjljFrame (frame);
}

JNC_EXTERN_C
JNC_EXPORT_O
void*
jnc_Runtime_getUserData (jnc_Runtime* runtime)
{
	return runtime->m_userData;
}

JNC_EXTERN_C
JNC_EXPORT_O
void*
jnc_Runtime_setUserData (
	jnc_Runtime* runtime,
	void* data
	)
{
	return (void*) sys::atomicXchg ((size_t volatile*) &runtime->m_userData, (size_t) data);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Runtime_checkStackOverflow (jnc_Runtime* runtime)
{
	return runtime->checkStackOverflow ();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Runtime*
jnc_getCurrentThreadRuntime ()
{
	return jnc::rt::getCurrentThreadRuntime ();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Tls*
jnc_getCurrentThreadTls ()
{
	return jnc::rt::getCurrentThreadTls ();
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_dynamicThrow ()
{
	jnc::rt::Runtime::dynamicThrow ();
}

static
void
primeIface (
	jnc_Box* box,
	jnc_Box* root,
	jnc_IfaceHdr* iface,
	jnc_ClassType* type,
	const void* vtable
	)
{
	using namespace jnc;

	iface->m_vtable = vtable;
	iface->m_box = box;

	// primeClass all the base types

	sl::Array <ct::BaseTypeSlot*> baseTypePrimeArray = type->getBaseTypePrimeArray ();
	size_t count = baseTypePrimeArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		ct::BaseTypeSlot* slot = baseTypePrimeArray [i];
		ASSERT (slot->getType ()->getTypeKind () == TypeKind_Class);

		primeIface (
			box,
			root,
			(IfaceHdr*) ((char*) iface + slot->getOffset ()),
			(ct::ClassType*) slot->getType (),
			(void**) vtable + slot->getVTableIndex ()
			);
	}

	// primeClass all the class fields

	sl::Array <ct::StructField*> fieldPrimeArray = type->getClassMemberFieldArray ();
	count = fieldPrimeArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		ct::StructField* field = fieldPrimeArray [i];
		ASSERT (field->getType ()->getTypeKind () == TypeKind_Class);

		ct::ClassType* fieldType = (ct::ClassType*) field->getType ();
		Box* fieldBox = (Box*) ((char*) iface + field->getOffset ());

		primeClass (
			fieldBox,
			root,
			fieldType
			);
	}
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_primeClass (
	jnc_Box* box,
	jnc_Box* root,
	jnc_ClassType* type,
	const void* vtable
	)
{
	ASSERT (root <= box);

	if (!vtable)
		vtable = type->getVTableVariable ()->getStaticData ();

	memset (box, 0, type->getSize ());

	box->m_type = type;
	box->m_flags = jnc_BoxFlag_ClassMark | jnc_BoxFlag_DataMark | jnc_BoxFlag_WeakMark;
	box->m_rootOffset = (char*) box - (char*) root;

	primeIface (box, root, (jnc_IfaceHdr*) (box + 1), type, vtable);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_IfaceHdr*
jnc_strengthenClassPtr (jnc_IfaceHdr* iface)
{
	using namespace jnc;

	if (!iface)
		return NULL;

	ASSERT (iface->m_box->m_type->getTypeKind () == TypeKind_Class);
	ClassType* classType = (ClassType*) iface->m_box->m_type;
	ClassTypeKind classTypeKind = classType->getClassTypeKind ();

	return classTypeKind == ClassTypeKind_FunctionClosure || classTypeKind == ClassTypeKind_PropertyClosure ?
		((ct::ClosureClassType*) classType)->strengthen (iface) :
		(iface->m_box->m_flags & BoxFlag_ClassMark) && !(iface->m_box->m_flags & BoxFlag_Zombie) ? iface : NULL;
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_strLen (jnc_DataPtr ptr)
{
	if (!ptr.m_validator || ptr.m_p < ptr.m_validator->m_rangeBegin)
		return 0;

	char* p0 = (char*) ptr.m_p;
	char* end = (char*) ptr.m_validator->m_rangeEnd;

	char* p = p0;
	while (p < end && *p)
		p++;

	return p - p0;
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DataPtr
jnc_strDup (
	const char* p,
	size_t length
	)
{
	using namespace jnc;

	if (length == -1)
		length = strlen_s (p);

	if (!length)
		return g_nullPtr;

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	DataPtr resultPtr = gcHeap->tryAllocateBuffer (length + 1);
	if (!resultPtr.m_p)
		return g_nullPtr;

	if (p)
		memcpy (resultPtr.m_p, p, length);

	return resultPtr;
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DataPtr
jnc_memDup (
	const void* p,
	size_t size
	)
{
	using namespace jnc;

	if (!size)
		return g_nullPtr;

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	DataPtr resultPtr = gcHeap->tryAllocateBuffer (size);
	if (!resultPtr.m_p)
		return g_nullPtr;

	if (p)
		memcpy (resultPtr.m_p, p, size);

	return resultPtr;
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB
