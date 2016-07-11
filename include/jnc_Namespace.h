#pragma once

typedef struct jnc_Namespace jnc_Namespace;
typedef struct jnc_Function jnc_Function;
typedef struct jnc_Property jnc_Property;

//.............................................................................

jnc_Function*
jnc_Namespace_findFunction (
	jnc_Namespace* nspace,
	const char* name
	);

jnc_Property*
jnc_Namespace_findProperty (
	jnc_Namespace* nspace,
	const char* name
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_Namespace
{
#ifdef __cplusplus
	jnc_Function*
	findFunction (const char* name)
	{
		return jnc_Namespace_findFunction (this, name);
	}

	jnc_Property*
	findProperty (const char* name)
	{
		return jnc_Namespace_findProperty (this, name);
	}
#endif // __cplusplus
};

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

typedef jnc_Namespace Namespace;

//.............................................................................

} // namespace jnc

#endif // __cplusplus
