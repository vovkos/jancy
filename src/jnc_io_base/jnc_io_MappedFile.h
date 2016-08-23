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
	JNC_CDECL
	setDynamicViewLimit (size_t limit);

	void
	JNC_CDECL
	setSize (uint64_t size);

	bool
	JNC_CDECL
	open (
		DataPtr namePtr,
		uint_t flags
		);

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

//.............................................................................

} // namespace io
} // namespace jnc
