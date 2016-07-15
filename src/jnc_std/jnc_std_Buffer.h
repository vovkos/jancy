#pragma once

#include "jnc_ext_ExtensionLib.h"
#include "jnc_std_StdLibGlobals.h"

namespace jnc {
namespace std {
		
//.............................................................................

struct ConstBufferRef
{
	JNC_BEGIN_TYPE_MAP ("std.ConstBufferRef", g_stdLibCacheSlot, StdLibCacheSlot_ConstBufferRef)
	JNC_END_TYPE_MAP ()

public:
	DataPtr m_ptr;
	size_t m_size;
	bool m_isFinal;
};

//.............................................................................

struct ConstBuffer
{
	JNC_BEGIN_TYPE_MAP ("std.ConstBuffer", g_stdLibCacheSlot, StdLibCacheSlot_ConstBuffer)
		JNC_MAP_FUNCTION ("copy", &ConstBuffer::copy_s1)
		JNC_MAP_OVERLOAD (&ConstBuffer::copy_s2)
	JNC_END_TYPE_MAP ()

public:
	DataPtr m_ptr;
	size_t m_size;

public:
	bool 
	copy (ConstBufferRef ref);

	bool 
	copy (
		DataPtr ptr,
		size_t size
		);

protected:
	static
	bool 
	copy_s1 (
		DataPtr selfPtr,
		ConstBufferRef ref
		)
	{
		return ((ConstBuffer*) selfPtr.m_p)->copy (ref);
	}

	static
	bool 
	copy_s2 (
		DataPtr selfPtr,
		DataPtr ptr,
		size_t size
		)
	{
		return ((ConstBuffer*) selfPtr.m_p)->copy (ptr, size);
	}
};

//.............................................................................

struct BufferRef
{
	JNC_BEGIN_TYPE_MAP ("std.BufferRef", g_stdLibCacheSlot, StdLibCacheSlot_BufferRef)
	JNC_END_TYPE_MAP ()

public:
	DataPtr m_ptr;
	size_t m_size;
};

//.............................................................................

class Buffer: public IfaceHdr
{
public:
	JNC_BEGIN_TYPE_FUNCTION_MAP ("std.Buffer", g_stdLibCacheSlot, StdLibCacheSlot_Buffer)
		JNC_MAP_FUNCTION ("copy", &Buffer::copy)
		JNC_MAP_FUNCTION ("append", &Buffer::append)
	JNC_END_TYPE_FUNCTION_MAP ()

public:
	DataPtr m_ptr;
	size_t m_size;
	size_t m_maxSize;

public:
	bool 
	AXL_CDECL
	copy (
		DataPtr ptr,
		size_t size
		);

	bool 
	AXL_CDECL
	append (
		DataPtr ptr,
		size_t size
		);

protected:
	bool
	setSize (
		size_t size,
		bool saveContents
		);
};

//.............................................................................

} // namespace std
} // namespace jnc
