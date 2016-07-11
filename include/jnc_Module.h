#pragma once

#include "jnc_Def.h"

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
bool
jnc_Module_addImport (
	jnc_Module* module,
	const char* fileName
	);

JNC_EXTERN_C
void
jnc_Module_addSource (
	jnc_Module* module,
	const char* fileName,
	const char* source,
	size_t length = -1
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
		return jnc_Module_mapFunction (this, function, p);
	}

	bool
	addImport (const char* fileName)
	{
		return jnc_Module_addImport (this, fileName);
	}

	void
	addSource (
		const char* fileName,
		const char* source,
		size_t length = -1
		)
	{
		jnc_Module_addSource (this, fileName, source, length);
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

typedef jnc_Module Module;
typedef jnc_ModuleItem ModuleItem;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

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
