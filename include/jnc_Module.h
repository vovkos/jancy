#pragma once

#include "jnc_RuntimeStructs.h"

typedef struct jnc_Module jnc_Module;
typedef struct jnc_Namespace jnc_Namespace;
typedef struct jnc_ModuleItem jnc_ModuleItem;
typedef struct jnc_DerivableType jnc_DerivableType;
typedef struct jnc_ClassType jnc_ClassType;
typedef struct jnc_Function jnc_Function;

//.............................................................................

jnc_Namespace*
jnc_Module_getGlobalNamespace (jnc_Module* self);

jnc_ModuleItem*
jnc_Module_findItem (
	jnc_Module* self,
	const char* name,
	size_t libCacheSlot,
	size_t itemCacheSlot
	);

void
jnc_Module_mapFunction (
	jnc_Module* self,
	jnc_Function* function,
	void* p
	);

bool
jnc_Module_addImport (
	jnc_Module* self,
	const char* fileName
	);

void
jnc_Module_addSource (
	jnc_Module* self,
	const char* fileName,
	const char* source,
	size_t size
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_Module
{
#ifdef __cplusplus
	jnc_Namespace*
	getGlobalNamespace ()
	{
		return jnc_Module_getGlobalNamespace (this);
	}

	jnc_ModuleItem*
	findItem (
		const char* name,
		size_t libCacheSlot,
		size_t itemCacheSlot
		)
	{
		return jnc_Module_findItem (this, name, libCacheSlot, itemCacheSlot);
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
		size_t size
		)
	{
		jnc_Module_addSource (this, fileName, source, size);
	}
#endif // __cplusplus
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

jnc_DerivableType*
jnc_verifyModuleItemIsDerivableType (
	jnc_ModuleItem* item,
	const char* name
	);

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
