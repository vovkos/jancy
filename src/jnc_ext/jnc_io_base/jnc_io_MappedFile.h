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

JNC_DECLARE_OPAQUE_CLASS_TYPE (MappedFile)

//..............................................................................

class MappedFile: public IfaceHdr
{
	friend class IoThread;

protected:
	size_t m_dynamicViewLimit;

	bool m_isOpen;

protected:
	Runtime* m_runtime;
	axl::io::MappedFile m_file;

public:
	MappedFile ();

	void
	JNC_CDECL
	setDynamicViewLimit (size_t limit);

	uint64_t
	JNC_CDECL
	getSize ()
	{
		return m_file.getSize ();
	}

	bool
	JNC_CDECL
	setSize (uint64_t size)
	{
		return m_file.setSize (size);
	}

	bool
	JNC_CDECL
	open (
		DataPtr namePtr,
		uint_t flags
		)
	{
		return m_file.open ((const char*) namePtr.m_p, flags);
	}

	void
	JNC_CDECL
	close ()
	{
		m_file.close ();
	}

	static
	DataPtr
	JNC_CDECL
	view (
		MappedFile* self,
		uint64_t offset,
		size_t size,
		bool isPermanent
		);
};

//..............................................................................

} // namespace io
} // namespace jnc
