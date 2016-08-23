#include "pch.h"
#include "jnc_io_IoLib.h"

#include "jnc_io_Serial.h"
#include "jnc_io_Socket.h"
#include "jnc_io_SocketAddress.h"
#include "jnc_io_SocketAddressResolver.h"
#include "jnc_io_NetworkAdapter.h"
#include "jnc_io_MappedFile.h"
#include "jnc_io_FileStream.h"

#if (_JNC_ENV == JNC_ENV_WIN)
#	include "jnc_io_NamedPipe.h"
#	include "jnc_io_Mailslot.h"
#endif

namespace jnc {
namespace io {

//.............................................................................

JNC_DEFINE_LIB (
	IoLib,
	g_ioLibGuid,
	"IoLib",
	"Jancy standard IO extension library"
	)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE (IoLib)
JNC_END_LIB_SOURCE_FILE_TABLE ()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE (IoLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (Serial)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (Socket)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (SocketAddressResolver)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (MappedFile)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (FileStream)
#if (_JNC_ENV == JNC_ENV_WIN)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (NamedPipe)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (Mailslot)
#endif
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE ()

JNC_BEGIN_LIB_FUNCTION_MAP (IoLib)
	JNC_MAP_TYPE (Serial)
	JNC_MAP_TYPE (Socket)
	JNC_MAP_TYPE (Address_ip4)
	JNC_MAP_TYPE (Address_ip6)
	JNC_MAP_TYPE (SocketAddress_ip4)
	JNC_MAP_TYPE (SocketAddress_ip6)
	JNC_MAP_TYPE (SocketAddress)
	JNC_MAP_TYPE (SocketAddressResolver)
	JNC_MAP_TYPE (MappedFile)
	JNC_MAP_TYPE (FileStream)
#if (_JNC_ENV == JNC_ENV_WIN)
	JNC_MAP_TYPE (NamedPipe)
	JNC_MAP_TYPE (Mailslot)
#endif
	JNC_MAP_FUNCTION ("io.createNetworkAdapterDescList", &createNetworkAdapterDescList)
	JNC_MAP_FUNCTION ("io.createSerialPortDescList",     &createSerialPortDescList)
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
	return jnc::io::IoLib_getLib ();
}

//.............................................................................
