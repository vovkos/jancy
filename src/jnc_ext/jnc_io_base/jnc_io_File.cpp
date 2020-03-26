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

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	File,
	"io.File",
	g_ioLibGuid,
	IoLibCacheSlot_File,
	File,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(File)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<File>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<File>)
	JNC_MAP_CONST_PROPERTY("m_kind", &File::getKind)
	JNC_MAP_CONST_PROPERTY("m_size", &File::getSize)
	JNC_MAP_PROPERTY("m_position", &File::getPosition, &File::setPosition)
	JNC_MAP_FUNCTION("open",  &File::open)
	JNC_MAP_FUNCTION("close", &File::close)
	JNC_MAP_FUNCTION("setSize", &File::setSize)
	JNC_MAP_FUNCTION("read",  &File::read)
	JNC_MAP_FUNCTION("write", &File::write)
	JNC_MAP_FUNCTION("flush", &File::flush)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

FileKind
getFileKind(const axl::io::File& file)
{
	if (!file.isOpen())
		return FileKind_Unknown;

#if (_JNC_OS_WIN)
	static FileKind fileKindTable[] =
	{
		FileKind_Unknown, // FILE_TYPE_UNKNOWN   (0x0000)
		FileKind_Disk,    // FILE_TYPE_DISK      (0x0001)
		FileKind_Serial,  // FILE_TYPE_CHAR      (0x0002)
		FileKind_Pipe,    // FILE_TYPE_PIPE      (0x0003)
	};

	dword_t type = file.m_file.getType();
	return type < countof(fileKindTable) ? fileKindTable[type] : FileKind_Unknown;
#else
	struct stat st;
	int result = ::fstat(file.m_file, &st);

	return
		result == -1 ? FileKind_Unknown :
		S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode) ? FileKind_Pipe :
		S_ISCHR(st.st_mode) ? FileKind_Serial :
		S_ISREG(st.st_mode) ? FileKind_Disk :
		FileKind_Unknown;
#endif
}

//..............................................................................

bool
JNC_CDECL
File::open(
	DataPtr namePtr,
	uint_t flags
	)
{
	close();

	bool result = m_file.open((const char*) namePtr.m_p, flags);
	if (!result)
		return false;

	m_isOpen = true;
	return true;
}

void
JNC_CDECL
File::close()
{
	m_file.close();
	m_isOpen = false;
}

//..............................................................................

} // namespace io
} // namespace jnc
