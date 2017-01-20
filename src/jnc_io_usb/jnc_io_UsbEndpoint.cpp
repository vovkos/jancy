//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#include "pch.h"
#include "jnc_io_UsbEndpoint.h"
#include "jnc_io_UsbLib.h"
#include "jnc_Error.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_TYPE (
	UsbEventParams,
	"io.UsbEventParams",
	g_usbLibGuid,
	UsbLibCacheSlot_UsbEventParams
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (UsbEventParams)
JNC_END_TYPE_FUNCTION_MAP ()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	UsbEndpoint,
	"io.UsbEndpoint",
	g_usbLibGuid,
	UsbLibCacheSlot_UsbEndpoint,
	UsbEndpoint,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (UsbEndpoint)
	JNC_MAP_CONSTRUCTOR (&jnc::construct <UsbEndpoint>)
	JNC_MAP_DESTRUCTOR (&jnc::destruct <UsbEndpoint>)
	JNC_MAP_CONST_PROPERTY ("m_endpointDesc", &UsbEndpoint::getEndpointDesc)
	JNC_MAP_FUNCTION ("write", &UsbEndpoint::write)
	JNC_MAP_FUNCTION ("read",  &UsbEndpoint::read)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

} // namespace io
} // namespace jnc
