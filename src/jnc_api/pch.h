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

#pragma once

#if (!_JNC_DYNAMIC_EXTENSION_LIB)
#	define _JNC_CORE 1
#	include "jnc_ct_Pch.h"
#endif // _JNC_DYNAMIC_EXTENSION_LIB

#include "axl_sl_BitIdx.h"
#include "axl_err_Error.h"

#if (_JNC_IO_USB)
#	include "axl_io_UsbError.h"
#endif

using namespace axl;
