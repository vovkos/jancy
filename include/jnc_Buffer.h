#pragma once

#include "jnc_DataPtrType.h"
#include "jnc_Api.h"
#include "jnc_StdLibApiSlots.h"

namespace jnc {
		
//.............................................................................

struct ConstBufferRef
{
	JNC_BEGIN_TYPE ("jnc.ConstBufferRef", ApiSlot_jnc_ConstBufferRef)
	JNC_END_TYPE ()

public:
	DataPtr m_ptr;
	size_t m_size;
	bool m_isFinal;
};

//.............................................................................

struct ConstBuffer
{
	JNC_BEGIN_TYPE ("jnc.ConstBuffer", ApiSlot_jnc_ConstBuffer)
		JNC_FUNCTION ("copy", &ConstBuffer::copy_s1)
		JNC_OVERLOAD (&ConstBuffer::copy_s2)
	JNC_END_TYPE ()

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
	JNC_BEGIN_TYPE ("jnc.BufferRef", ApiSlot_jnc_BufferRef)
	JNC_END_TYPE ()

public:
	DataPtr m_ptr;
	size_t m_size;
};

//.............................................................................

class Buffer: public IfaceHdr
{
public:
	JNC_BEGIN_CLASS ("jnc.Buffer", ApiSlot_jnc_Buffer)
		JNC_FUNCTION_0 ("copy")
		JNC_OVERLOAD (&Buffer::copy)
		JNC_FUNCTION_0 ("append")
		JNC_OVERLOAD (&Buffer::append)
	JNC_END_CLASS ()

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

} // namespace jnc
