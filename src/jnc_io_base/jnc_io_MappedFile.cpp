#include "pch.h"
#include "jnc_io_MappedFile.h"

namespace jnc {
namespace io {

//.............................................................................

MappedFile::MappedFile ()
{
	m_runtime = rt::getCurrentThreadRuntime ();
	m_dynamicViewLimit = axl::io::MappedFile::DefaultsKind_MaxDynamicViewCount;
	m_size = 0;
	m_isOpen = false;
}

bool
AXL_CDECL
MappedFile::open (
	rt::DataPtr namePtr,
	uint_t flags
	)
{
	bool result = m_file.open ((const char*) namePtr.m_p, flags);
	if (!result)
		return false;

	m_size = m_file.getSize ();
	return true;
}

rt::DataPtr
AXL_CDECL
MappedFile::view (
	MappedFile* self,
	uint64_t offset,
	size_t size,
	bool isPermanent
	)
{
	rt::DataPtr ptr = { 0 };

	void* p = self->m_file.view (offset, size, isPermanent);
	if (!p)
		return ptr;
	
	// lifetime of the resulting view is not guaranteed
	// but its the best we can do with the direct use of axl::io::MappedFile

	ptr.m_p = p;
	ptr.m_validator = rt::createDataPtrValidator (self->m_runtime, self->m_box, p, size);
	return ptr;
}

void
AXL_CDECL 
MappedFile::setDynamicViewLimit (size_t limit)
{
	bool result = m_file.setup (limit, axl::io::MappedFile::DefaultsKind_ReadAheadSize);
	if (result)
		m_dynamicViewLimit = limit;
}

void
AXL_CDECL 
MappedFile::setSize (uint64_t size)
{
	bool result = m_file.setSize (size);
	if (result)
		m_size = size;
}

//.............................................................................

} // namespace io
} // namespace jnc
