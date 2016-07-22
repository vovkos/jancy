#pragma once

#include "jnc_ExtensionLib.h"

namespace jnc {
namespace std {

JNC_DECLARE_TYPE (ConstBufferRef)
JNC_DECLARE_TYPE (ConstBuffer)
JNC_DECLARE_TYPE (BufferRef)
JNC_DECLARE_TYPE (Buffer)

//.............................................................................

struct ConstBufferRef
{
	DataPtr m_ptr;
	size_t m_size;
	bool m_isFinal;
};

//.............................................................................

struct ConstBuffer
{
	DataPtr m_ptr;
	size_t m_size;

	bool 
	copy (ConstBufferRef ref);

	bool 
	copy (
		DataPtr ptr,
		size_t size
		);

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
	DataPtr m_ptr;
	size_t m_size;
};

//.............................................................................

class Buffer: public IfaceHdr
{
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
