#include "pch.h"
#include "jnc_io_SshLib.h"
#include "jnc_io_Ssh.h"

namespace jnc {
namespace io {

//.............................................................................

JNC_DEFINE_LIB (
	SshLib,
	g_sshLibGuid,
	"SshLib",
	"Jancy libSSH2 wrapper extension library"
	)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE (SshLib)
	JNC_LIB_FORCED_IMPORT ("io_Ssh.jnc")
JNC_END_LIB_SOURCE_FILE_TABLE ()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE (SshLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (SshChannel)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE ()

JNC_BEGIN_LIB_FUNCTION_MAP (SshLib)
	JNC_MAP_TYPE (SshChannel)
JNC_END_LIB_FUNCTION_MAP ()

//.............................................................................

} // namespace io
} // namespace jnc


jnc_DynamicExtensionLibHost* jnc_g_dynamicExtensionLibHost;

JNC_EXTERN_C
AXL_EXPORT
jnc_ExtensionLib* 
jncDynamicExtensionLibMain (jnc_DynamicExtensionLibHost* host)
{
	jnc_g_dynamicExtensionLibHost = host;
	return jnc::io::SshLib_getLib ();
}

//.............................................................................
