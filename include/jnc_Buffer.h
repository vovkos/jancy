#pragma once

#include "jnc_DataPtrType.h"
#include "jnc_Api.h"
#include "jnc_StdLibApiSlots.h"

namespace jnc {
		
//.............................................................................

struct ConstBuffer
{
	JNC_API_BEGIN_TYPE ("jnc.ConstBuffer", ApiSlot_jnc_ConstBuffer)
		JNC_API_FUNCTION_0 ("copy")
		JNC_API_OVERLOAD (&ConstBuffer::copy_s)
	JNC_API_END_TYPE ()

public:
	DataPtr m_ptr;
	size_t m_size;

public:
	bool 
	copy (
		DataPtr ptr,
		size_t size
		);

protected:
	static
	bool 
	copy_s (
		DataPtr selfPtr,
		DataPtr ptr,
		size_t size
		)
	{
		return ((ConstBuffer*) selfPtr.m_p)->copy (ptr, size);
	}
};

//.............................................................................

struct ConstBufferRef
{
	JNC_API_BEGIN_TYPE ("jnc.ConstBufferRef", ApiSlot_jnc_ConstBufferRef)
	JNC_API_END_TYPE ()

public:
	DataPtr m_ptr;
	size_t m_length;
	bool m_isFinal;
};

//.............................................................................

struct BufferRef
{
	JNC_API_BEGIN_TYPE ("jnc.BufferRef", ApiSlot_jnc_BufferRef)
	JNC_API_END_TYPE ()

public:
	DataPtr m_ptr;
	size_t m_length;
};

//.............................................................................

class Buffer: public IfaceHdr
{
public:
	JNC_API_BEGIN_CLASS ("jnc.Buffer", ApiSlot_jnc_Buffer)
		JNC_API_FUNCTION_0 ("copy")
		JNC_API_OVERLOAD (&Buffer::copy)
		JNC_API_FUNCTION_0 ("append")
		JNC_API_OVERLOAD (&Buffer::append)
	JNC_API_END_CLASS ()

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
};

//.............................................................................

} // namespace jnc
