#pragma once

#include "jnc_io_IoLibGlobals.h"

namespace jnc {
namespace io {

//.............................................................................

class MappedFile: public IfaceHdr
{
	friend class IoThread;

public:
	JNC_OPAQUE_CLASS_TYPE_INFO (MappedFile, NULL)

	JNC_BEGIN_TYPE_FUNCTION_MAP ("io.MappedFile", g_ioLibCacheSlot, IoLibCacheSlot_MappedFile)
		JNC_MAP_CONSTRUCTOR (&sl::construct <MappedFile>)
		JNC_MAP_DESTRUCTOR (&sl::destruct <MappedFile>)
		JNC_MAP_AUTOGET_PROPERTY ("m_dynamicViewLimit", &MappedFile::setDynamicViewLimit)
		JNC_MAP_AUTOGET_PROPERTY ("m_size", &MappedFile::setSize)
		JNC_MAP_FUNCTION ("open",  &MappedFile::open)
		JNC_MAP_FUNCTION ("close", &MappedFile::close)
		JNC_MAP_FUNCTION ("view",  &MappedFile::view)
	JNC_END_TYPE_FUNCTION_MAP ()

protected:
	size_t m_dynamicViewLimit;

	bool m_isOpen;
	uint64_t m_size;

protected:
	rt::Runtime* m_runtime;
	axl::io::MappedFile m_file;

public:
	MappedFile ();

	void
	AXL_CDECL
	setDynamicViewLimit (size_t limit);

	void
	AXL_CDECL
	setSize (uint64_t size);

	bool
	AXL_CDECL
	open (
		DataPtr namePtr,
		uint_t flags
		);

	void
	AXL_CDECL
	close ()
	{
		m_file.close ();
	}

	static
	DataPtr
	AXL_CDECL
	view (
		MappedFile* self,
		uint64_t offset,
		size_t size,
		bool isPermanent
		);
};

//.............................................................................

} // namespace io
} // namespace jnc
