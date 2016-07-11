#pragma once

typedef struct jnc_Function jnc_Function;
typedef struct jnc_Property jnc_Property;

//.............................................................................

jnc_Function*
jnc_Property_getGetter (jnc_Property* self);

jnc_Function*
jnc_Property_getSetter (jnc_Property* self);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_Property
{
#ifdef __cplusplus
	jnc_Function*
	getGetter ()
	{
		return jnc_Property_getGetter (this);
	}

	jnc_Function*
	getSetter ()
	{
		return jnc_Property_getSetter (this);
	}
#endif
};

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

typedef jnc_Property Property;

//.............................................................................

} // namespace jnc

#endif // __cplusplus
