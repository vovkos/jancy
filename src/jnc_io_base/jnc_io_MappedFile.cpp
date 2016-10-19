#include "pch.h"
#include "jnc_io_MappedFile.h"
#include "jnc_io_IoLib.h"
#include "jnc_Error.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	MappedFile,
	"io.MappedFile",
	g_ioLibGuid,
	IoLibCacheSlot_MappedFile,
	MappedFile,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (MappedFile)
	JNC_MAP_CONSTRUCTOR (&jnc::construct <MappedFile>)
	JNC_MAP_DESTRUCTOR (&jnc::destruct <MappedFile>)
	JNC_MAP_AUTOGET_PROPERTY ("m_dynamicViewLimit", &MappedFile::setDynamicViewLimit)
	JNC_MAP_PROPERTY ("m_size", &MappedFile::getSize, &MappedFile::setSize)
	JNC_MAP_FUNCTION ("open",  &MappedFile::open)
	JNC_MAP_FUNCTION ("close", &MappedFile::close)
	JNC_MAP_FUNCTION ("view",  &MappedFile::view)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

MappedFile::MappedFile ()
{
	m_runtime = getCurrentThreadRuntime ();
	m_dynamicViewLimit = axl::io::MappedFile::DefaultsKind_MaxDynamicViewCount;
	m_isOpen = false;
}

bool
JNC_CDECL
MappedFile::open (
	DataPtr namePtr,
	uint_t flags
	)
{
	bool result = m_file.open ((const char*) namePtr.m_p, flags);
	if (!result)
		propagateLastError ();

	return result;
}

DataPtr
JNC_CDECL
MappedFile::view (
	MappedFile* self,
	uint64_t offset,
	size_t size,
	bool isPermanent
	)
{
	void* p = self->m_file.view (offset, size, isPermanent);
	if (!p)
	{
		propagateLastError ();
		return g_nullPtr;
	}

	// lifetime of the resulting view is not guaranteed
	// but its the best we can do with the direct use of axl::io::MappedFile

	GcHeap* gcHeap = self->m_runtime->getGcHeap ();

	DataPtr ptr;
	ptr.m_p = p;
	ptr.m_validator = gcHeap->createDataPtrValidator (self->m_box, p, size);
	return ptr;
}

void
JNC_CDECL
MappedFile::setDynamicViewLimit (size_t limit)
{
	bool result = m_file.setup (limit, axl::io::MappedFile::DefaultsKind_ReadAheadSize);
	if (result)
		m_dynamicViewLimit = limit;
}

//..............................................................................

} // namespace io
} // namespace jnc
