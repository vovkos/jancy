#pragma once

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE (MappedFile)

//.............................................................................

class MappedFile: public IfaceHdr
{
	friend class IoThread;

protected:
	size_t m_dynamicViewLimit;

	bool m_isOpen;
	uint64_t m_size;

protected:
	Runtime* m_runtime;
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
