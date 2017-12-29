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
#include "jnc_io_File.h"
#include "jnc_io_IoLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	File,
	"io.File",
	g_ioLibGuid,
	IoLibCacheSlot_File,
	File,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (File)
	JNC_MAP_CONSTRUCTOR (&jnc::construct <File>)
	JNC_MAP_DESTRUCTOR (&jnc::destruct <File>)
	JNC_MAP_PROPERTY ("m_size", &File::getSize, &File::setSize)
	JNC_MAP_PROPERTY ("m_position", &File::getPosition, &File::setPosition)
	JNC_MAP_FUNCTION ("open",  &File::open)
	JNC_MAP_FUNCTION ("close", &File::close)
	JNC_MAP_FUNCTION ("read",  &File::read)
	JNC_MAP_FUNCTION ("write",  &File::write)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

File::File ()
{
	m_runtime = getCurrentThreadRuntime ();
	m_isOpen = false;
}

bool
JNC_CDECL
File::open (
	DataPtr namePtr,
	uint_t flags
	)
{
	bool result = m_file.open ((const char*) namePtr.m_p, flags);
	if (!result)
		propagateLastError ();

	return result;
}

//..............................................................................

} // namespace io
} // namespace jnc
