#include "pch.h"
#include "jnc_Runtime.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif (defined _JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
#endif

//.............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_Module*
jnc_Runtime_getModule (jnc_Runtime* runtime)
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_getModuleFunc (runtime);
}

JNC_EXTERN_C
jnc_GcHeap*
jnc_Runtime_getGcHeap (jnc_Runtime* runtime)
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_getGcHeapFunc (runtime);
}

JNC_EXTERN_C
void
jnc_Runtime_initializeThread (
	jnc_Runtime* runtime,
	jnc_ExceptionRecoverySnapshot* ers
	)
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_initializeThreadFunc (runtime, ers);
}

JNC_EXTERN_C
void
jnc_Runtime_uninitializeThread (
	jnc_Runtime* runtime,
	jnc_ExceptionRecoverySnapshot* ers
	)
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_uninitializeThreadFunc (runtime, ers);
}

JNC_EXTERN_C
jnc_Runtime*
jnc_getCurrentThreadRuntime ()
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_getCurrentThreadRuntimeFunc ();
}

JNC_EXTERN_C
void
jnc_primeClass (
	jnc_Box* box,
	jnc_Box* root,
	jnc_ClassType* type,
	void* vtable
	)
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_primeClassFunc (box, root, type, vtable);
}

JNC_EXTERN_C
size_t 
jnc_strLen (jnc_DataPtr ptr)
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_strLenFunc (ptr);
}

JNC_EXTERN_C
jnc_DataPtr
jnc_strDup (
	const char* p,
	size_t length
	)
{
	return jnc_g_dynamicExtensionLibHost->m_runtimeFuncTable->m_strDupFunc (p, length);
}

JNC_EXTERN_C
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
jnc_Module*
jnc_Runtime_getModule (jnc_Runtime* runtime)
{
	return runtime->getModule ();
}

JNC_EXTERN_C
jnc_GcHeap*
jnc_Runtime_getGcHeap (jnc_Runtime* runtime)
{
	return &runtime->m_gcHeap;
}

JNC_EXTERN_C
void
jnc_Runtime_initializeThread (
	jnc_Runtime* runtime,
	jnc_ExceptionRecoverySnapshot* ers
	)
{
	runtime->initializeThread (ers);
}

JNC_EXTERN_C
void
jnc_Runtime_uninitializeThread (
	jnc_Runtime* runtime,
	jnc_ExceptionRecoverySnapshot* ers
	)
{
	runtime->uninitializeThread (ers);
}

JNC_EXTERN_C
jnc_Runtime*
jnc_getCurrentThreadRuntime ()
{
	return jnc::rt::getCurrentThreadRuntime ();
}

static
void
primeIface (
	jnc_Box* box,
	jnc_Box* root,
	jnc_IfaceHdr* iface,
	jnc_ClassType* type,
	void* vtable
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
		ASSERT (slot->getType ()->getTypeKind () == ct::TypeKind_Class);
		
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
		ASSERT (field->getType ()->getTypeKind () == ct::TypeKind_Class);

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
void
jnc_primeClass (
	jnc_Box* box,
	jnc_Box* root,
	jnc_ClassType* type,
	void* vtable
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
jnc_DataPtr
jnc_strDup (
	const char* p,
	size_t length
	)
{
	using namespace jnc;

	if (length == -1)
		length = axl_strlen (p);

	if (!length)
		return g_nullPtr;

	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	DataPtr resultPtr = runtime->m_gcHeap.tryAllocateBuffer (length + 1);
	if (!resultPtr.m_p)
		return g_nullPtr;

	if (p)
		memcpy (resultPtr.m_p, p, length);

	return resultPtr;
}

JNC_EXTERN_C
jnc_DataPtr
jnc_memDup (
	const void* p,
	size_t size
	)
{
	using namespace jnc;

	if (!size)
		return g_nullPtr;

	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	DataPtr resultPtr = runtime->m_gcHeap.tryAllocateBuffer (size);
	if (!resultPtr.m_p)
		return g_nullPtr;

	if (p)
		memcpy (resultPtr.m_p, p, size);

	return resultPtr;
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB