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

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE(File)

//..............................................................................

enum FileKind
{
	FileKind_Unknown,
	FileKind_Disk,
	FileKind_Serial,
	FileKind_Pipe,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

FileKind
getFileKind(const axl::io::File& file);

//..............................................................................

class File: public IfaceHdr
{
	friend class IoThread;

public:
	bool m_isOpen;

protected:
	axl::io::File m_file;

public:
	uintptr_t
	JNC_CDECL
	getOsHandle()
	{
#if (_JNC_OS_WIN)
		return (uintptr_t)(handle_t)m_file.m_file;
#else
		return m_file.m_file;
#endif
	}

	FileKind
	JNC_CDECL
	getKind()
	{
		return getFileKind(m_file);
	}

	uint64_t
	JNC_CDECL
	getSize()
	{
		return m_file.getSize();
	}

	bool
	JNC_CDECL
	setSize(uint64_t size)
	{
		return m_file.setSize(size);
	}

	uint64_t
	JNC_CDECL
	getPosition()
	{
		return m_file.getPosition();
	}

	bool
	JNC_CDECL
	setPosition(uint64_t offset)
	{
		return m_file.setPosition(offset);
	}

	bool
	JNC_CDECL
	open(
		DataPtr namePtr,
		uint_t flags
		);

	void
	JNC_CDECL
	close();

	size_t
	JNC_CDECL
	read(
		DataPtr ptr,
		size_t size
		)
	{
		return m_file.read(ptr.m_p, size);
	}

	size_t
	JNC_CDECL
	write(
		DataPtr ptr,
		size_t size
		)
	{
		return m_file.write(ptr.m_p, size);
	}

	bool
	JNC_CDECL
	flush()
	{
		return m_file.flush();
	}
};

//..............................................................................

} // namespace io
} // namespace jnc
