#pragma once

#include "jnc_Namespace.h"
#include "jnc_DerivableType.h"
#include "jnc_Function.h"
#include "jnc_Property.h"

//.............................................................................

JNC_EXTERN_C
jnc_Namespace*
jnc_Module_getGlobalNamespace (jnc_Module* module);

JNC_EXTERN_C
jnc_ModuleItem*
jnc_Module_findItem (
	jnc_Module* module,
	const char* name,
	const jnc_Guid* libGuid,
	size_t itemCacheSlot
	);

JNC_EXTERN_C
void
jnc_Module_mapFunction (
	jnc_Module* module,
	jnc_Function* function,
	void* p
	);

JNC_EXTERN_C
void
jnc_Module_addSource (
	jnc_Module* module,
	int isForced,
	const char* fileName,
	const char* source,
	size_t length = -1
	);

JNC_EXTERN_C
void
jnc_Module_addOpaqueClassTypeInfo (
	jnc_Module* module,
	const char* qualifiedName,
	const jnc_OpaqueClassTypeInfo* info
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)
struct jnc_Module
{
	jnc_Namespace*
	getGlobalNamespace ()
	{
		return jnc_Module_getGlobalNamespace (this);
	}

	jnc_ModuleItem*
	findItem (
		const char* name,
		const jnc_Guid* libGuid,
		size_t itemCacheSlot
		)
	{
		return jnc_Module_findItem (this, name, libGuid, itemCacheSlot);
	}

	void
	mapFunction (
		jnc_Function* function,
		void* p
		)
	{
		jnc_Module_mapFunction (this, function, p);
	}


	void
	addSource (
		bool isForced,
		const char* fileName,
		const char* source,
		size_t length = -1
		)
	{
		jnc_Module_addSource (this, isForced, fileName, source, length);
	}

	void
	addSource (
		const char* fileName,
		const char* source,
		size_t length = -1
		)
	{
		jnc_Module_addSource (this, false, fileName, source, length);
	}

	void
	addOpaqueClassTypeInfo (
		const char* qualifiedName,
		const jnc_OpaqueClassTypeInfo* info
		)
	{
		jnc_Module_addOpaqueClassTypeInfo (this, qualifiedName, info);
	}
};
#endif // _JNC_CORE

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
jnc_DerivableType*
jnc_verifyModuleItemIsDerivableType (
	jnc_ModuleItem* item,
	const char* name
	);

JNC_EXTERN_C
jnc_ClassType*
jnc_verifyModuleItemIsClassType (
	jnc_ModuleItem* item,
	const char* name
	);

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

inline
jnc_DerivableType*
verifyModuleItemIsDerivableType (
	ModuleItem* item,
	const char* name
	)
{
	return jnc_verifyModuleItemIsDerivableType (item, name);
}

inline
jnc_ClassType*
verifyModuleItemIsClassType (
	ModuleItem* item,
	const char* name
	)
{
	jnc_verifyModuleItemIsClassType (item, name);
}

//.............................................................................

} // namespace jnc

#endif // __cplusplus
