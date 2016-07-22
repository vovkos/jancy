#include "pch.h"
#include "jnc_StructType.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif defined (_JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
#endif

//.............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_ModuleItemDecl*
jnc_StructField_getItemDecl (jnc_StructField* field)
{
	return field;
}

JNC_EXTERN_C
jnc_Type*
jnc_StructField_getType (jnc_StructField* field)
{
	return field->getType ();
}

JNC_EXTERN_C
size_t
jnc_StructField_getOffset (jnc_StructField* field)
{
	return field->getOffset ();
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//.............................................................................
