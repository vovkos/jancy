#include "pch.h"
#include "jnc_Variant.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif (defined _JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
#endif

//.............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

#else // _JNC_DYNAMIC_EXTENSION_LIB

int
jnc_Variant_cast (
	jnc_Variant* variant,
	jnc_Type* type,
	void* buffer
	)
{
	using namespace jnc;
	ct::Module* module = type->getModule ();

	ct::Value opValue (variant, module->m_typeMgr.getPrimitiveType (TypeKind_Variant));
	ct::CastOperator* castOp = module->m_operatorMgr.getStdCastOperator (ct::StdCast_FromVariant);

	memset (buffer, 0, type->getSize ());
	return castOp->constCast (opValue, type, buffer);
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//.............................................................................
